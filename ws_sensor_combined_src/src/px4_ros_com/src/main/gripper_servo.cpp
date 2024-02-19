#include <rclcpp/rclcpp.hpp>
#include <mavros_msgs/srv/command_long.hpp>

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("aux_channel_control");

    // 서비스 클라이언트 설정
    auto client = node->create_client<mavros_msgs::srv::CommandLong>("/mavros/cmd/command");

    // 서비스 호출 메시지 설정
    auto request = std::make_shared<mavros_msgs::srv::CommandLong::Request>();
    request->broadcast = false;
    request->command = 183; // MAV_CMD_DO_SET_SERVO
    request->param1 = 1.0; // servo 번호 (0~5까지)
    request->param2 = 1000.0; // PWM 값 설정

    // 서비스 호출
    while (!client->wait_for_service(std::chrono::seconds(1))) {
        if (!rclcpp::ok()) {
            RCLCPP_ERROR(node->get_logger(), "Interrupted while waiting for the service. Exiting.");
            return 1;
        }
        RCLCPP_INFO(node->get_logger(), "Service not available, waiting again...");
    }

    auto result_future = client->async_send_request(request);

    // 서비스 호출 결과 처리
    if (rclcpp::spin_until_future_complete(node, result_future) == rclcpp::FutureReturnCode::SUCCESS) {
        auto result = result_future.get();
        if (result->success) {
            RCLCPP_INFO(node->get_logger(), "Aux channel command sent successfully");
        } else {
            RCLCPP_ERROR(node->get_logger(), "Failed to call service: %d", result->result);
            return 1;
        }
    } else {
        RCLCPP_ERROR(node->get_logger(), "Failed to call service");
        return 1;
    }

    return 0;
}
