#include "px4_ros_com/OffboardMavros.hpp"


bool OffboardMavros::isFiveSecondsPassed() {
return (this->now() - last_request).seconds() > 5.0;
}

void OffboardMavros::printSuccessInfo(bool success, const char* msg[]) const {
if (success) {
    RCLCPP_INFO(this->get_logger(), "%s", msg[vtol::SUCCESS]);
} else {
    RCLCPP_INFO(this->get_logger(), "%s", msg[vtol::FAIL]);
}
}

void OffboardMavros::printReferenceInput(void) {
    std::cout << "Position Command {"
        << _local_position[vtol::NORTH] << ", "
        << _local_position[vtol::EAST] << ", "
        << _local_position[vtol::UP] << "} (North, East, Up)"
        << std::endl;
}

bool OffboardMavros::isGlobalPositionGettingValue(const t_global_position& input_global_position) const{
    return (input_global_position[0] >= 0 && input_global_position[1] >= 0 && input_global_position[2] >= 0);
}

void OffboardMavros::commandFlagTurnOn(const t_bit& flag) {
    _cmd_flag |= flag;
}

void OffboardMavros::commandFlagTurnOff(const t_bit& flag) {
    _cmd_flag &= ~flag;
}
