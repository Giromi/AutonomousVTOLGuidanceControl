#include <rclcpp/rclcpp.hpp>
#include <mavros_msgs/msg/actuator_control.hpp>

class ServoControlPublisher : public rclcpp::Node {
public:
    ServoControlPublisher(void);

private:
    void _publish_servo_control_message(void);

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<mavros_msgs::msg::ActuatorControl>::SharedPtr publisher_;
    static float counter_;
};

