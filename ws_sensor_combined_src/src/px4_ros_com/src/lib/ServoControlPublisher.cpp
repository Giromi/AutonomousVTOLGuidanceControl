
#include "px4_ros_com/ServoControlPublisher.hpp"

float ServoControlPublisher::counter_ = -1.0f;

ServoControlPublisher::ServoControlPublisher(void)
    : Node("servo_control_publisher") {
        publisher_ = this->create_publisher<mavros_msgs::msg::ActuatorControl>("/mavros/actuator_control", 10);
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(1000),
            std::bind(&ServoControlPublisher::_publish_servo_control_message, this));
    }

void ServoControlPublisher::_publish_servo_control_message(void)
{
    if (ServoControlPublisher::counter_ < 1)
        ServoControlPublisher::counter_ += 0.1;
    auto message = mavros_msgs::msg::ActuatorControl();
    message.header.stamp = this->get_clock()->now();
    message.group_mix = 3;  // Use group 2 for AUX channels in PX4
    message.controls[0] = ServoControlPublisher::counter_;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    message.controls[1] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    message.controls[6] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    message.controls[7] = 0.5;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    std::cout << "Publishing servo control message" << counter_ << std::endl;

    publisher_->publish(message);
}

