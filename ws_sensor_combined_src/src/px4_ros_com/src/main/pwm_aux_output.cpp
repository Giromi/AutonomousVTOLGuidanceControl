#include "px4_ros_com/ServoControlPublisher.hpp"

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ServoControlPublisher>());
    rclcpp::shutdown();
    return 0;
}
