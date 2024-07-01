#include "px4_ros_com/OffboardMavros.hpp"


bool OffboardMavros::is_five_seconds_passed() {
return (this->now() - last_request_).seconds() > 5.0;
}

void OffboardMavros::print_success_info(bool success, const char* msg[]) const {
if (success) {
    RCLCPP_INFO(this->get_logger(), "%s", msg[vtol::SUCCESS]);
} else {
    RCLCPP_INFO(this->get_logger(), "%s", msg[vtol::FAIL]);
}
}

void OffboardMavros::print_reference_input(void) {
    std::cout << "Position Command {"
        << local_position_[vtol::NORTH] << ", "
        << local_position_[vtol::EAST] << ", "
        << local_position_[vtol::UP] << "} (North, East, Up)"
        << std::endl;
}