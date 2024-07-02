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

    global_position_[vtol::ALT]=msg->altitude;
    global_position_[vtol::LAT]=msg->latitude;
    global_position_[vtol::LON]=msg->longitude;


    DEBUG::print("gps_alt: ", global_position_[vtol::ALT], GREEN);
    DEBUG::print("gps_lat: ", global_position_[vtol::LAT], GREEN);
    DEBUG::print("gps_lon: ", global_position_[vtol::LON], GREEN);
}

/* -- Callback Functions -- */
void OffboardMavros::stateCallBack(const mavros_msgs::msg::State::SharedPtr msg) {
    fcuState_ = *msg;

    DEBUG::msg("\n[DEBUG] ", "-----------------");
    DEBUG::print("Mode : ", msg->mode, CYAN);
    DEBUG::print_bool("Arming : ", msg->armed, RED);
    DEBUG::print_binary("Command flag : ", cmdFlag_, YELLOW);
    DEBUG::print("System status : ", fcuState_.system_status, BLUE);
    DEBUG::msg("[DEBUG] ", "-----------------\n");

    // if ((statusFlag == vtol::LAND) && is_real_arming_status_() && is_five_seconds_passed()) {
    // if ((statusFlag == vol::TAKEOFF) && is_fcu_arming_status_() && is_five_seconds_passed()) {
    // }
    // TODO: status_XXX_() 함수를 만들어서 사용


    // TODO: 생성자에서 초기화
    if (OffboardMavros::cmdFlag_ == vtol::INIT) {
        if (fcuState_.armed == true) {
            update_landing_status();
        } else {
            OffboardMavros::cmdFlag_ = vtol::READY;
        }
    }
    if (OffboardMavros::cmdFlag_ == vtol::READY) {
        if (fcuState_.mode != vtol::FCU_HOLD) {
            update_disarming_status();
            update_hold_mode();
            update_custom_mode(vtol::FCU_HOLD, 
                    &OffboardMavros::hold_response_callback);
        }
    }
    if (OffboardMavros::cmdFlag_ == vtol::ARMED) {
        DEBUG::print("", ">> ARMED <<", BOLDGREEN);
        if (fcuState_.armed != true) {
            update_arming_status();
        }
    }

    if (OffboardMavros::cmdFlag_ == vtol::FLY) {
        if (fcuState_.mode != vtol::FCU_HOLD) {
            update_hold_mode();
        }
        std::cout << "Flying..." << std::endl;
    }
    if (OffboardMavros::cmdFlag_ == vtol::TAKEOFF) {
        DEBUG::print("", ">> Take Off <<", BOLDGREEN);
        // 순서 중요
        if (global_position_[0] >= 0 && global_position_[1] >= 0 && global_position_[2] >= 0) {
            if (fcuState_.mode != vtol::FCU_TAKEOFF && fcuState_.armed == true) {
                update_takeoff_status();
            } else if (fcuState_.mode == vtol::FCU_TAKEOFF && fcuState_.armed == false) {
                update_arming_status();
            }
        }

        
    }

    if (OffboardMavros::cmdFlag_ == vtol::START) {
        if (fcuState_.mode == vtol::FCU_HOLD) {
            update_offboard_mode();
        }
    }

    if (OffboardMavros::cmdFlag_ == vtol::TO_FIXED) {
        update_transition_fixed_status();
    }

    if (OffboardMavros::cmdFlag_ == vtol::TO_QUAD) {
        update_transition_quad_status();
    }

    if (OffboardMavros::cmdFlag_ == vtol::LAND) {
        if (fcuState_.mode != vtol::FCU_LAND && fcuState_.armed == true) {
            update_landing_status();
        } else if (fcuState_.mode == vtol::FCU_HOLD) {
            std::cout << "Landing success" << std::endl;
            OffboardMavros::cmdFlag_ = vtol::READY;
        }
    }
}

void    OffboardMavros::StatusReady(void) {
    if (OffboardMavros::cmdFlag_ != vtol::READY) {
        return ;
    }

    if (fcuState_.mode != vtol::FCU_HOLD) {
        OffboardMavros::cmdFlag_ = vtol::READY;
        update_custom_mode("AUTO.LOITER",
                    &OffboardMavros::hold_response_callback);
        update_disarming_status();
    }
}


/* -- Callback Functions -- */
void OffboardMavros::offboard_response_callback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
    const char* msg[] = {
        "Offboard mode sent successfully", 
        "Failed to send Offboard mode"
    };
    print_success_info(future.get()->mode_sent, msg);
}

void OffboardMavros::hold_response_callback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
    const char* msg[] = {
        "Hold mode sent successfully", 
        "Failed to send Hold mode"
    };
    print_success_info(future.get()->mode_sent, msg);
}

void OffboardMavros::chatterCallback(const std_msgs::msg::String::SharedPtr msg) {
    RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());


    size_t i = 0;
    for (; i < OffboardMavros::action_string_array_.size() && msg->data != OffboardMavros::action_string_array_[i]; ++i);

    if (i == OffboardMavros::action_string_array_.size()) {
        std::cout << "Invalid input" << std::endl;
        return ;
    }
    OffboardMavros::action_func_[i]();
    OffboardMavros::print_reference_input();
}

void OffboardMavros::arming_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future) {
    const char* msg[] = {
        "Vehicle armed", 
        "Arming failed"
    };
    print_success_info(future.get()->success, msg);
}

void OffboardMavros::disarming_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future) {
    const char* msg[] = {
        "Vehicle disarmed", 
        "Disarming failed"
    };
    print_success_info(future.get()->success, msg);
}

void OffboardMavros::transition_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture future) {
    const char* msg[] = {
        "Transition success", 
        "Transition failed"
    };
    print_success_info(future.get()->success, msg);
}


void OffboardMavros::takeoff_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future) {
    const bool success = future.get()->success;
    const char* msg[] = {
        "Takeoff command sent successfully", 
        "Failed to send Takeoff command"
    };
    print_success_info(success, msg);
}

void OffboardMavros::land_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future) {
    const bool success = future.get()->success;
    const char* msg[] = {
        "Land command sent successfully", 
        "Failed to send land command"
    };
    print_success_info(success, msg);
}
// shared_future를 사용하는 이유는 비동기로 요청을 보내기 때문에 요청에 대한 응답을 받아야하기 때문이다.



//return request

void OffboardMavros::location_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future) {
    const char* msg[] = {
        "Location command sent successfully", 
        "Failed to send location command"
    };
    print_success_info(future.get()->success, msg);
}

void OffboardMavros::currentpositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
    cur_position_ = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};

    if (cmdFlag_ == vtol::TAKEOFF) {
        if (global_position_[vtol::ALT] > init_global_position[vtol::ALT] - 1) {
            cmdFlag_ = vtol::FLY;
            prev_position_[vtol::NORTH] = cur_position_[vtol::NORTH];
            prev_position_[vtol::EAST] = cur_position_[vtol::EAST];
            DEBUG::print("Landing point North :", prev_position_[vtol::NORTH], BOLDYELLOW);
            DEBUG::print("Landing point East  :", prev_position_[vtol::EAST], BOLDYELLOW);
        }
    } else if (cmdFlag_ == vtol::TO_FIXED) {
        DEBUG::print("North :", cur_position_[vtol::NORTH], WHITE);
        DEBUG::print("East  :", cur_position_[vtol::EAST], WHITE);
        if (cur_position_[vtol::NORTH] > prev_position_[vtol::NORTH] + 1 
            || cur_position_[vtol::EAST] > prev_position_[vtol::EAST] + 1) {
            DEBUG::print("Transition success North :", cur_position_[vtol::NORTH], BOLDYELLOW);
            DEBUG::print("Transition success East  :", cur_position_[vtol::EAST], BOLDYELLOW);
            cmdFlag_ = vtol::FIXED;
        }
    } else if (cmdFlag_ == vtol::TO_QUAD) {
        if (cur_position_[vtol::NORTH] - prev_position_[vtol::NORTH] < 0.1
            && cur_position_[vtol::EAST] - prev_position_[vtol::EAST] < 0.1) {
            DEBUG::print("Transition success North :", cur_position_[vtol::NORTH], BOLDYELLOW);
            DEBUG::print("Transition success East  :", cur_position_[vtol::EAST], BOLDYELLOW);
            // cmdFlag_ &= ~vtol::BIT_TRANSITION;
            cmdFlag_ = vtol::QUAD;
        }
        prev_position_[vtol::NORTH] = cur_position_[vtol::NORTH];
        prev_position_[vtol::EAST] = cur_position_[vtol::EAST];
    }
}