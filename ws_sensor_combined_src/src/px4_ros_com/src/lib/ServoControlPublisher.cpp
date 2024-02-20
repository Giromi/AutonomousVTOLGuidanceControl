#include "px4_ros_com/ServoControlPublisher.hpp"


ServoControlPublisher::ServoControlPublisher(void)
<<<<<<< HEAD
    : Node("servo_control_publisher"), _pwm(800), _pwm_nomallize(0.0f), _offboard_setpoint_counter(0) {
        _offboard_control_mode_publisher = this->create_publisher<px4_msgs::msg::OffboardControlMode>("/fmu/in/offboard_control_mode", 10);
        _publisher_arm = this->create_publisher<px4_msgs::msg::ActuatorServos>( "/fmu/in/actuator_servos", 10);
    _vehicle_command_publisher = this->create_publisher<px4_msgs::msg::VehicleCommand>("/fmu/in/vehicle_command", 10);
    _timer = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&ServoControlPublisher::_publish_pwm_output_message, this));
=======
    : Node("servo_control_publisher") {
        publisher_ = this->create_publisher<mavros_msgs::msg::ActuatorControl>("/mavros/actuator_control", 10);
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(1000),
            std::bind(&ServoControlPublisher::_publish_servo_control_message, this));
    }

void ServoControlPublisher::_publish_servo_control_message(void)
{
    auto message = mavros_msgs::msg::ActuatorControl();
    message.header.stamp = this->get_clock()->now();
    message.group_mix = 3;  // Use group 2 for AUX channels in PX4
    message.controls[0] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    message.controls[1] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    message.controls[2] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    message.controls[3] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    message.controls[4] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    message.controls[5] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    message.controls[6] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    message.controls[7] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel


    publisher_->publish(message);
>>>>>>> main
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
        // this->_publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_DO_SET_MODE, 1, 6);
        // this->_arm();
    // }
    _publish_offboard_control_mode();
    _publish_arm_control_message();
    _offboard_setpoint_counter++;
    // _publish_disarm_control_message();
    // _publish_disarm_control_message2();
    // _publish_disarm_control_message_param();
}


void ServoControlPublisher::_publish_arm_control_message(void) {
    px4_msgs::msg::ActuatorServos msg{};
    // auto message = mavros_msgs::msg::ActuatorControl();
	msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
    msg.control[0] = _pwm_nomallize;
    msg.control[1] = _pwm_nomallize;
    std::cout << "Publishing arm control message " << _pwm_nomallize << std::endl;
    _pwm_nomallize = _offboard_setpoint_counter <= 25 ? 0.25f : 
                     _offboard_setpoint_counter <= 50 ? 0.5f :
                     _offboard_setpoint_counter <= 75 ? 0.25f :
                     _offboard_setpoint_counter <= 100 ? 0.0f : 
                     _offboard_setpoint_counter <= 125 ? -0.25f : 
                     _offboard_setpoint_counter <= 150 ? -0.5f :
                     _offboard_setpoint_counter <= 175 ? -0.25f : 0.0f;
    _publisher_arm->publish(msg);
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
