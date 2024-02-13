#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

class KeySubscriber : public rclcpp::Node {
public:
    KeySubscriber() : Node("key_subscriber") {
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "chatter", 10, std::bind(
                &KeySubscriber::chatterCallback, 
                this, 
                std::placeholders::_1
        ));
    }

private:
    void chatterCallback(const std_msgs::msg::String::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());
    }
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<KeySubscriber>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
