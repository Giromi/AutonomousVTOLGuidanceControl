#include <rclcpp/rclcpp.hpp>
#include <mavros_msgs/msg/actuator_control.hpp>
#include <mavros_msgs/msg/override_rc_in.hpp>
#include <rclcpp/parameter_client.hpp>
#include <mavros_msgs/srv/param_set_v2.hpp>

class ServoControlPublisher : public rclcpp::Node {
public:
    ServoControlPublisher(void);
private:
    int      _pwm;
    float    _pwm_nomallize;

    void _publish_pwm_output_message(void);
    // void _publish_arm_control_message(void);
    void _publish_disarm_control_message(void);
    void _response_callback(rclcpp::Client<mavros_msgs::srv::ParamSetV2>::SharedFuture future);
    std::string generate_uuid(void);
    // void _publish_disarm_control_message2(void);
    // void _publish_disarm_control_message_param(void);
    // void _set_mavros_param(const std::string& param_name, int param_value);
    // void _set_ros_param(const std::string& param_name, int param_value);
    // 복사 생성자와 할당 연산자 금지

    rclcpp::TimerBase::SharedPtr                                        _timer;
    std::shared_ptr<rclcpp::SyncParametersClient>                       _parameters_client;

    rclcpp::Publisher<mavros_msgs::msg::ActuatorControl>::SharedPtr     _publisher_arm;
    rclcpp::Publisher<mavros_msgs::msg::OverrideRCIn>::SharedPtr        _publisher_disarm;
    rclcpp::Client<mavros_msgs::srv::ParamSetV2>::SharedPtr _param_client;


};

