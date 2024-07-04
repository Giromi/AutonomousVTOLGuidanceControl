#include <rclcpp/rclcpp.hpp>
#include <mavros_msgs/msg/actuator_control.hpp>
#include <mavros_msgs/msg/override_rc_in.hpp>
#include <rclcpp/parameter_client.hpp>
#include <mavros_msgs/srv/param_set_v2.hpp>
#include <std_msgs/msg/string.hpp>

class ServoControlPublisher : public rclcpp::Node {
public:
    ServoControlPublisher(void);
private:
    int      pwm;
    float    pwm_nomallize;

    /* 1 */
    void _publishpwm_output_message(void);
    void _publish_disarm_control_message(void);
    void _response_callback(rclcpp::Client<mavros_msgs::srv::ParamSetV2>::SharedFuture future);
    std::string generate_uuid(void);
    void _chatterCallback(const std_msgs::msg::String::SharedPtr msg);


    rclcpp::TimerBase::SharedPtr                              _timer;
    std::shared_ptr<rclcpp::SyncParametersClient>             _parameters_client;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr    _subscription;
};

