#include "px4_ros_com/OffboardMavros.hpp"

/* ---------------------- Mode Sent Response Callback ---------------------- */
// void OffboardMavros::holdModeSuccessBitHandler(void) {
//     /* 이거 주석해제 하면 Offboard mavros 시작 시 자동으로 takeoff */
//     // } else if (_stt_cmd_flag == vtol::ARMED) {
//     //     _stt_cmd_flag = vtol::TAKING_OFF; 
//     // }
// }
//
// void OffboardMavros::holdModeFailureBitHandler(void) {
// }

void OffboardMavros::holdModeResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2>& msg) {
    const bool result = future.get()->mode_sent;
    printSuccessInfo(result, msg);
    if (result == true) {
        if (_stt_cmd_flag == vtol::READY) {
            _stt_cmd_flag = vtol::ARMING;
        } else if (_stt_cmd_flag == vtol::MC) {
            _stt_cmd_flag = vtol::MC_STARTING;
        } else if (_stt_cmd_flag == vtol::MC_MISSION) {
            _stt_cmd_flag = vtol::MC_STOPPING;
        } else if (_stt_cmd_flag == vtol::FW_TO_QUAD) {
            _stt_cmd_flag = vtol::MC;
        }
    } else {
        if (_stt_cmd_flag == vtol::READY) {
            _stt_cmd_flag = isPassedSeconds(5) ? static_cast<t_bit>(vtol::ARMING) 
                                               : _stt_cmd_flag;
        } else if (_stt_cmd_flag == vtol::MC) {
            _stt_cmd_flag = isPassedSeconds(5) ? static_cast<t_bit>(vtol::READY)
                                               : _stt_cmd_flag;
        }
    }
}

void OffboardMavros::offboardModeResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2>& msg) {
    const bool result = future.get()->mode_sent;
    printSuccessInfo(result, msg);

    if (result == true) {
        if (_stt_cmd_flag == vtol::FW_STARTING) {
            _stt_cmd_flag = vtol::FW_MISSION;
        } else if (_stt_cmd_flag == vtol::MC_STARTING) {
            _stt_cmd_flag = vtol::MC_MISSION;
        }
        RCLCPP_INFO(this->get_logger(), "Mode sent successfully");
    } 
    else {
        RCLCPP_ERROR(this->get_logger(), "Failed to send mode");
    }
}

void OffboardMavros::positionModeResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2>& msg) {
    const bool result = future.get()->mode_sent;
    printSuccessInfo(result, msg);
    if (result == true) {
        if (_stt_cmd_flag == vtol::MC) {
            _stt_cmd_flag = vtol::MC_TO_FIXED;
        } else if (_stt_cmd_flag == vtol::FW) {
            _stt_cmd_flag = vtol::FW_STARTING;
        } else if (_stt_cmd_flag == vtol::FW_MISSION) {
            _stt_cmd_flag = vtol::FW_STOPPING;
        } else if (_stt_cmd_flag == vtol::FW_STOPPING) {
            _stt_cmd_flag = vtol::FW_TO_QUAD;
        }
    } else {
        RCLCPP_ERROR(this->get_logger(), "Failed to send mode");
    }
}


void OffboardMavros::takeoffModeResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2>& msg) {
    const bool result = future.get()->mode_sent;
    printSuccessInfo(result, msg);
    if (result == true) {
        ;
    } else {
        RCLCPP_ERROR(this->get_logger(), "Failed to send mode");
    }
}


/* 안쓰는 함수 */
void OffboardMavros::mavrosMissionModeResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2>& msg) {
    const bool result = future.get()->mode_sent;
    printSuccessInfo(result, msg);

    if (result == true) {
        RCLCPP_INFO(this->get_logger(), "Mode sent successfully");
    } else {
        RCLCPP_ERROR(this->get_logger(), "Failed to send mode");
    }
}

/* ---------------------- Success Callback Functions ---------------------- */

void OffboardMavros::armingResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future) {
    const bool result = future.get()->success;
    const std::array<const std::string, 2> msg = {
        "Vehicle Successfully armed", 
        "Vehicle Arming failed"
    };
    printSuccessInfo(result, msg);
    if (result == true) {
        ; // _stt_cmd_flag = vtol::ARMED; <- BitHandler로 감
    } else {
        _stt_cmd_flag = isPassedSeconds(5) ? static_cast<t_bit>(vtol::READY)
                                           : _stt_cmd_flag;
    }
}

void OffboardMavros::takeoffResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future) {
    const bool result = future.get()->success;
    const std::array<const std::string, 2> msg = {
        "Takeoff command sent successfully", 
        "Failed to send Takeoff command"
    };
    printSuccessInfo(result, msg);
    if (result == true) {
        // _stt_cmd_flag = vtol::MC_TAKING_OFF;
    } else {
        _stt_cmd_flag = isPassedSeconds(5) ? static_cast<t_bit>(vtol::READY)
                                           : _stt_cmd_flag;
    }
}

void OffboardMavros::transitionResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture future) {
    const bool result = future.get()->success;
    const std::array<const std::string, 2> msg = {
        "Transition success", 
        "Transition failed"
    };
    printSuccessInfo(result, msg);
    if (result == true) {
        ;
    } else {
        if (_stt_cmd_flag == vtol::MC_TO_FIXED) {
            // _stt_cmd_flag = isPassedSeconds(5) ? static_cast<t_bit>(vtol::MC)
                                               // : _stt_cmd_flag;
        } else if (_stt_cmd_flag == vtol::FW_TO_QUAD) {
            ;
            // _stt_cmd_flag = isPassedSeconds(5) ? static_cast<t_bit>(vtol::FW)
                                               // : _stt_cmd_flag;
        }
    }
}

void OffboardMavros::landResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future) {
    const bool result = future.get()->success;
    const std::array<const std::string, 2> msg = {
        "Land command sent successfully", 
        "Failed to send land command"
    };
    printSuccessInfo(result, msg);
    if (result == true) {
        _stt_cmd_flag = vtol::STAND_BY;
    } else {
        ;
        // _stt_cmd_flag = isPassedSeconds(5) ? static_cast<t_bit>(vtol::ARMED)
        //                                    : _stt_cmd_flag;
    }
}

void OffboardMavros::disarmingResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future) {
    const bool result = future.get()->success;
    const std::array<const std::string, 2> msg = {
        "Vehicle disarmed", 
        "Disarming failed"
    };
    printSuccessInfo(result, msg);
    if (result == true) {
        if (fcu.state.second.mode == vtol::FCU_LAND) {
            _stt_cmd_flag = vtol::STAND_BY;
        }
    }
}

// shared_future를 사용하는 이유는 비동기로 요청을 보내기 때문에 요청에 대한 응답을 받아야하기 때문이다.
void OffboardMavros::locationResponseCallback(
    const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future
) {
    const bool result = future.get()->success;
    const std::array<const std::string, 2> msg = {
        "Location command sent successfully", 
        "Failed to send location command"
    };
    printSuccessInfo(result, msg);
    if (result == true) {
        ;
    } else {
        ;
        // if (isPassedSeconds();
        // if (last_request == this->now()) { ros duration
    }
}

void OffboardMavros::waypointPushResponseCallback(const rclcpp::Client<mavros_msgs::srv::WaypointPush>::SharedFuture future) {
    const bool result = future.get()->success;
    const std::array<const std::string, 2> msg = {
        "Waypoint push success", 
        "Waypoint push failed"
    };
    printSuccessInfo(result, msg);
    if (result == true) {
        ;
    } else {
        ;
    }
}

void OffboardMavros::waypointClearResponseCallback(const rclcpp::Client<mavros_msgs::srv::WaypointClear>::SharedFuture future) {
    const bool result = future.get()->success;
    const std::array<const std::string, 2> msg = {
        "Waypoint clear success", 
        "Waypoint clear failed"
    };
    printSuccessInfo(result, msg);
    if (result == true) {
        if (_stt_cmd_flag == vtol::CHECK) {
            _stt_cmd_flag = vtol::READY;
        }
    } else {
        ;
    }
}
