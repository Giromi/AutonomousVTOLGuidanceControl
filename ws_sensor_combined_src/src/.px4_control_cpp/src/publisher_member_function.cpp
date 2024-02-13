#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <termios.h>
#include <unistd.h>
#include <thread>
#include <iostream>

class MinimalPublisher : public rclcpp::Node {
public:
    MinimalPublisher() : Node("minimal_publisher") {
        publisher_ = this->create_publisher<std_msgs::msg::String>("topic", 10);
        input_thread_ = std::thread(&MinimalPublisher::getInput, this);
    }

    ~MinimalPublisher() {
        input_thread_.join();
    }

private:
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    std::thread input_thread_;

    void getInput() {
        char ch;
        struct termios oldt, newt;

        // Get current terminal settings
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;

        // Disable buffered IO
        newt.c_lflag &= ~(ICANON | ECHO);

        // Set new terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        while (rclcpp::ok()) {
            ch = getchar();

            if (ch == 'e') {
                break;
            } else {
                auto message = std_msgs::msg::String();
                message.data = std::string(1, ch);
                RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
                publisher_->publish(message);
            }
        }

        // Restore terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MinimalPublisher>();
    RCLCPP_INFO(node->get_logger(), "Start spin");
    rclcpp::spin(node);
    RCLCPP_INFO(node->get_logger(), "End spin");
    rclcpp::shutdown();
    return 0;
}

