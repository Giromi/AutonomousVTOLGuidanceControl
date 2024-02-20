#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <mavros_msgs/srv/command_bool.hpp>
#include <mavros_msgs/srv/set_mode.hpp>
#include <mavros_msgs/msg/state.hpp>

class OffbNode : public rclcpp::Node {
public:
    OffbNode() : Node("offb_node") {
        state_sub = this->create_subscription<mavros_msgs::msg::State>(
            "mavros/state", 10, std::bind(&OffbNode::state_cb, this, std::placeholders::_1));

        local_pos_pub = this->create_publisher<geometry_msgs::msg::PoseStamped>(
            "mavros/setpoint_position/local", 10);

        arming_client = this->create_client<mavros_msgs::srv::CommandBool>(
            "mavros/cmd/arming");

        set_mode_client = this->create_client<mavros_msgs::srv::SetMode>(
            "mavros/set_mode");

        _timer = this->create_wall_timer(
            std::chrono::milliseconds(50), std::bind(&OffbNode::timer_callback, this));
    }

private:
    void state_cb(const mavros_msgs::msg::State::SharedPtr msg) {
        current_state = *msg;
    }

    void timer_callback() {
        if (!current_state.connected) {
            return;
        }

        if (current_state.mode != "OFFBOARD") {
            // 요청을 OFFBOARD 모드로 변경
            auto request = std::make_shared<mavros_msgs::srv::SetMode::Request>();
            request->custom_mode = "OFFBOARD";

            auto result = set_mode_client->async_send_request(request);
            // Handle response in a callback or using a future
            last_request = this->now();
        } else if (!current_state.armed) {
            // 요청을 ARM
            auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
            request->value = true;

            auto result = arming_client->async_send_request(request);
            // Handle response in a callback or using a future
            last_request = this->now();
        } else {
            // OFFBOARD 모드이고 ARMED인 경우에만 명령 전송
            geometry_msgs::msg::PoseStamped pose;
            pose.pose.position.x = 0;
            pose.pose.position.y = 0;
            pose.pose.position.z = 2;

            local_pos_pub->publish(pose);
        }
    }

    rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr state_sub;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr local_pos_pub;
    rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr arming_client;
    rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr set_mode_client;
    mavros_msgs::msg::State current_state;
    rclcpp::Time last_request = rclcpp::Time(0);
    rclcpp::TimerBase::SharedPtr _timer;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<OffbNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
