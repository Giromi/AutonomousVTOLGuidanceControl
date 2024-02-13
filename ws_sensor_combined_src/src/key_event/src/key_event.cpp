#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>


class KeySubscriber : public rclcpp::Node {
public:
    KeySubscriber() : Node("key_subscriber")
    {
        subscription_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "cmd_vel", 10, std::bind(&KeySubscriber::chatterCallback, this, std::placeholders::_1));
    }

private:
    void chatterCallback(const geometry_msgs::msg::Twist::ConstSharedPtr& msg) {
    // void chatterCallback(const std_msgs::msg::String::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "Linear  x: '%f'", msg->linear.x);
        RCLCPP_INFO(this->get_logger(), "Linear  y: '%f'", msg->linear.y);
        RCLCPP_INFO(this->get_logger(), "Linear  z: '%f'", msg->linear.z);
        RCLCPP_INFO(this->get_logger(), "Angular x: '%f'", msg->angular.x);
        RCLCPP_INFO(this->get_logger(), "Angular y: '%f'", msg->angular.y);
        RCLCPP_INFO(this->get_logger(), "Angular z: '%f'", msg->angular.z);

        // if (msg->data == "u") {
        //     RCLCPP_INFO(this->get_logger(), "Forward");
        // } else if (msg->data == "i") {
        //     RCLCPP_INFO(this->get_logger(), "Backward");
        // } else if (msg->data == "a") {
        //     RCLCPP_INFO(this->get_logger(), "Left");
        // } else if (msg->data == "d") {
        //     RCLCPP_INFO(this->get_logger(), "Right");
        // } else if (msg->data == "q") {
        //     RCLCPP_INFO(this->get_logger(), "Stop");
        // } else {
        //     RCLCPP_INFO(this->get_logger(), "Invalid Key");
        // }
    }
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscription_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<KeySubscriber>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

