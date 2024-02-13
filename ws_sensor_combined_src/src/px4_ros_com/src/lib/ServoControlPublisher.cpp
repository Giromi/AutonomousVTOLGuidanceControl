
#include "px4_ros_com/ServoControlPublisher.hpp"

ServoControlPublisher::ServoControlPublisher(void)
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
    message.group_mix = 2;  // Use group 2 for AUX channels in PX4
    message.controls[0] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    message.controls[1] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel

    publisher_->publish(message);
}

