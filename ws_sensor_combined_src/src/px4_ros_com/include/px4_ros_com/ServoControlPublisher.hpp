#ifndef SERVO_CONTROL_PUBLISHER_HPP
#define SERVO_CONTROL_PUBLISHER_HPP
# include <rclcpp/rclcpp.hpp>
# include <px4_msgs/msg/offboard_control_mode.hpp>
# include <mavros_msgs/msg/actuator_control.hpp>
# include <mavros_msgs/msg/override_rc_in.hpp>
# include <rclcpp/parameter_client.hpp>
# include <mavros_msgs/srv/param_set_v2.hpp>
# include <std_msgs/msg/string.hpp>
# include <px4_msgs/msg/vehicle_command.hpp>
# include <px4_msgs/msg/vehicle_control_mode.hpp>
# include <px4_msgs/msg/actuator_servos.hpp>

class ServoControlPublisher : public rclcpp::Node {
public:
    ServoControlPublisher(void);
private:
    int      _pwm;
    float    _pwm_nomallize;
	uint64_t _offboard_setpoint_counter;    //!< counter for the number of setpoints sent

    /* 2 */
    void _publish_arm_control_message(void);
    void _publish_pwm_output_message(void);
	void _publish_vehicle_command(uint16_t command, float param1 = 0.0, float param2 = 0.0);
    void _publish_offboard_control_mode();

    void _arm();
    void _disarm();




    // void _publish_disarm_control_message2(void);
    // void _publish_disarm_control_message_param(void);
    // void _set_mavros_param(const std::string& param_name, int param_value);
    // void _set_ros_param(const std::string& param_name, int param_value);
    // 복사 생성자와 할당 연산자 금지

    rclcpp::TimerBase::SharedPtr                                        _timer;
    std::shared_ptr<rclcpp::SyncParametersClient>                       _parameters_client;
	rclcpp::Publisher<px4_msgs::msg::VehicleCommand>::SharedPtr         _vehicle_command_publisher;
    rclcpp::Publisher<px4_msgs::msg::OffboardControlMode>::SharedPtr    _offboard_control_mode_publisher;
    rclcpp::Publisher<px4_msgs::msg::ActuatorServos>::SharedPtr         _publisher_arm;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr              _subscription;

    rclcpp::Publisher<mavros_msgs::msg::OverrideRCIn>::SharedPtr        _publisher_disarm;
    rclcpp::Client<mavros_msgs::srv::ParamSetV2>::SharedPtr _param_client;


};

#endif // SERVO_CONTROL_PUBLISHER_HPP
