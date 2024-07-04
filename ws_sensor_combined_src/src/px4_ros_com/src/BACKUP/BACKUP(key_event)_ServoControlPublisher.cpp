
#include "px4_ros_com/ServoControlPublisher.hpp"
#include <uuid/uuid.h>

ServoControlPublisher::ServoControlPublisher(void)
    // : Node("servo_control_publisher_" + generate_uuid()), pwm(800), pwm_nomallize(-1.0) {
    : Node("servo_control_publisher"), pwm(800), pwm_nomallize(-1.0) {
    _param_client = this->create_client<mavros_msgs::srv::ParamSetV2>("/mavros/param/set");
    _subscription = this->create_subscription<std_msgs::msg::String>( "chatter", 10,
            std::bind( &ServoControlPublisher::_chatterCallback, this, std::placeholders::_1
    ));
    // _timer = this->create_wall_timer(
    //         std::chrono::milliseconds(1000),
    //         std::bind(&ServoControlPublisher::_publishpwm_output_message, this));
}

void ServoControlPublisher::_publishpwm_output_message(void) {
    // _publish_arm_control_message();
    _publish_disarm_control_message();
    // _publish_disarm_control_message2();
    // _publish_disarm_control_message_param();
}

void ServoControlPublisher::_publish_disarm_control_message(void) {
    if (!_param_client->wait_for_service(std::chrono::seconds(1))) {
        RCLCPP_INFO(this->get_logger(), "Service /mavros/param/set not available, waiting...");
        return;
    }

    auto request = std::make_shared<mavros_msgs::srv::ParamSetV2::Request>();
    request->force_set = false;
    request->param_id = "PWM_AUX_DIS1";
    request->value.integer_value = pwm;
    request->value.type = rcl_interfaces::msg::ParameterType::PARAMETER_INTEGER;

    auto future = _param_client->async_send_request(request, std::bind(&ServoControlPublisher::_response_callback, this, std::placeholders::_1));
    // auto future = _param_client->async_send_request(request);
    // try {
    //     auto response = future.get();
    //     if (response->success) {
    //         RCLCPP_INFO(this->get_logger(), "Parameter %s set to %lu successfully.", request->param_id.c_str(), request->value.integer_value);
            // rclcpp::shutdown();
    //     } else {
    //         RCLCPP_ERROR(this->get_logger(), "Failed to set parameter %s.", request->param_id.c_str());
    //         rclcpp::shutdown();
    //     }
    // } catch (const std::exception& e) {
    //     RCLCPP_ERROR(this->get_logger(), "Service call failed: %s", e.what());
    // }
    // -> 이렇게 하니깐 안꺼짐
}

void ServoControlPublisher::_response_callback(rclcpp::Client<mavros_msgs::srv::ParamSetV2>::SharedFuture future) {
    auto response = future.get();
    std::cout << "response: " << response->success << std::endl;
    if (response->success) {
        RCLCPP_INFO(this->get_logger(), "Parameter set to successfully.");
        
        // // 모든 작업이 성공적으로 완료되었다면 노드 종료
        // RCLCPP_INFO(this->get_logger(), "Shutting down node due to successful operation.");

    } else {
        RCLCPP_ERROR(this->get_logger(), "Failed to set parameter.");
    }
}


std::string ServoControlPublisher::generate_uuid() {
    uuid_t uuid;
    char uuid_str[37]; // UUID 문자열 크기
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuid_str);

    std::string uuid_modified(uuid_str);
    std::replace(uuid_modified.begin(), uuid_modified.end(), '-', '_'); // 하이픈을 밑줄로 대체

    return uuid_modified;
}

void ServoControlPublisher::_chatterCallback(const std_msgs::msg::String::SharedPtr msg) {
    RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());

    switch (msg->data.c_str()[0]) {
        case 'w':
            pwm += (pwm < 2000) * 200;
            _publishpwm_output_message();
            break;
        case 's':
            pwm -= (pwm > 800) * 200;
            _publishpwm_output_message();
            break;
        default:
            break;
    }
}
