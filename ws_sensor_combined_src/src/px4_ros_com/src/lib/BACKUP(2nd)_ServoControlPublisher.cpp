
#include "px4_ros_com/ServoControlPublisher.hpp"


ServoControlPublisher::ServoControlPublisher(void)
    : Node("servo_control_publisher"), _pwm(800), _pwm_nomallize(-1.0){
        _offboard_control_mode_publisher = this->create_publisher<px4_msgs::msg::OffboardControlMode>("/fmu/in/offboard_control_mode", 10);
    _publisher_arm = this->create_publisher<mavros_msgs::msg::ActuatorControl>( "/mavros/actuator_control", 10);
    _vehicle_command_publisher = this->create_publisher<px4_msgs::msg::VehicleCommand>("/fmu/in/vehicle_command", 10);
    _timer = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&ServoControlPublisher::_publish_pwm_output_message, this));
}

void ServoControlPublisher::_publish_offboard_control_mode()
{
    px4_msgs::msg::OffboardControlMode msg{};
	msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
	msg.position = false;
	msg.velocity = false;
	msg.acceleration = false;
	msg.attitude = false;
	msg.body_rate = false;
    msg.thrust_and_torque = false;
    msg.direct_actuator = true;
	_offboard_control_mode_publisher->publish(msg);
}

void ServoControlPublisher::_publish_pwm_output_message(void) {
    // if (_pwm_nomallize == -1.0) {
    //     this->_publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_DO_SET_MODE, 1, 6);
    //     this->_arm();
    // }
    // if (_offboard_setpoint_counter == 10) {
    //     // Change to Offboard mode after 10 setpoints
    //
    //     // Arm the vehicle
    // }
    _publish_offboard_control_mode();
    _publish_arm_control_message();
    // if (_offboard_setpoint_counter < 11) {
    //     _offboard_setpoint_counter++;
    // }
    // _publish_disarm_control_message();
    // _publish_disarm_control_message2();
    // _publish_disarm_control_message_param();
}


void ServoControlPublisher::_publish_arm_control_message(void) {
    auto actuator_control_msg = std::make_shared<mavros_msgs::msg::ActuatorControl>();
    // auto message = mavros_msgs::msg::ActuatorControl();
    actuator_control_msg->header.stamp = this->now();
    actuator_control_msg->header.frame_id = "camera_servo";
    actuator_control_msg->group_mix = 2;  // Use group 2 for AUX channels in PX4
    // actuator_control_msg->controls.resize(8);  // 사용할 채널 수에 따라 크기 조정
    actuator_control_msg->controls[0] = _pwm_nomallize;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    _pwm_nomallize += (_pwm_nomallize < 0.9) * 0.1;
    std::cout << "Publishing arm control message" << _pwm_nomallize << std::endl;
    _publisher_arm->publish(*actuator_control_msg);
}

void ServoControlPublisher::_publish_vehicle_command(uint16_t command, float param1, float param2)
{
    px4_msgs::msg::VehicleCommand msg{};
	msg.param1 = param1;
	msg.param2 = param2;
	msg.command = command;
	msg.target_system = 1;
	msg.target_component = 1;
	msg.source_system = 1;
	msg.source_component = 1;
	msg.from_external = true;
	msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
	_vehicle_command_publisher->publish(msg);
}

void ServoControlPublisher::_arm()
{
	_publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 1.0);

	RCLCPP_INFO(this->get_logger(), "Arm command send");
}

/**
 * @brief Send a command to Disarm the vehicle
 */
void ServoControlPublisher::_disarm()
{
	_publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 0.0);

	RCLCPP_INFO(this->get_logger(), "Disarm command send");
}
