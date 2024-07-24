#include "px4_ros_com/OffboardMavros.hpp"

void OffboardMavros::stateCallBack(
    const mavros_msgs::msg::State::SharedPtr msg
) {
    fcu.state.second = *msg;
    //
    // if (isGlobalPositionGettingValue(_global_position) == false) {
    //     return;
    // }
    //
    // if(msg->system_status != vtol::mavlink::SystemStatus::STANDBY) {
    //     _stt_cmd_flag = vtol::STAND_BY;
    // }

    // stateCallBackModeHandler();

}
//
// void    OffboardMavros::stateCallBackModeHandler(const std::string& mode) {
//     switch (mode) {
//         case State::MODE_PX4_LOITER;
//             break;
//         case State::MODE_PX4_TAKEOFF:
//             break;
//         case State::MODE_PX4_READY:
//             break;
//         case State::MODE_PX4_OFFBOARD;
//             break;
//         case State::MODE_PX4_POSITION;
//             break;
//         case State::MODE_PX4_STABILIZED:
//             break;
//     }
// }
//
void OffboardMavros::extendedStateCallBack(
    const mavros_msgs::msg::ExtendedState::SharedPtr msg
) {
    fcu.extended_state.second = *msg; 
    switch (msg->vtol_state) {
        case mavros_msgs::msg::ExtendedState::VTOL_STATE_UNDEFINED:
            break;
        case mavros_msgs::msg::ExtendedState::VTOL_STATE_TRANSITION_TO_FW:
            if (_stt_cmd_flag == vtol::MC_TO_FIXED) {
                _stt_cmd_flag |= vtol::CMD_TRANSITION;
            }
            break;
        case mavros_msgs::msg::ExtendedState::VTOL_STATE_TRANSITION_TO_MC:
            if (_stt_cmd_flag == vtol::FW_TO_QUAD) {
                _stt_cmd_flag |= vtol::CMD_TRANSITION;
            }
            break;
        case mavros_msgs::msg::ExtendedState::VTOL_STATE_MC:
            _stt_cmd_flag &= ~vtol::STT_FIXED;
            break;
        case mavros_msgs::msg::ExtendedState::VTOL_STATE_FW:
            _stt_cmd_flag |= vtol::STT_FIXED;
            break;
    }

}


void OffboardMavros::poseCallBack(
    const geometry_msgs::msg::PoseStamped::SharedPtr msg
) {
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
    fcu.global_position.second = *msg;

     if (msg->status.status >= sensor_msgs::msg::NavSatStatus::STATUS_FIX) {
            gps_locked = true;
        }

    if (isGlobalPositionGettingValue(init_global_position) == false) {
        init_global_position[vtol::ALT] = msg->altitude + vtol::INIT_UP;
        init_global_position[vtol::LAT] = msg->latitude;
        init_global_position[vtol::LON] = msg->longitude;
        DEBUG::print("alt: ", init_global_position[vtol::ALT],BLUE);
        DEBUG::print("lat: ", init_global_position[vtol::LAT],BLUE);
        DEBUG::print("lon: ", init_global_position[vtol::LON],BLUE);
    }
  
    // _global_position[vtol::ALT] = msg->altitude;
    // _global_position[vtol::LAT] = msg->latitude;
    // _global_position[vtol::LON] = msg->longitude;

    // DEBUG::print("gps_alt: ", _global_position[vtol::ALT], GREEN);
    // DEBUG::print("gps_lat: ", _global_position[vtol::LAT], GREEN);
    // DEBUG::print("gps_lon: ", _global_position[vtol::LON], GREEN);
}

void OffboardMavros::localPositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
    /* 예전 timestamp 기록 해놓고, 현재 timestamp에 현재 위치 저장 */
    fcu.local_position.second = *msg;
    _cur_position = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};
    // if (_stt_cmd_flag == vtol::TAKING_OFF) {
        ;
        // if (_global_position[vtol::ALT] > init_global_position[vtol::ALT] - 1) {
        //     _stt_cmd_flag = vtol::MC;
        //     _prev_position[vtol::NORTH] = _cur_position[vtol::NORTH];
        //     _prev_position[vtol::EAST] = _cur_position[vtol::EAST];
        //     DEBUG::print("Landing point North :", _prev_position[vtol::NORTH], BOLDYELLOW);
        //     DEBUG::print("Landing point East  :", _prev_position[vtol::EAST], BOLDYELLOW);
        // }
    // } else if (_stt_cmd_flag == vtol::MC_TO_FIXED) {
    //     DEBUG::print("North :", _cur_position[vtol::NORTH], WHITE);
    //     DEBUG::print("East  :", _cur_position[vtol::EAST], WHITE);
    //     if (_cur_position[vtol::NORTH] > _prev_position[vtol::NORTH] + 1 
    //         || _cur_position[vtol::EAST] > _prev_position[vtol::EAST] + 1) {
    //         DEBUG::print("Transition success North :", _cur_position[vtol::NORTH], BOLDYELLOW);
    //         DEBUG::print("Transition success East  :", _cur_position[vtol::EAST], BOLDYELLOW);
    //         _stt_cmd_flag = vtol::FW;
    //     }
    // } else if (_stt_cmd_flag == vtol::FW_TO_QUAD) {
    //     // if (_cur_position[vtol::NORTH] - _prev_position[vtol::NORTH] < 0.1
    //     //     && _cur_position[vtol::EAST] - _prev_position[vtol::EAST] < 0.1) {
    //     //     DEBUG::print("Transition success North :", _cur_position[vtol::NORTH], BOLDYELLOW);
    //     //     DEBUG::print("Transition success East  :", _cur_position[vtol::EAST], BOLDYELLOW);
    //         // _stt_cmd_flag &= ~vtol::BIT_TRANSITION;
    //         // _stt_cmd_flag = vtol::MC;
    //     }
    //     _prev_position[vtol::NORTH] = _cur_position[vtol::NORTH];
    //     _prev_position[vtol::EAST] = _cur_position[vtol::EAST];
    // } else if (_stt_cmd_flag == vtol::MC_MISSION 
    //         || _stt_cmd_flag == vtol::FW_MISSION) {
    //     localPositionCommandStart();
    // }
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

