#include "px4_ros_com/OffboardMavros.hpp"

void OffboardMavros::poseCallBack(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {

    const double q1 = msg->pose.orientation.x;
    const double q2 = msg->pose.orientation.y;
    const double q3 = msg->pose.orientation.z;
    const double q4 = msg->pose.orientation.w;

    const double t1 = 2 *(q4*q3+q1*q2);
    const double t2 = 1 - 2 *(q2*q2+q3*q3);

    yaw_current = atan2(t1,t2) * vtol::RAD_2_DEG;
    // DEBUG::print("[Pose] Yaw current: ", yaw_current,GREEN);
}

void OffboardMavros::gpsCallBack(const sensor_msgs::msg::NavSatFix::SharedPtr msg) {

    if (_global_position[0] == -1.0f || _global_position[1] == -1.0f || _global_position[2] == -1.0f) {
        _init_global_position[vtol::ALT] = msg->altitude + 5.0f;
        _init_global_position[vtol::LAT] = msg->latitude;
        _init_global_position[vtol::LON] = msg->longitude;
        DEBUG::print("alt: ", _init_global_position[vtol::ALT],BLUE);
        DEBUG::print("lat: ", _init_global_position[vtol::LAT],BLUE);
        DEBUG::print("lon: ", _init_global_position[vtol::LON],BLUE);
    }
    _global_position[vtol::ALT] = msg->altitude;
    _global_position[vtol::LAT] = msg->latitude;
    _global_position[vtol::LON] = msg->longitude;

    DEBUG::print("gps_alt: ", _global_position[vtol::ALT], GREEN);
    DEBUG::print("gps_lat: ", _global_position[vtol::LAT], GREEN);
    DEBUG::print("gps_lon: ", _global_position[vtol::LON], GREEN);
}


/* -- Callback Functions -- */
void OffboardMavros::stateCallBack(const mavros_msgs::msg::State::SharedPtr msg) {
    if (_global_position[0] == -1.0f || _global_position[1] == -1.0f || _global_position[2] == -1.0f) {
        return ;
    }

    fcu_state = *msg;

    DEBUG::msg("\n[DEBUG] ", "-----------------");
    DEBUG::print("Mode : ", msg->mode, CYAN);
    DEBUG::printBool("Arming : ", msg->armed, RED);
    DEBUG::printBinary("Command flag : ", _cmd_flag, YELLOW);
    DEBUG::print("System status : ", fcu_state.system_status, BLUE);
    DEBUG::print("Yaw current: ", yaw_current, GREEN);
    DEBUG::printArray("local_velocity_: ", _local_velocity, 3, MAGENTA);
    DEBUG::msg("[DEBUG] ", "-----------------\n");

    // if ((statusFlag == vtol::LAND) && is_real_arming_status_() && isFiveSecondsPassed()) {
    // if ((statusFlag == vol::TAKEOFF) && is_fcu_arming_status_() && isFiveSecondsPassed()) {
    // }
    // TODO: status_XXX_() 함수를 만들어서 사용

//ros::Time::now() - last_request > ros::Duration(5.0)
    // TODO: 생성자에서 초기화
    if (OffboardMavros::_cmd_flag == vtol::INIT) {
        if (fcu_state.armed == true) {
            updateLandingStatus();
        } else {
            OffboardMavros::_cmd_flag = vtol::READY;
        }
    }
    if (OffboardMavros::_cmd_flag == vtol::READY) {
        if (fcu_state.mode != vtol::FCU_HOLD) {
            updateDisarmingStatus();
            updateHoldMode();
            updateCustomMode(vtol::FCU_HOLD, 
                    &OffboardMavros::holdResponseCallback);
        }
    }
    if (OffboardMavros::_cmd_flag == vtol::ARMED) {
        DEBUG::print("", ">> ARMED <<", BOLDGREEN);
        if (fcu_state.armed != true) {
            updateArmingStatus();
        }
    }

    if (OffboardMavros::_cmd_flag == vtol::FLY) {
        if (fcu_state.mode != vtol::FCU_HOLD) {
            updateHoldMode();
        }
        std::cout << "Flying..." << std::endl;
    }
    if (OffboardMavros::_cmd_flag == vtol::TAKEOFF) {
        DEBUG::print("", ">> Take Off <<", BOLDGREEN);
        // 순서 중요
        if (fcu_state.mode != vtol::FCU_TAKEOFF && fcu_state.armed == true) {
            updateTakeoffStatus();
        } else if (fcu_state.mode == vtol::FCU_TAKEOFF && fcu_state.armed == false) {
            updateArmingStatus();
        }
    }

    if (OffboardMavros::_cmd_flag == vtol::START) {
        if (fcu_state.mode == vtol::FCU_HOLD) {
            updateOffboardMode();
        }
    }

    if (OffboardMavros::_cmd_flag == vtol::TO_FIXED) {
        updateTransitionFixedStatus();
    }

    if (OffboardMavros::_cmd_flag == vtol::TO_QUAD) {
        updateTransitionQuadStatus();
    }

    if (OffboardMavros::_cmd_flag == vtol::LAND) {
        if (fcu_state.mode != vtol::FCU_LAND && fcu_state.armed == true) {
            updateLandingStatus();
        } else if (fcu_state.mode == vtol::FCU_HOLD) {
            std::cout << "Landing success" << std::endl;
            OffboardMavros::_cmd_flag = vtol::READY;
        }
    }
}

void    OffboardMavros::statusReady(void) {
    if (OffboardMavros::_cmd_flag != vtol::READY) {
        return ;
    }

    if (fcu_state.mode != vtol::FCU_HOLD) {
        OffboardMavros::_cmd_flag = vtol::READY;
        updateCustomMode("AUTO.LOITER",
                    &OffboardMavros::holdResponseCallback);
        updateDisarmingStatus();
    }
}


/* -- Callback Functions -- */
void OffboardMavros::offboardResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
    const char* msg[] = {
        "Offboard mode sent successfully", 
        "Failed to send Offboard mode"
    };
    printSuccessInfo(future.get()->mode_sent, msg);
}

void OffboardMavros::holdResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
    const char* msg[] = {
        "Hold mode sent successfully", 
        "Failed to send Hold mode"
    };
    printSuccessInfo(future.get()->mode_sent, msg);
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

void OffboardMavros::armingResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future) {
    const char* msg[] = {
        "Vehicle armed", 
        "Arming failed"
    };
    printSuccessInfo(future.get()->success, msg);
}

void OffboardMavros::disarmingResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future) {
    const char* msg[] = {
        "Vehicle disarmed", 
        "Disarming failed"
    };
    printSuccessInfo(future.get()->success, msg);
}

void OffboardMavros::transitionResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture future) {
    const char* msg[] = {
        "Transition success", 
        "Transition failed"
    };
    printSuccessInfo(future.get()->success, msg);
}


void OffboardMavros::takeoffResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future) {
    const bool success = future.get()->success;
    const char* msg[] = {
        "Takeoff command sent successfully", 
        "Failed to send Takeoff command"
    };
    printSuccessInfo(success, msg);
}

void OffboardMavros::landResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future) {
    const bool success = future.get()->success;
    const char* msg[] = {
        "Land command sent successfully", 
        "Failed to send land command"
    };
    printSuccessInfo(success, msg);
}
// shared_future를 사용하는 이유는 비동기로 요청을 보내기 때문에 요청에 대한 응답을 받아야하기 때문이다.



//return request

void OffboardMavros::locationResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future) {
    const char* msg[] = {
        "Location command sent successfully", 
        "Failed to send location command"
    };
    printSuccessInfo(future.get()->success, msg);
}

void OffboardMavros::currentPositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
    _cur_position = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};


    if (_cmd_flag == vtol::TAKEOFF) {
        if (_global_position[vtol::ALT] > _init_global_position[vtol::ALT] - 1) {
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
    }
}
