#include "px4_ros_com/OffboardMavros.hpp"

void OffboardMavros::poseCallBack(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
    const double q1=msg->pose.orientation.x;
    const double q2=msg->pose.orientation.y;
    const double q3=msg->pose.orientation.z;
    const double q4=msg->pose.orientation.w;

    const double t1 = 2 *(q4*q3+q1*q2);
    const double t2 = 1 - 2 *(q2*q2+q3*q3);

    yaw_current = atan2(t1,t2) * vtol::RAD_2_DEG;
    // DEBUG::print("[Pose] Yaw current: ", yaw_current,GREEN);
}

void OffboardMavros::gpsCallBack(const sensor_msgs::msg::NavSatFix::SharedPtr msg) {
     if (msg->status.status >= sensor_msgs::msg::NavSatStatus::STATUS_FIX) {
            gps_locked = true;
        }

    if (isGlobalPositionGettingValue(init_global_position) == false) {
        init_global_position[vtol::ALT] = msg->altitude + 30.0f;
        init_global_position[vtol::LAT] = msg->latitude;
        init_global_position[vtol::LON] = msg->longitude;
        DEBUG::print("alt: ", init_global_position[vtol::ALT],BLUE);
        DEBUG::print("lat: ", init_global_position[vtol::LAT],BLUE);
        DEBUG::print("lon: ", init_global_position[vtol::LON],BLUE);
    }
  
    _global_position[vtol::ALT] = msg->altitude;
    _global_position[vtol::LAT] = msg->latitude;
    _global_position[vtol::LON] = msg->longitude;

    // DEBUG::print("gps_alt: ", _global_position[vtol::ALT], GREEN);
    // DEBUG::print("gps_lat: ", _global_position[vtol::LAT], GREEN);
    // DEBUG::print("gps_lon: ", _global_position[vtol::LON], GREEN);
}

void OffboardMavros::stateCallBack(const mavros_msgs::msg::State::SharedPtr msg) {
    if (isGlobalPositionGettingValue(_global_position) == false) {
        return;
    }
    fcu_state = *msg;

    DEBUG::message("\n[DEBUG] ", "-----------------", BOLDWHITE);
    DEBUG::print("System status : ", fcu_state.system_status, WHITE);
    DEBUG::print("Mode : ", fcu_state.mode, CYAN);
    DEBUG::printBool("Arming : ", fcu_state.armed, RED);
    DEBUG::printBinary("Command flag : ", _cmd_flag, YELLOW);
    DEBUG::print("Yaw current: ", yaw_current, GREEN);
    DEBUG::printArray("local_velocity: ", _local_velocity, 6, MAGENTA);
    DEBUG::printArray("global_position      : ", _global_position, 3, MAGENTA);
    DEBUG::printArray("init_global_position : ", init_global_position, 3, MAGENTA);
    DEBUG::message("[DEBUG] ", "-----------------\n", BOLDWHITE);

    const std::array<t_bit, vtol::STATE_SIZE>::iterator  it = std::find(state_value_array.begin(), state_value_array.end(), _cmd_flag);
//ros::Time::now() - last_request > ros::Duration(5.0)
    if (it == state_value_array.end()) {
        RCLCPP_ERROR(this->get_logger(), " Invalid State ");
        return;
    }
    const size_t i = std::distance(state_value_array.begin(), it);
    stateFunc[i]();
}

void OffboardMavros::localPositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
    _cur_position = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};
    if (_cmd_flag == vtol::TAKEOFF) {
        if (_global_position[vtol::ALT] > init_global_position[vtol::ALT] - 1) {
            _cmd_flag = vtol::FLY;
            _prev_position[vtol::NORTH] = _cur_position[vtol::NORTH];
            _prev_position[vtol::EAST] = _cur_position[vtol::EAST];
            DEBUG::print("Landing point North :", _prev_position[vtol::NORTH], BOLDYELLOW);
            DEBUG::print("Landing point East  :", _prev_position[vtol::EAST], BOLDYELLOW);
        }
    } else if (_cmd_flag == vtol::TO_FIXED) {
        DEBUG::print("North :", _cur_position[vtol::NORTH], WHITE);
        DEBUG::print("East  :", _cur_position[vtol::EAST], WHITE);
        if (_cur_position[vtol::NORTH] > _prev_position[vtol::NORTH] + 1 
            || _cur_position[vtol::EAST] > _prev_position[vtol::EAST] + 1) {
            DEBUG::print("Transition success North :", _cur_position[vtol::NORTH], BOLDYELLOW);
            DEBUG::print("Transition success East  :", _cur_position[vtol::EAST], BOLDYELLOW);
            _cmd_flag = vtol::FIXED;
        }
    } else if (_cmd_flag == vtol::TO_QUAD) {
        if (_cur_position[vtol::NORTH] - _prev_position[vtol::NORTH] < 0.1
            && _cur_position[vtol::EAST] - _prev_position[vtol::EAST] < 0.1) {
            DEBUG::print("Transition success North :", _cur_position[vtol::NORTH], BOLDYELLOW);
            DEBUG::print("Transition success East  :", _cur_position[vtol::EAST], BOLDYELLOW);
            // _cmd_flag &= ~vtol::BIT_TRANSITION;
            _cmd_flag = vtol::QUAD;
        }
        _prev_position[vtol::NORTH] = _cur_position[vtol::NORTH];
        _prev_position[vtol::EAST] = _cur_position[vtol::EAST];
    } else if (_cmd_flag == vtol::MC_START || _cmd_flag == vtol::FW_START) {
        localPositionCommandStart();
    }
}

void OffboardMavros::chatterCallback(const std_msgs::msg::String::SharedPtr msg) {
    RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());

    size_t i = 0;
    for (; i < OffboardMavros::_action_string_array.size() && msg->data != OffboardMavros::_action_string_array[i]; ++i);

    if (i == OffboardMavros::_action_string_array.size()) {
        std::cout << "Invalid input" << std::endl;
        return ;
    }
    OffboardMavros::actionFunc[i]();
    OffboardMavros::printReferenceInput();
}
