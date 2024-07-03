#include "px4_ros_com/OffboardMavros.hpp"


 /* -- Initialize Functions -- */

void OffboardMavros::initializeConstant(void) {
    init_global_position[vtol::ALT] = std::atoi(std::getenv("PX4_HOME_ALT"))+40;
    init_global_position[vtol::LAT] = std::atoi(std::getenv("PX4_HOME_LAT"));
    init_global_position[vtol::LON] = std::atoi(std::getenv("PX4_HOME_LON"));

    DEBUG::print("alt: ", init_global_position[vtol::ALT],BLUE);
    DEBUG::print("lat: ", init_global_position[vtol::LAT],BLUE);
    DEBUG::print("lon: ", init_global_position[vtol::LON],BLUE);
}

void OffboardMavros::initializePublishers(void) {
    local_pos_pub = create_publisher<geometry_msgs::msg::PoseStamped>("/mavros/setpoint_position/local", 10);
    local_vel_pub = this->create_publisher<geometry_msgs::msg::Twist>("/mavros/setpoint_velocity/cmd_vel_unstamped", 10);

    local_pub = this->create_publisher<mavros_msgs::msg::PositionTarget>("mavros/setpoint_raw/local", 10);
    att_pub = this->create_publisher<geometry_msgs::msg::TwistStamped>("mavros/setpoint_attitude/cmd_vel", 10);
    actuator_control_pub = this->create_publisher<mavros_msgs::msg::ActuatorControl>( "/mavros/actuator_control", 10);
}

void OffboardMavros::initializeSubscribers(void) {
    auto default_qos = rclcpp::QoS(rclcpp::SystemDefaultsQoS());
    
    state_sub = create_subscription<mavros_msgs::msg::State>(
            "mavros/state", default_qos, std::bind(&OffboardMavros::stateCallBack, this, std::placeholders::_1));

    subscription = this->create_subscription<std_msgs::msg::String>("/chatter", 10,
            std::bind( &OffboardMavros::chatterCallback, this, std::placeholders::_1
                ));
    current_pos_sub = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/local_position/pose", default_qos,
            std::bind(&OffboardMavros::currentPositionCallback, this, std::placeholders::_1
                ));

    pose_sub = create_subscription<geometry_msgs::msg::PoseStamped>(
            "/mavros/local_position/pose", default_qos, std::bind(&OffboardMavros::poseCallBack, this, std::placeholders::_1));

    gps_sub_=create_subscription<sensor_msgs::msg::NavSatFix>(
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
}

void OffboardMavros::initializeTimers(const int rate_hz) {
    const int rate_ms = 1000 / rate_hz;
    timer = this->create_wall_timer(
            std::chrono::milliseconds(rate_ms),
            std::bind(&OffboardMavros::publish, this));
}
