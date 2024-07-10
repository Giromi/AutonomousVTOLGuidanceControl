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
    vc_manual_pub =this->create_publisher<mavros_msgs::msg::ManualControl>( "/mavros/manual_control/send", default_qos);
    waypoints_pub = this->create_publisher<mavros_msgs::msg::WaypointList>("/mavros/mission/waypoints", default_qos);
    gp_origin_pub = this->create_publisher<geographic_msgs::msg::GeoPoseStamped>("/mavros/global_position/set_gp_origin", default_qos);
}

void OffboardMavros::initializeSubscribers(void) {
    auto default_qos = rclcpp::QoS(rclcpp::SystemDefaultsQoS());
    const std::function<void(const mavros_msgs::msg::State::SharedPtr)> state_bind = std::bind(&OffboardMavros::stateCallBack, this, std::placeholders::_1);
    const std::function<void(const std_msgs::msg::String::SharedPtr)> subscription_bind = std::bind(&OffboardMavros::chatterCallback, this, std::placeholders::_1);

    state_sub = create_subscription<mavros_msgs::msg::State>("mavros/state", default_qos, state_bind);
    subscription = this->create_subscription<std_msgs::msg::String>("/chatter", 10, subscription_bind);

    current_pos_sub = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/local_position/pose", default_qos,
            std::bind(&OffboardMavros::currentPositionCallback, this, std::placeholders::_1
                ));

    pose_sub = create_subscription<geometry_msgs::msg::PoseStamped>(
            "/mavros/local_position/pose", default_qos, std::bind(&OffboardMavros::poseCallBack, this, std::placeholders::_1));

    gps_sub =create_subscription<sensor_msgs::msg::NavSatFix>(
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
    state_value_array = { 
        vtol::INIT, 
        vtol::READY, 
        vtol::ARMED,
        vtol::FLY,
        vtol::TAKEOFF,
        vtol::LAND,
        vtol::START,
        vtol::MISSION,
        vtol::TO_FIXED,
        vtol::TO_QUAD

    };


    // queue는 리스트초기화 안됨
    // deque로 초기화 후 queue로 이동했음
    const std::deque<vtol::Waypoint> input({ 
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 2.0f},
        {0.0f, 2.0f, 2.0f},
        {2.0f, 2.0f, 2.0f},
        {2.0f, 0.0f, 2.0f},
        {0.0f, 0.0f, 2.0f},
        {0.0f, 0.0f, 0.0f}
    });

    // 이동 시멘틱을 사용하여 operator=으로 std::queue 초기화
    waypoints = std::queue<vtol::Waypoint>(std::move(input));

    init_global_position = {-1.0f, -1.0f, -1.0f};

}

void OffboardMavros::initializeStateFuncPointerArray(
    const std::array<std::function <void(void)>, vtol::STATE_SIZE>& input
    ) {
    for (size_t i = 0; i < vtol::STATE_SIZE; ++i) {
        stateFunc[i] = input[i];
    }
}

void OffboardMavros::initializeFunctionPointerArray(void) {
    initializeStateFuncPointerArray( { std::bind(&OffboardMavros::stateCommandInit, this),
                                       std::bind(&OffboardMavros::stateCommandReady,this),
                                       std::bind(&OffboardMavros::stateCommandArmed, this),
                                       std::bind(&OffboardMavros::stateCommandFly, this),
                                       std::bind(&OffboardMavros::stateCommandTakeOff, this),
                                       std::bind(&OffboardMavros::stateCommandLand, this),
                                       std::bind(&OffboardMavros::stateCommandStart, this),
                                       std::bind(&OffboardMavros::stateCommandMission, this),
                                       std::bind(&OffboardMavros::stateCommandToFixed, this),
                                       std::bind(&OffboardMavros::stateCommandToQuad, this)
                                    } );
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
                mavros_msgs::msg::CommandCode::NAV_LAND, false, true, 
                {0, 0, 0, 1.57}, { 0, target_pos[1], target_pos[2]});
}

void OffboardMavros::initializeWaypoints(void) {
    // WP0
                                                   /*{ 상대, 절대,   절대 } */
    std::array<double, 3> home_alt_global_position = { 20.0, 47.398, 8.54616 };
    triangleScenarioFW(home_alt_global_position);
}
