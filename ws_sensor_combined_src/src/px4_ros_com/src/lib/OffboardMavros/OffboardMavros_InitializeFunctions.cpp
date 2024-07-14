#include "px4_ros_com/OffboardMavros.hpp"

void OffboardMavros::initializeConstant(void) {
    const char* home_alt = std::getenv("PX4_HOME_ALT");
    const char* home_lat = std::getenv("PX4_HOME_LAT");
    const char* home_lon = std::getenv("PX4_HOME_LON");

    if (home_alt == nullptr || home_lat == nullptr || home_lon == nullptr) {
        RCLCPP_ERROR(this->get_logger(), "Please set the environment variables: PX4_HOME_ALT, PX4_HOME_LAT, PX4_HOME_LON");
        rclcpp::shutdown();
    }

    init_global_position[vtol::ALT] = std::atoi(home_alt) + 5;
    init_global_position[vtol::LAT] = std::atoi(home_lat);
    init_global_position[vtol::LON] = std::atoi(home_lon);

}

void OffboardMavros::initializePublishers(void) {
    auto default_qos = rclcpp::QoS(rclcpp::SystemDefaultsQoS());
    local_pos_pub = create_publisher<geometry_msgs::msg::PoseStamped>("/mavros/setpoint_position/local", 10);
    local_vel_pub = this->create_publisher<geometry_msgs::msg::Twist>("/mavros/setpoint_velocity/cmd_vel_unstamped", 10);
    local_pub = this->create_publisher<mavros_msgs::msg::PositionTarget>("/mavros/setpoint_raw/local", 10);
    att_pub = this->create_publisher<geometry_msgs::msg::TwistStamped>("/mavros/setpoint_attitude/cmd_vel", 10);
    actuator_control_pub = this->create_publisher<mavros_msgs::msg::ActuatorControl>( "/mavros/actuator_control", 10);

    // waypoints_pub = this->create_publisher<mavros_msgs::msg::WaypointList>("/mavros/mission/waypoints", default_qos);
    gp_origin_pub = this->create_publisher<geographic_msgs::msg::GeoPoseStamped>("/mavros/global_position/set_gp_origin", default_qos);
}

void OffboardMavros::initializeSubscribers(void) {
    auto default_qos = rclcpp::QoS(rclcpp::SystemDefaultsQoS());
    const std::function<void(const mavros_msgs::msg::State::SharedPtr)> state_bind = std::bind(&OffboardMavros::stateCallBack, this, std::placeholders::_1);
    const std::function<void(const std_msgs::msg::String::SharedPtr)> subscription_bind = std::bind(&OffboardMavros::chatterCallback, this, std::placeholders::_1);

    state_sub = create_subscription<mavros_msgs::msg::State>("mavros/state", default_qos, state_bind);

    local_position_sub = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/local_position/pose", default_qos,
            std::bind(&OffboardMavros::localPositionCallback, this, std::placeholders::_1));
    subscription = this->create_subscription<std_msgs::msg::String>("/chatter", 10, subscription_bind);

    pose_sub = create_subscription<geometry_msgs::msg::PoseStamped>(
            "/mavros/local_position/pose", default_qos, std::bind(&OffboardMavros::poseCallBack, this, std::placeholders::_1));

    global_posistion_sub =create_subscription<sensor_msgs::msg::NavSatFix>(
            "/mavros/global_position/global", default_qos, std::bind(&OffboardMavros::gpsCallBack, this, std::placeholders::_1));
}

void    OffboardMavros::initializeClients(void) {
    set_mode_client = create_client<mavros_msgs::srv::SetMode>("/mavros/set_mode");
    arming_client = create_client<mavros_msgs::srv::CommandBool>("/mavros/cmd/arming");
    takeoff_client = create_client<mavros_msgs::srv::CommandTOL>("/mavros/cmd/takeoff");
    landing_client = create_client<mavros_msgs::srv::CommandTOL>("/mavros/cmd/land");
    location_client = this->create_client<mavros_msgs::srv::CommandLong>("/mavros/cmd/command");
    transition_client = this->create_client<mavros_msgs::srv::CommandVtolTransition>("/mavros/cmd/vtol_transition");
    cmd_client = this->create_client<mavros_msgs::srv::CommandLong>("/mavros/cmd/command");

    // 이거 사용할 빠에는 QGC로 하는게 나음
    waypoint_push_client = this->create_client<mavros_msgs::srv::WaypointPush>("/mavros/mission/push");
    waypoint_clear_client = this->create_client<mavros_msgs::srv::WaypointClear>("/mavros/mission/clear");
}

void OffboardMavros::initializeTimers(const int rate_hz) {
    const int rate_ms = 1000 / rate_hz;
    timer = this->create_wall_timer(
            std::chrono::milliseconds(rate_ms),
            std::bind(&OffboardMavros::publish, this));
}

void OffboardMavros::initializeVariables(void) {

    // queue는 리스트초기화 안됨
    // deque로 초기화 후 queue로 이동했음
    const std::deque<vtol::ReferenceWaypoint> input({ 
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 2.0f},
        {0.0f, 2.0f, 2.0f},
        {2.0f, 2.0f, 2.0f},
        {2.0f, 0.0f, 2.0f},
        {0.0f, 0.0f, 2.0f},
        {0.0f, 0.0f, 0.0f}
    });

    // 이동 시멘틱을 사용하여 operator=으로 std::queue 초기화
    ref_waypoints = std::queue<vtol::ReferenceWaypoint>(std::move(input));

    init_global_position = {-1.0f, -1.0f, -1.0f};

    state_value_array = { 
        vtol::INIT, 
        vtol::READY, 
        vtol::ARMED,
        vtol::FLY,
        vtol::TAKEOFF,
        vtol::LAND,
        vtol::MISSION,
        vtol::FIXED,
        vtol::TO_FIXED,
        vtol::TO_QUAD,
        vtol::MC_START,
        vtol::FW_START,
    };
}

void OffboardMavros::initializeStateFuncPointerArray(
    const std::array<std::function <void(void)>, vtol::STATE_SIZE>& input
    ) {
    for (size_t i = 0; i < vtol::STATE_SIZE; ++i) {
        stateFunc[i] = input[i];
    }
}

void OffboardMavros::initializeFunctionPointerArray(void) {
    initializeStateFuncPointerArray({ 
            std::bind(&OffboardMavros::stateCommandInit,    this),
            std::bind(&OffboardMavros::stateCommandReady,   this),
            std::bind(&OffboardMavros::stateCommandArmed,   this),
            std::bind(&OffboardMavros::stateCommandFly,     this),
            std::bind(&OffboardMavros::stateCommandTakeOff, this),
            std::bind(&OffboardMavros::stateCommandLand,    this),
            std::bind(&OffboardMavros::stateCommandMission, this),
            std::bind(&OffboardMavros::stateCommandFixed,   this),
            std::bind(&OffboardMavros::stateCommandToFixed, this),
            std::bind(&OffboardMavros::stateCommandToQuad,  this),
            std::bind(&OffboardMavros::stateCommandStartMC, this),
            std::bind(&OffboardMavros::stateCommandStartFW, this),
    });
}

/**
 * @brief setWaypoint
 * @param float  param[4] : param1, param2, param3, param4
 * @param double pos[3]   : x_lat, y_long, z_alt
 */
void OffboardMavros::setWaypoint(const uint8_t frame, const uint16_t command, 
                                 const bool is_current, const bool autocontinue, 
                                 const std::array<float, 4> param, 
                                 const std::array<double, 3> pos) {
    mavros_msgs::msg::Waypoint  wp;

    wp.frame = frame;
    wp.command = command;
    wp.is_current = is_current;
    wp.autocontinue = autocontinue;
    wp.param1 = param[0];
    wp.param2 = param[1];
    wp.param3 = param[2];
    wp.param4 = param[3];
    wp.z_alt = pos[0];  
    wp.x_lat = pos[1];
    wp.y_long = pos[2];

    DEBUG::printArray("Waypoint", pos, 3, BOLDGREEN);
    waypoint_list.waypoints.push_back(wp);
}

void OffboardMavros::triangleScenarioMC(const std::array<double, 3>& target_pos) {
    // Takeoff
    setWaypoint(mavros_msgs::msg::Waypoint::FRAME_GLOBAL_REL_ALT,
                mavros_msgs::msg::CommandCode::NAV_TAKEOFF, true, true, 
                // mavros_msgs::msg::CommandCode::NAV_VTOL_TAKEOFF, true, true, 
                {0, 0, 0, vtol::NaN}, { target_pos[0],
                                        target_pos[1],
                                        target_pos[2]});

    // Loiter
    setWaypoint(mavros_msgs::msg::Waypoint::FRAME_GLOBAL_REL_ALT,
                mavros_msgs::msg::CommandCode::NAV_LOITER_TIME, false, true, 
                {10, 0, 0, vtol::NaN}, { target_pos[0],
                                         target_pos[1],
                                         target_pos[2]});

    // Waypoint 1
    setWaypoint(mavros_msgs::msg::Waypoint::FRAME_GLOBAL_REL_ALT,
                mavros_msgs::msg::CommandCode::NAV_WAYPOINT, false, true, 
                {5, 0, 0, vtol::NaN}, { target_pos[0],
                                        target_pos[1],
                                        target_pos[2] + 0.00090});

    // Waypoint 2
    setWaypoint(mavros_msgs::msg::Waypoint::FRAME_GLOBAL_REL_ALT,
                mavros_msgs::msg::CommandCode::NAV_WAYPOINT, false, true, 
                {5, 0, 0, vtol::NaN}, { target_pos[0],
                                        target_pos[1] + 0.00027,
                                        target_pos[2] + 0.00090 });

    // Land
    setWaypoint(mavros_msgs::msg::Waypoint::FRAME_GLOBAL_REL_ALT,
                mavros_msgs::msg::CommandCode::NAV_LAND, false, true, 
                {0, 0, 0, 1.57}, { 0, target_pos[1], target_pos[2]});
}


void OffboardMavros::triangleScenarioFW(const std::array<double, 3>& target_pos) {
    const float VTOL_TRANSITION_HEADING_NEXT_WAYPOINT = static_cast<float>(mavlink::common::VTOL_TRANSITION_HEADING::NEXT_WAYPOINT);

    // Takeoff
    setWaypoint(mavros_msgs::msg::Waypoint::FRAME_GLOBAL_REL_ALT,
                mavros_msgs::msg::CommandCode::NAV_VTOL_TAKEOFF, true, true, 
                {0, VTOL_TRANSITION_HEADING_NEXT_WAYPOINT , 0, vtol::NaN}, 
                { target_pos[0], target_pos[1], target_pos[2]});

    // Loiter
    setWaypoint(mavros_msgs::msg::Waypoint::FRAME_GLOBAL_REL_ALT,
                mavros_msgs::msg::CommandCode::NAV_WAYPOINT, false, true, 
                {5, 0, 0, vtol::NaN}, { target_pos[0],
                                        target_pos[1],
                                        target_pos[2] + 0.00090});
    // Waypoint 1
    setWaypoint(mavros_msgs::msg::Waypoint::FRAME_GLOBAL_REL_ALT,
                mavros_msgs::msg::CommandCode::NAV_WAYPOINT, false, true, 
                {5, 0, 0, vtol::NaN}, { target_pos[0],
                                        target_pos[1] + 0.00027,
                                        target_pos[2] + 0.00090 });
    // Waypoint 2
    setWaypoint(mavros_msgs::msg::Waypoint::FRAME_GLOBAL_REL_ALT,
                mavros_msgs::msg::CommandCode::NAV_LAND, false, false, 
                {0, 0, 0, 1.57}, { 0, target_pos[1], target_pos[2]});
}

void OffboardMavros::initializeWaypoints(void) {
    // WP0
    std::array<double, 3> home_alt_global_position = { 20.0, 47.398, 8.54616 };
    triangleScenarioFW(home_alt_global_position);

    // wp_manager.setPath(_star_path);
    wp_manager.setPath(_square_path);
    RCLCPP_INFO(this->get_logger(), "Waypoint list size: %u", wp_manager.size());
    wp_manager.printWaypoints();
}

const std::array<Eigen::Vector4d, 4> OffboardMavros::_square_path = {
    Eigen::Vector4d(200.0,        0.0,	    30.0,      0.00),
    Eigen::Vector4d(200.0,      200.0,	30.0,      1.57),
    Eigen::Vector4d(  0.0,      200.0,	    30.0,      -1.57),
    Eigen::Vector4d(  0.0,        0.0,	    30.0,      0.00),
};

const std::array<Eigen::Vector4d, 4> OffboardMavros::_triangle_path = {
    Eigen::Vector4d(0, 0, 0, 0),
    Eigen::Vector4d(0, 10, 0, 0),
    Eigen::Vector4d(10, 10, 0, 0),
    Eigen::Vector4d(5, 5, 0, 0),
};

const std::array<Eigen::Vector4d, 11> OffboardMavros::_star_path = {
    Eigen::Vector4d(100.0,   0.0,      30.0,  0.0),    // 동쪽
    Eigen::Vector4d(40.45,  29.39,     30.0,  2.214),  // 북동쪽
    Eigen::Vector4d(30.9,   95.11,     30.0,  1.57),   // 북쪽
    Eigen::Vector4d(-15.45, 47.55,     30.0,  1.884),  // 북서쪽
    Eigen::Vector4d(-80.9,  58.78,     30.0,  2.042),  // 북서쪽
    Eigen::Vector4d(-50.0,   0.0,      30.0,  3.14),   // 서쪽
    Eigen::Vector4d(-80.9, -58.78,     30.0, -2.042),  // 남서쪽
    Eigen::Vector4d(-15.45, -47.55,    30.0, -1.257),  // 남서쪽
    Eigen::Vector4d(30.9,  -95.11,     30.0, -1.57),   // 남쪽
    Eigen::Vector4d(40.45, -29.39,     30.0, -2.214),  // 남동쪽
    Eigen::Vector4d(100.0,   0.0,      30.0,  0.0)     // 동쪽
};

