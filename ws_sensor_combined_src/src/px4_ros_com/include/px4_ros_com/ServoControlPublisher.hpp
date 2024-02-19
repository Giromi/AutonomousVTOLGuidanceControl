#include <rclcpp/rclcpp.hpp>
#include <mavros_msgs/msg/actuator_control.hpp>
#include <mavros_msgs/msg/override_rc_in.hpp>
#include <rclcpp/parameter_client.hpp>
#include <mavros_msgs/srv/param_set.hpp>

class ServoControlPublisher : public rclcpp::Node {
public:
    ServoControlPublisher(void);

private:
    void _publish_pwm_output_message(void);
    void _publish_arm_control_message(void);
    void _publish_disarm_control_message(void);
    void _publish_disarm_control_message2(void);
    void _publish_disarm_control_message_param(void);
    void _set_mavros_param(const std::string& param_name, int param_value);
    void _set_ros_param(const std::string& param_name, int param_value);

    rclcpp::TimerBase::SharedPtr                                        _timer;
    rclcpp::Publisher<mavros_msgs::msg::ActuatorControl>::SharedPtr     _publisher_arm;
    rclcpp::Publisher<mavros_msgs::msg::OverrideRCIn>::SharedPtr        _publisher_disarm;
    rclcpp::Client<rcl_interfaces::srv::SetParameters>::SharedPtr       _param_client;

    std::shared_ptr<rclcpp::SyncParametersClient>                       _parameters_client;  // 파라미터 클라이언트 추가

    static int      _pwm;
    static float    _pwm_nomallize;
};

