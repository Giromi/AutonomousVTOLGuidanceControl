#include "px4_ros_com/OffboardMavros.hpp"

/* ---------------------- State Command Callback ---------------------- */

void OffboardMavros::statusCommandStandBy(void){
    RCLCPP_INFO(this->get_logger(), "< State Command Stand By >");
    // 처음은 바로 check로 넘어가기 때문에 착륙시 여기서 다시 초기화
    init_global_position = {-1.0f, -1.0f, -1.0f};
    // _stt_cmd_flag = vtol::CHECK;
}

void OffboardMavros::statusCommandCheck(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Ready >");
    publishGpOrigin();


    if (fcu.extended_state.second.landed_state 
        == mavros_msgs::msg::ExtendedState::LANDED_STATE_IN_AIR) {
        updateLandingStatus();
    } else if (
        fcu.state.second.system_status 
            == vtol::mavlink::State::STANDBY
        && fcu.extended_state.second.landed_state 
            == mavros_msgs::msg::ExtendedState::LANDED_STATE_ON_GROUND) {
        updateDisarmingStatus();
        updateHoldMode();
        updateWaypointClear();
    }
}

void OffboardMavros::statusCommandReady(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Ready >");
    /** system status stand by 이면 Ariming으로 바꿈 */ 
    updateHoldMode(); 
}

void OffboardMavros::statusCommandArming(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Arming >");
    updateArmingStatus();
}

void OffboardMavros::statusCommandArmed (void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Armed >");
    /** Arming으로 바뀌는 것은 bitHanlder 로감
     *  't' 누르면 시작
     */
}
void OffboardMavros::statusCommandTakingOff(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Taking Off >");

    if (fcu.extended_state.second.vtol_state
            != mavros_msgs::msg::ExtendedState::VTOL_STATE_MC) {
        DEBUG::message("VTOL_STATE_MC", BOLDGREEN);
        return ;
    }

    /** Takeoff 상태에서 Armed false 되었을 때 
     *  안그럼 다시 t 눌러야 함
     * */

    if (fcu.state.second.armed == false) {
        updateArmingStatus();
    }

    if (fcu.state.second.mode
         != mavros_msgs::msg::State::MODE_PX4_TAKEOFF
        && fcu.extended_state.second.landed_state 
         == mavros_msgs::msg::ExtendedState::LANDED_STATE_ON_GROUND) {
        updateTakeoffMode();
        updateTakeoffStatus();
    } else if (fcu.extended_state.second.landed_state
                == mavros_msgs::msg::ExtendedState::LANDED_STATE_TAKEOFF) {
        DEBUG::message("Taking off ... ", BOLDGREEN);
    } else if (
            fcu.state.second.mode == vtol::FCU_HOLD
            && fcu.extended_state.second.landed_state 
                == mavros_msgs::msg::ExtendedState::LANDED_STATE_IN_AIR
            && fcu.global_position.second.altitude 
                >= init_global_position[vtol::ALT] - 1.0) {
        /** HOLD 상태로 돌아가야 take off 성공한거임 */
        DEBUG::message("Taking off sucess", BOLDGREEN);
        _stt_cmd_flag = vtol::MC;
    }

    /** 현재 Takeoff 상태가 아니고 착륙 상태가 ON_GROUND 일때 */
}

// void OffboardMavros::statusCommandTakingOffMC(void) {
//     RCLCPP_INFO(this->get_logger(), "< State Command Taking Off >");
//
//     /* holding success으로 하면 안됨 */
//     if (fcu.state.second.mode == vtol::FCU_HOLD
//         && fcu.extended_state.second.vtol_state 
//             == mavros_msgs::msg::ExtendedState::VTOL_STATE_MC 
//         && fcu.extended_state.second.landed_state 
//             == mavros_msgs::msg::ExtendedState::LANDED_STATE_IN_AIR) {
//         _stt_cmd_flag = vtol::MC;
//     }
// }


void OffboardMavros::statusCommandMC(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command MC >");
    if (fcu.extended_state.second.landed_state  
            != mavros_msgs::msg::ExtendedState::LANDED_STATE_IN_AIR) {
        DEBUG::message("Not in the air", BOLDRED);
        return ;
    }

    if (fcu.state.second.mode == vtol::FCU_HOLD) {
        if (std::isnan(wp_manager.getTarget()[vtol::YAW])) {
            updatePositionMode();   // Success : MC -> MC_TO_FIXED
        } else {
            updateHoldMode();       // Success : MC -> MC_STARTING
        }
    } 
}

void OffboardMavros::statusCommandToFixedMC(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command To Fixed >");
    if (fcu.state.second.mode 
        != mavros_msgs::msg::State::MODE_PX4_POSITION) {
        return ;
        DEBUG::message("Position mode is not set", BOLDRED);
    } else if (fcu.extended_state.second.landed_state  
            != mavros_msgs::msg::ExtendedState::LANDED_STATE_IN_AIR) {
        DEBUG::message("Not in the air", BOLDRED);
        return ;
    } 
    if (fcu.extended_state.second.vtol_state
         == mavros_msgs::msg::ExtendedState::VTOL_STATE_MC) {
        updateTransitionFixedStatus();   
        DEBUG::message("VTOL_STATE_MC", BOLDGREEN);
    } else if (
        fcu.extended_state.second.vtol_state
         == mavros_msgs::msg::ExtendedState::VTOL_STATE_TRANSITION_TO_FW) {
        DEBUG::message("VTOL_STATE_TRANSITION_TO_FW", BOLDGREEN);
        _stt_cmd_flag = vtol::FW;
    } 
}

void OffboardMavros::statusCommandFW(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Fixed >");
    updatePositionMode();   // Success FW -> FW_STARTING;
}


void OffboardMavros::statusCommandStartingFW(void) { 
    RCLCPP_INFO(this->get_logger(), "< State Command FW Starting >");
    // if (fcu.state.second.mode != vtol::FCU_POSITION) {
    //     DEBUG::message("Not in the position mode", BOLDRED);
    //     return ;
    //
    if (fcu.state.second.mode 
          != mavros_msgs::msg::State::MODE_PX4_OFFBOARD) {
        updateOffboardMode();
        publishRawLocalPosition();
    } else {
        _stt_cmd_flag = vtol::FW_MISSION;
    }
    // } else if (fcu.state.second.mode 
    //       != mavros_msgs::msg::State::MODE_PX4_POSITION) {
    //     updatePositionMode();   // Success FW -> FW_STARTING;
}

void OffboardMavros::statusCommandMissionFW(void) { 
    RCLCPP_INFO(this->get_logger(), "< State Command FW Mission >");
    // if (fcu.state.second.mode != vtol::FCU_OFFBOARD) {
    //     DEBUG::message("Not in the offboard mode", BOLDRED);
    //     return ;
    // }
    // publishRawLocalPosition();
    // publishPose();
    // publishCmdVel();
    // publishVelocity(); // 
    // publishManual();
    // publishRawAttitude();
    // publishAttitudePosition();
    // publishRawLocal();
    // if (fcu.state.second.mode == "AUTO.MISSION") {
    // publishWaypoint();
    // } else {
    //     publishRawLocal(); // publishLocalFixed();
    // }
    // if (std::isnan(wp_manager.getTarget()[vtol::YAW])) {
    localPositionWaypointsMission();
    // }
    // } else {
    //     updatePositionMode();           // Sucess : FW_MISSION -> FW_STOPPING
    // }
}
    
void OffboardMavros::statusCommandStopingFW(void) { 
    RCLCPP_INFO(this->get_logger(), "< State Command FW Stoping >");
    if (fcu.state.second.mode != vtol::FCU_POSITION) {
        DEBUG::message("Not in the position mode", BOLDRED);
        return ;
    }

    if (fcu.extended_state.second.vtol_state
         == mavros_msgs::msg::ExtendedState::VTOL_STATE_FW) {
        updateHoldMode();
    }
}

void OffboardMavros::statusCommandToQuadFW(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command To Quad >");
    if (fcu.extended_state.second.vtol_state
         == mavros_msgs::msg::ExtendedState::VTOL_STATE_FW) {
        DEBUG::message("VTOL_STATE_FW", BOLDGREEN);
        updateTransitionQuadStatus();
    } else if (
        fcu.extended_state.second.vtol_state
         == mavros_msgs::msg::ExtendedState::VTOL_STATE_TRANSITION_TO_MC) {
        DEBUG::message("VTOL_STATE_TRANSITION_TO_MC", BOLDGREEN);
        updateHoldMode();
    } 
}

void OffboardMavros::statusCommandStartingMC(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command FW Starting >");
    if (fcu.state.second.mode != vtol::FCU_HOLD) {
        return ;
    }
    updateOffboardMode();
}


void OffboardMavros::statusCommandMissionMC(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command MC Mission >");
    if (wp_manager.getSize()) {
        localPositionWaypointsMission();
    } else {
        updateHoldMode();
    }
}

void OffboardMavros::statusCommandStopingMC(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command FW Starting >");
    if (fcu.state.second.mode != vtol::FCU_POSITION) {
        return ;
    }
    updateHoldMode();
}


void OffboardMavros::statusCommandLanding(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Land >");
    if (fcu.state.second.mode != vtol::FCU_LAND && fcu.state.second.armed == true) {
        updateLandingStatus();
    } else if (fcu.state.second.mode == vtol::FCU_HOLD) {
        std::cout << "Landing success" << std::endl;
        updateDisarmingStatus();
    }
}

void OffboardMavros::statusCommandDisarming(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Disarming >");
    if (fcu.state.second.armed == true) {
        updateDisarmingStatus();
    }
    _stt_cmd_flag = vtol::READY;
}


// void OffboardMavros::stateCommandMavrosWaypointsMission(void) {
//     RCLCPP_INFO(this->get_logger(), "< State Command Mission >");
//     if (fcu.state.second.armed == false) {
//         RCLCPP_ERROR(this->get_logger(), "Need to arm Mission mode");
//         _stt_cmd_flag = vtol::READY;
//         return ;
//     }
//
//     if (fcu.state.second.mode != vtol::FCU_MISSION) {
//         updateWaypointPush();
//         updateMissionMode();
//     }
// }

/* --------------------- localPosition Command Callback --------------------- */

void OffboardMavros::localPositionWaypointsMission(void) {
    const Eigen::Vector4d cur_position_vector3d(
        fcu.local_position.second.pose.position.x,
        fcu.local_position.second.pose.position.y,
        fcu.local_position.second.pose.position.z,
        vtol::NaN
    );
    if (wp_manager.isArrived(cur_position_vector3d)) {
        wp_manager.pop();
    }
}
