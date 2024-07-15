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


const Quaternion OffboardMavros::rpy_to_quat(const double roll, const double pitch, const double yaw) {
    const double cy = cos(yaw * 0.5);
    const double sy = sin(yaw * 0.5);
    const double cp = cos(pitch * 0.5);
    const double sp = sin(pitch * 0.5);
    const double cr = cos(roll * 0.5);
    const double sr = sin(roll * 0.5);

    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return q;
}
