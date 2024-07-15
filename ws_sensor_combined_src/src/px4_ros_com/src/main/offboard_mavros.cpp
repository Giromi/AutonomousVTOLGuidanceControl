#include <rclcpp/rclcpp.hpp>
#include "px4_ros_com/OffboardMavros.hpp"

int main(int argc, char* argv[]) {

    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OffboardMavros>());
    rclcpp::shutdown();
    return 0;
}

