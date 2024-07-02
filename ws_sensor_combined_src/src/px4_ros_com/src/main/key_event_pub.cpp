#include "key_event/KeyPublisher.hpp"

int main(int argc, char **argv) {
	signal(SIGINT, KeyPublisher::sigint_handler);
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<KeyPublisher>());
    rclcpp::shutdown();
    return 0;
}
