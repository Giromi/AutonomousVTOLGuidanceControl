#include "px4_ros_com/OffboardControl.hpp"
#include "px4_ros_com/Dubins.hpp"

std::queue<WayPoint> OffboardControl::_way_points;
std::queue<DubinsPathPoint> OffboardControl::_dubins_path_points;

double OffboardControl::_turning_radius = 10.0;
double OffboardControl::_sampling_interval = 2.0;

OffboardControl::OffboardControl()
    : Node("offboard_control"), pwm(800), pwm_nomallize(-1.0) {
    initializePublishers();
    initializeSubscribers();
    initializeClients();

    auto timer_callback = [this]() -> void {
        if (offboard_setpoint_counter == 10) {
            // Change to Offboard mode after 10 setpoints
            this->publishVehicleCommand(VehicleCommand::VEHICLE_CMD_DO_SET_MODE, 1, 6);

            // Arm the vehicle
            this->arm();
        }

        // offboard_control_mode needs to be paired with trajectory_setpoint
        publishOffboardControlMode();
        // publishTrajectorySetpoint();
        publishPwmOutputMessage();
        // stop the counter after reaching 11
        if (offboard_setpoint_counter < 11) {
            offboard_setpoint_counter++;
        }
    };
    timer = this->create_wall_timer(1000ms, timer_callback);
}


void OffboardControl::initializeSubscribers() {
    rmw_qos_profile_t qos_profile = rmw_qos_profile_sensor_data;
    auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, 5), qos_profile);

    vehicle_local_position_subscription = this->create_subscription<px4_msgs::msg::VehicleLocalPosition>("/fmu/out/vehicle_local_position", qos, [this](const px4_msgs::msg::VehicleLocalPosition::UniquePtr msg) {
            local_position = {msg->x, msg->y, msg->z, msg->heading};
            const int width = 10;
            std::cout << std::fixed << std::setprecision(2); // 소수점 이하 두 자리까지만 표시
            std::cout << "\n\n"
            << "RECEIVED SENSOR COMBINED DATA\n"
            << "===================================================\n"
            << "  Local Position  |  Way Point\n"
            << "===================================================\n"
            << "North: " << std::setw(width) << local_position[vtol::NORTH] << " ➔ " 
            << std::setw(width) << _way_points.front().north << "\n"
            << "East : " << std::setw(width) << local_position[vtol::EAST]  << " ➔ " 
            << std::setw(width) << _way_points.front().east << "\n"
            << "Down : " << std::setw(width) << local_position[vtol::UP]  << " ➔ " 
            << std::setw(width) << _way_points.front().down << "\n"
            << "Yaw  : " << std::setw(width) << local_position[vtol::YAW]   << " | " 
            << std::setw(width) << _way_points.front().yaw << "\n"
            << "---------------------------------------------------\n"
            << "(way point length) => " << sqrt(pow(local_position[vtol::NORTH] - _way_points.front().north
                        , 2) + pow(local_position[vtol::EAST] - _way_points.front().east, 2) 
                    + pow(local_position[vtol::UP] - _way_points.front().down, 2)) << "\n"
            << "(dubins path length) => " << sqrt(pow(local_position[vtol::NORTH] - _dubins_path_points.front().north , 2) + pow(local_position[vtol::EAST] - _dubins_path_points.front().east, 2)) << "\n"
            << "===================================================\n"
            << "Left way points         : " << _way_points.size() << "\n"
            << "Left Dubins path points : " << _dubins_path_points.size() << "\n"
            << "===================================================\n";
    });

}

void OffboardControl::initializePublishers() {
    offboard_control_mode_publisher = this->create_publisher<OffboardControlMode>("/fmu/in/offboard_control_mode", 10);
    trajectory_setpoint_publisher = this->create_publisher<TrajectorySetpoint>("/fmu/in/trajectory_setpoint", 10);
    vehicle_command_publisher = this->create_publisher<VehicleCommand>("/fmu/in/vehicle_command", 10);
    publisher_arm = this->create_publisher<mavros_msgs::msg::ActuatorControl>( "/mavros/actuator_control", 10);

}

void OffboardControl::initializeClients() {

}

/**
 * @brief Send a command to Arm the vehicle
 */
void OffboardControl::arm()
{
    publishVehicleCommand(VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 1.0);

    RCLCPP_INFO(this->get_logger(), "Arm command send");
}

/**
 * @brief Send a command to Disarm the vehicle
 */
void OffboardControl::disArm()
{
    publishVehicleCommand(VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 0.0);

    RCLCPP_INFO(this->get_logger(), "Disarm command send");
}

/**
 * @brief Publish the offboard control mode.
 *        For this example, only position and altitude controls are active.
 */
void OffboardControl::publishOffboardControlMode()
{
    OffboardControlMode msg{};
    msg.position = true;
    msg.velocity = false;
    msg.acceleration = false;
    msg.attitude = false;
    msg.body_rate = false;
    msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
    offboard_control_mode_publisher->publish(msg);
}

/**
 * @brief Publish a trajectory setpoint
 *        For this example, it sends a trajectory setpoint to make the
 *        vehicle hover at 5 meters with a yaw angle of 180 degrees.
 */
void OffboardControl::publishTrajectorySetpoint(void)
{
    TrajectorySetpoint msg{};
    std::array<float, 3> norm = {_way_points.front().north, _way_points.front().east, _way_points.front().down};
    if (_way_points.front().is_dubins_path 
            && !isReachWayPointWithNorm(norm)) {
        makeDubinsTrajectorySetpoint(msg);
    } else {
        makeGeneralTrajectorySetpoint(msg);
    }
    trajectory_setpoint_publisher->publish(msg);
}

void OffboardControl::makeGeneralTrajectorySetpoint(TrajectorySetpoint& msg) {
    msg.position = {_way_points.front().north, _way_points.front().east, _way_points.front().down};
    msg.yaw = _way_points.front().yaw ? _way_points.front().yaw
        : atan2(_way_points.front().east - local_position[vtol::EAST], 
                _way_points.front().north - local_position[vtol::NORTH]); // -pi ~ pi
    msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
    if (_way_points.size() == 1) {
        return ;
    }
    if (isReachWayPointWithNorm(msg.position)) {
        std::cout << "way point reached" << std::endl;
        _way_points.pop();
    }
}

void OffboardControl::makeDubinsTrajectorySetpoint(TrajectorySetpoint& msg) {
    if (_dubins_path_points.empty()) {
        std::cout << "dubins path planning" << std::endl;
        const std::array<double, 3> start = {local_position[vtol::NORTH], local_position[vtol::EAST], local_position[vtol::YAW]};
        const std::array<double, 3> end = {_way_points.front().north, _way_points.front().east, _way_points.front().yaw};
        Dubins dubins(start, end, OffboardControl::_turning_radius);
        dubins.shortestPath();
        dubins.pathSampleMany(OffboardControl::_sampling_interval, OffboardControl::_setDubinsPathPoint, &local_position);
    }
    msg.position = {_dubins_path_points.front().north, _dubins_path_points.front().east, _dubins_path_points.front().down};
    msg.yaw = _dubins_path_points.front().yaw ? _dubins_path_points.front().yaw
        : atan2(_dubins_path_points.front().east - local_position[vtol::EAST], 
                _dubins_path_points.front().north - local_position[vtol::NORTH]); // -pi ~ pi
    msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
    const std::array<float, 2> planar = {msg.position[vtol::NORTH], msg.position[vtol::EAST]};
    if (isReachWayPointWithNorm(planar)) {
        std::cout << "way point reached" << std::endl;
        _dubins_path_points.pop();
    }
}

bool OffboardControl::isReachWayPointWithSquare(std::array<float, 3> target) {
    return (abs(local_position[vtol::NORTH] - target[vtol::NORTH]) < 1.0 &&
            abs(local_position[vtol::EAST] - target[vtol::EAST]) < 1.0 &&
            abs(local_position[vtol::UP] - target[vtol::UP]) < 1.0);
}

bool OffboardControl::isReachWayPointWithNorm(std::array<float, 3> target) {
    return sqrt(pow(local_position[vtol::NORTH] - target[vtol::NORTH] , 2) +
            pow(local_position[vtol::EAST] - target[vtol::EAST], 2) +
            pow(local_position[vtol::UP] - target[vtol::UP], 2)) < 10.0;
}

bool OffboardControl::isReachWayPointWithNorm(std::array<float, 2> target) {
    return sqrt(pow(local_position[vtol::NORTH] - target[vtol::NORTH] , 2) +
            pow(local_position[vtol::EAST] - target[vtol::EAST], 2)) < 10.0;
}
// abs(local_position[vtol::YAW] - _way_points.front()[vtol::YAW]) < 1.0) {

/**
 * @brief Publish vehicle commands
 * @param command   Command code (matches VehicleCommand and MAVLink MAV_CMD codes)
 * @param param1    Command parameter 1
 * @param param2    Command parameter 2
 */
void OffboardControl::publishVehicleCommand(uint16_t command, float param1, float param2)
{
    VehicleCommand msg{};
    msg.param1 = param1;
    msg.param2 = param2;
    msg.command = command;
    msg.target_system = 1;
    msg.target_component = 1;
    msg.source_system = 1;
    msg.source_component = 1;
    msg.from_external = true;
    msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
    vehicle_command_publisher->publish(msg);
}

// void OffboardControl::dubinsPathPlanning(float x, float y) {
//
// }

// void OffboardControl::_setWayPoint(std::array<float, 4> way_point) {
//     OffboardControl::_way_points.push(way_point); 
// }

void OffboardControl::_setWayPoint(WayPoint way_point) {
    OffboardControl::_way_points.push(way_point); 
}

int OffboardControl::_setDubinsPathPoint(double q[3], double x, void* user_data) {
    static_cast<void>(x); // for unused
    LocalPosition* local_position = static_cast<LocalPosition *>(user_data);  
 
    DubinsPathPoint dubins_path_point(q[0], q[1], (*local_position)[vtol::UP], q[2], x);
    OffboardControl::_dubins_path_points.push(dubins_path_point);
    return 0;
}

// void OffboardControl::chatterCallback(const std_msgs::msg::String::SharedPtr msg) {
//     RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());
// }

void OffboardControl::publishPwmOutputMessage(void) {
    publishArmControlMessage();
    // _publish_disarm_control_message();
    // _publish_disarm_control_message2();
    // _publish_disarm_control_message_param();
}

void OffboardControl::publishArmControlMessage(void) {
    auto actuator_control_msg = mavros_msgs::msg::ActuatorControl();
    actuator_control_msg.header.stamp = this->now();
    actuator_control_msg.header.frame_id = "camera_servo";
    actuator_control_msg.group_mix = 2;  // Use group 2 for AUX channels in PX4
                                         // actuator_control_msg->controls.resize(8);  // 사용할 채널 수에 따라 크기 조정
    actuator_control_msg.controls[0] = pwm_nomallize;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    pwm_nomallize += (pwm_nomallize < 0.9) * 0.1;
    std::cout << "Publishing arm control message" << pwm_nomallize << std::endl;
    publisher_arm->publish(actuator_control_msg);
}
