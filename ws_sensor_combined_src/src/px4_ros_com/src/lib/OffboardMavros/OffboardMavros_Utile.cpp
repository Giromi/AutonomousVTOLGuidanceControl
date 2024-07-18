#include "px4_ros_com/OffboardMavros.hpp"

bool OffboardMavros::isPassedSeconds(const double timer) {
    const double clock = (this->now() - last_request).seconds();
    RCLCPP_INFO(this->get_logger(), "isPassed %lf > %lf Seconds?", clock, timer);
    return (this->now() - last_request).seconds() > timer;
}

void OffboardMavros::printSuccessInfo(const bool success, const std::array<const std::string, 2>& msg) const {
    RCLCPP_INFO(this->get_logger(), "%s", (success) ? msg[vtol::SUCCESS].c_str() 
                                                    : msg[vtol::FAIL].c_str());
}

void OffboardMavros::handleCommandFlag(const t_bit flag) {
    if (_stt_cmd_flag == flag) {
        return ;
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
    return (input_global_position[0] >= 0 
            && input_global_position[1] >= 0 
            && input_global_position[2] >= 0);
}

void OffboardMavros::commandFlagTurnOn(const t_bit& flag) {
    _stt_cmd_flag |= flag;
}

void OffboardMavros::commandFlagTurnOff(const t_bit& flag) {
    _stt_cmd_flag &= ~flag;
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

bool OffboardMavros::ifTimeNotSameInput(builtin_interfaces::msg::Time& t1, const builtin_interfaces::msg::Time& t2) {
    if (t1.sec == t2.sec && t1.nanosec == t2.nanosec) return true;
    t1 = t2;
    return false;
}

bool    OffboardMavros::isConnectionSafe(void) {

    if (fcu.state.second.header.stamp.sec == 0
        || fcu.extended_state.second.header.stamp.sec == 0
        || fcu.local_position.second.header.stamp.sec == 0
        || fcu.global_position.second.header.stamp.sec == 0
    ) {
        DEBUG::message("Subscribe Call back not yet", YELLOW);
        return false;
    }
    return true;
}
