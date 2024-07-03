
#include "px4_ros_com/ServoControlPublisher.hpp"

int ServoControlPublisher::pwm = 800;
float ServoControlPublisher::pwm_nomallize = -1.0;

ServoControlPublisher::ServoControlPublisher(void) : Node("servo_control_publisher") {
    _publisher_arm = this->create_publisher<mavros_msgs::msg::ActuatorControl>("/mavros/actuator_control", 10);
    _publisher_disarm = this->create_publisher<mavros_msgs::msg::OverrideRCIn>("/mavros/rc/override", 10);
    _param_client = this->create_client<rcl_interfaces::srv::SetParameters>("/mavros/param/set");
    _parameters_client = std::make_shared<rclcpp::SyncParametersClient>(this, "/mavros/param");  // 파라미터 클라이언트 초기화
    _timer = this->create_wall_timer(
            std::chrono::milliseconds(1000),
            std::bind(&ServoControlPublisher::_publishpwm_output_message, this));
}

void ServoControlPublisher::_publishpwm_output_message(void) {
    _publish_arm_control_message();
    // _publish_disarm_control_message();
    // _publish_disarm_control_message2();
    // _publish_disarm_control_message_param();
}

void ServoControlPublisher::_publish_arm_control_message(void) {
    if (ServoControlPublisher::pwm < 1)
        ServoControlPublisher::pwm += 0.1;
    auto message = mavros_msgs::msg::ActuatorControl();
    message.header.stamp = this->get_clock()->now();
    message.header.frame_id = "camera_servo";
    message.group_mix = 2;  // Use group 2 for AUX channels in PX4
    message.controls[0] = 1;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    ServoControlPublisher::pwm_nomallize += (ServoControlPublisher::pwm_nomallize < 0.9) * 0.1;
    std::cout << "Publishing arm control message" << pwm_nomallize << std::endl;
    _publisher_arm->publish(message);
}

void ServoControlPublisher::_publish_disarm_control_message(void) {

    auto message = mavros_msgs::msg::OverrideRCIn();
    message.channels[0] = 1000; // AUX1 채널에 PWM 값을 보냅니다.
    message.channels[16] = 1000; // AUX1 채널에 PWM 값을 보냅니다.
    std::cout << "Publishing disarm control message" << std::endl;
    _publisher_disarm->publish(message);
}

void ServoControlPublisher::_publish_disarm_control_message2(void) {
    if (!_param_client->wait_for_service(std::chrono::seconds(1))) {
        RCLCPP_ERROR(this->get_logger(), "Service /mavros/param/set not available.");
        return;
    }

    auto request = std::make_shared<rcl_interfaces::srv::SetParameters::Request>();
    rcl_interfaces::msg::Parameter p;
    p.name = "PWM_AUX_DIS1";
    p.value.type = rcl_interfaces::msg::ParameterType::PARAMETER_INTEGER;
    p.value.integer_value = 2000;
    request->parameters.push_back(p);
    auto future_result = _param_client->async_send_request(request);
    // Wait for the result.
    if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), future_result) ==
        rclcpp::FutureReturnCode::SUCCESS)
    {
        auto response = future_result.get();
        if (!response->results.empty() && response->results[0].successful) {
            RCLCPP_INFO(this->get_logger(), "PWM_AUX_DIS1 set to 2000 successfully.");
        } else {
            RCLCPP_ERROR(this->get_logger(), "Failed to set PWM_AUX_DIS1.");
        }
    } else {
        RCLCPP_ERROR(this->get_logger(), "Failed to call service /mavros/param/set");
    }
}

void ServoControlPublisher::_publish_disarm_control_message_param(void) {
    std::cout << "Publishing disarm control message " << pwm << std::endl;
    // _set_mavros_param("PWM_AUX_DIS1", pwm);
    _set_ros_param("PWM_AUX_DIS1", pwm);
    pwm += (ServoControlPublisher::pwm < 2000) * 100;
}

void ServoControlPublisher::_set_mavros_param(const std::string& param_name, int param_value) {
    auto request = std::make_shared<mavros_msgs::srv::ParamSet::Request>();
    request->param_id = param_name; // 파라미터 이름 설정
    request->value.integer = param_value; // 정수 값 설정
    request->value.real = 0.0; // 실수 값은 사용하지 않으므로 0.0으로 설정

    // 서비스 호출이 성공적으로 이루어졌는지 확인
    // auto result_future = _param_client->async_send_request(request);
    std::cout << "Set mavros param " << param_name << " to " << param_value << std::endl;
    // 응답을 기다리거나 처리하는 로직을 추가할 수 있습니다.
    // this->set_parameter(rclcpp::Parameter(param_name, param_value));
    // 응답을 기다리거나 처리하는 로직을 추가할 수 있습니다.
    // auto response = result_future.get();

    // if (response->success) {
    //     RCLCPP_INFO(this->get_logger(), "Set parameter %s to %d successfully", param_name.c_str(), param_value);
    // } else {
    //     RCLCPP_ERROR(this->get_logger(), "Failed to set parameter %s", param_name.c_str());
    // }
    // if (rclcpp::spin_until_future_complete(this->get_node_base_interface(), result_future) == rclcpp::FutureReturnCode::SUCCESS) {
    //     if (response->success) {
    //         RCLCPP_INFO(this->get_logger(), "Set parameter %s to %d successfully", param_name.c_str(), param_value);
    //     } else {
    //         RCLCPP_ERROR(this->get_logger(), "Failed to set parameter %s", param_name.c_str());
    //     }
    // } else {
    //     RCLCPP_ERROR(this->get_logger(), "Service call failed");
    // }
    // 설정한 파라미터 값을 확인하기 위해 로그를 출력합니다.
    // RCLCPP_INFO(this->get_logger(), "Set parameter %s to %d", 
    //                 param_name.c_str(), param_value);
}

void ServoControlPublisher::_set_ros_param(const std::string& param_name, int param_value) {
    if (!_parameters_client->wait_for_service(std::chrono::seconds(5))) {
        RCLCPP_ERROR(this->get_logger(), "Parameters service not available.");
        return;
    }

    std::cerr << "1 Set ros param " << param_name << " to " << param_value << std::endl;

    auto results = _parameters_client->set_parameters({
        rclcpp::Parameter(param_name, param_value) 
    });

    std::cerr << "2 Set ros param " << param_name << " to " << param_value << std::endl;

    for (const auto& result : results) {
        if (!result.successful) {
            RCLCPP_ERROR(this->get_logger(), "Failed to set parameter: %s", result.reason.c_str());
            return ; // Optional: Stop checking after the first failure
        }
    }

    std::cerr << "3 Set ros param " << param_name << " to " << param_value << std::endl;

    RCLCPP_INFO(this->get_logger(), "Parameter %s set to %d", param_name.c_str(), param_value);
}

