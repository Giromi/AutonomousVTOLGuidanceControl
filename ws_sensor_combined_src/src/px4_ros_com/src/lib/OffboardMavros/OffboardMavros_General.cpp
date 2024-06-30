#include "px4_ros_com/OffboardMavros.hpp"


unsigned char                           OffboardMavros::cmdFlag_ = vtol::INIT;
std::array<double, 3>		            OffboardMavros::local_position_{vtol::INIT_NORTH, vtol::INIT_EAST, vtol::INIT_UP};
std::array<double, 6>		            OffboardMavros::local_velocity_{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};  
std::array<double, 3>		            OffboardMavros::cur_position_{};
std::array<double, 3>		            OffboardMavros::prev_position_{};
double                                  OffboardMavros::offset_ = 0.5;

/**
 * @brief OffboardMavros 생성자
 */
OffboardMavros::OffboardMavros(void) : Node("offboard_mavros") {
    initializePublishers();
    initializeSubscribers();
    initializeClients();
    initializeTimers(50); 
}


//  /* -- Initialize Functions -- */
// void OffboardMavros::initializePublishers(void) {
//     local_pos_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("/mavros/setpoint_position/local", 10);

//     local_vel_pub = this->create_publisher<geometry_msgs::msg::TwistStamped>("/mavros/setpoint_velocity/cmd_vel", 10);
//     local_pub = this->create_publisher<mavros_msgs::msg::PositionTarget>("mavros/setpoint_raw/local", 10);
//     att_pub = this->create_publisher<geometry_msgs::msg::TwistStamped>("mavros/setpoint_attitude/cmd_vel", 10);
//     actuator_control_pub_ = this->create_publisher<mavros_msgs::msg::ActuatorControl>( "/mavros/actuator_control", 10);
// }

// void OffboardMavros::initializeSubscribers(void) {
//     state_sub_ = create_subscription<mavros_msgs::msg::State>(
//             "mavros/state", 10, std::bind(&OffboardMavros::stateCallBack, this, std::placeholders::_1));

//     subscription_ = this->create_subscription<std_msgs::msg::String>("/chatter", 10,
//             std::bind( &OffboardMavros::chatterCallback, this, std::placeholders::_1
//                 ));
//     auto default_qos = rclcpp::QoS(rclcpp::SystemDefaultsQoS());
//     current_pos_sub_ = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/local_position/pose", default_qos,
//             std::bind(&OffboardMavros::currentpositionCallback, this, std::placeholders::_1
//                 ));
// }

// void    OffboardMavros::initializeClients(void) {
//     set_mode_client_ = create_client<mavros_msgs::srv::SetMode>("/mavros/set_mode");
//     arming_client_ = create_client<mavros_msgs::srv::CommandBool>("/mavros/cmd/arming");
//     takeoff_client_ = create_client<mavros_msgs::srv::CommandTOL>("/mavros/cmd/takeoff");
//     landing_client_ = create_client<mavros_msgs::srv::CommandTOL>("/mavros/cmd/land");
//     location_client_ = this->create_client<mavros_msgs::srv::CommandLong>("/mavros/cmd/command");
//     transition_client_ = this->create_client<mavros_msgs::srv::CommandVtolTransition>("/mavros/cmd/vtol_transition");
// }

// void OffboardMavros::initializeTimers(const int rate_hz) {
//     const int rate_ms = 1000 / rate_hz;
//     timer_ = this->create_wall_timer(
//             std::chrono::milliseconds(rate_ms),
//             std::bind(&OffboardMavros::publish, this));
// }


// /* -- Callback Functions -- */
// void OffboardMavros::stateCallBack(const mavros_msgs::msg::State::SharedPtr msg) {
//     fcuState_ = *msg;

//     DEBUG::msg("\n[DEBUG] ", "-----------------");
//     DEBUG::print("Mode : ", msg->mode, CYAN);
//     DEBUG::print_bool("Arming : ", msg->armed, RED);
//     DEBUG::print_binary("Command flag : ", cmdFlag_, YELLOW);
//     DEBUG::print("System status : ", fcuState_.system_status, BLUE);
//     DEBUG::msg("[DEBUG] ", "-----------------\n");

//     // if ((statusFlag == vtol::LAND) && is_real_arming_status_() && is_five_seconds_passed()) {
//     // if ((statusFlag == vol::TAKEOFF) && is_fcu_arming_status_() && is_five_seconds_passed()) {
//     // }
//     // TODO: status_XXX_() 함수를 만들어서 사용


//     // TODO: 생성자에서 초기화
//     if (OffboardMavros::cmdFlag_ == vtol::INIT) {
//         if (fcuState_.armed == true) {
//             update_landing_status();
//         } else {
//             OffboardMavros::cmdFlag_ = vtol::READY;
//         }
//     }
//     if (OffboardMavros::cmdFlag_ == vtol::READY) {
//         if (fcuState_.mode != vtol::FCU_HOLD) {
//             update_disarming_status();
//             update_hold_mode();
//             update_custom_mode(vtol::FCU_HOLD, 
//                     &OffboardMavros::hold_response_callback);
//         }
//     }
//     if (OffboardMavros::cmdFlag_ == vtol::ARMED) {
//         DEBUG::print("", ">> ARMED <<", BOLDGREEN);
//         if (fcuState_.armed != true) {
//             update_arming_status();
//         }
//     }

//     if (OffboardMavros::cmdFlag_ == vtol::FLY) {
//         if (fcuState_.mode != vtol::FCU_HOLD) {
//             update_hold_mode();
//         }
//         std::cout << "Flying..." << std::endl;
//     }
//     if (OffboardMavros::cmdFlag_ == vtol::TAKEOFF) {
//         DEBUG::print("", ">> Take Off <<", BOLDGREEN);
//         // 순서 중요
//         if (fcuState_.mode != vtol::FCU_TAKEOFF && fcuState_.armed == true) {
//             update_takeoff_status();
//         } else if (fcuState_.mode == vtol::FCU_TAKEOFF && fcuState_.armed == false) {
//             update_arming_status();
//         }
//     }

//     if (OffboardMavros::cmdFlag_ == vtol::START) {
//         if (fcuState_.mode == vtol::FCU_HOLD) {
//             update_offboard_mode();
//         }
//     }

//     if (OffboardMavros::cmdFlag_ == vtol::TO_FIXED) {
//         update_transition_fixed_status();
//     }

//     if (OffboardMavros::cmdFlag_ == vtol::TO_QUAD) {
//         update_transition_quad_status();
//     }

//     if (OffboardMavros::cmdFlag_ == vtol::LAND) {
//         if (fcuState_.mode != vtol::FCU_LAND && fcuState_.armed == true) {
//             update_landing_status();
//         } else if (fcuState_.mode == vtol::FCU_HOLD) {
//             std::cout << "Landing success" << std::endl;
//             OffboardMavros::cmdFlag_ = vtol::READY;
//         }
//     }
// }

// void    OffboardMavros::StatusReady(void) {
//     if (OffboardMavros::cmdFlag_ != vtol::READY) {
//         return ;
//     }

//     if (fcuState_.mode != vtol::FCU_HOLD) {
//         OffboardMavros::cmdFlag_ = vtol::READY;
//         update_custom_mode("AUTO.LOITER",
//                     &OffboardMavros::hold_response_callback);
//         update_disarming_status();
//     }
// }

// /* -- Publish Functions -- */
// void    OffboardMavros::publish(void) {
//     if (cmdFlag_ != vtol::START) {
//         return ;
//     }
//     std::cout << "Publishing..." << std::endl;
//     // publishPose();
//     // publish_velocity_();
//     publish_local();
//     // publish_attitude_(); // orbit 안사라짐
// }

// void OffboardMavros::publishPose(void) {
//     geometry_msgs::msg::PoseStamped pose;
//     pose.pose.position.x = local_position_[vtol::EAST];
//     pose.pose.position.y = local_position_[vtol::NORTH];
//     pose.pose.position.z = local_position_[vtol::UP];
//     local_pos_pub_->publish(pose);
// }

// void OffboardMavros::publishActuatorControls(void) {
//     mavros_msgs::msg::ActuatorControl actuator_control_msg;
//     actuator_control_msg.group_mix = 2;
//     actuator_control_msg.header.stamp = this->now();
//     actuator_control_msg.header.frame_id = "standard_vtol_0";
//     actuator_control_msg.controls[0] = 1.0f;
//     actuator_control_msg.controls[1] = 1.0f;
//     actuator_control_msg.controls[2] = 1.0f;
//     actuator_control_msg.controls[7] = 1.0f;
//     RCLCPP_INFO(this->get_logger(), "publishing actuator controls");
//     actuator_control_pub_->publish(actuator_control_msg);
// }

// void OffboardMavros::publish_velocity(void) {
//     geometry_msgs::msg::TwistStamped vel;
//     vel.twist.linear.x = local_velocity_[0];
//     vel.twist.linear.y = local_velocity_[1];
//     vel.twist.linear.z = local_velocity_[2];
//     vel.twist.angular.x = local_velocity_[3];
//     vel.twist.angular.y = local_velocity_[4];
//     vel.twist.angular.z = local_velocity_[5];
//     local_vel_pub->publish(vel);
// }

// void OffboardMavros::publish_attitude(void) {
//     geometry_msgs::msg::TwistStamped att;
//     att.twist.linear.x = local_velocity_[0];
//     att.twist.linear.y = local_velocity_[1];
//     att.twist.linear.z = local_velocity_[2];
//     att.twist.angular.x = local_velocity_[3];
//     att.twist.angular.y = local_velocity_[4];
//     att.twist.angular.z = local_velocity_[5];
//     att_pub->publish(att);
// }

// void OffboardMavros::publish_local(void) {
//     mavros_msgs::msg::PositionTarget local_msg;

//     local_msg.header.stamp = this->now();
//     local_msg.header.frame_id = "standard_vtol_0";
//     local_msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_LOCAL_NED;
//     local_msg.type_mask = //mavros_msgs::msg::PositionTarget::IGNORE_PX |
//                           //mavros_msgs::msg::PositionTarget::IGNORE_PY |
//                           //mavros_msgs::msg::PositionTarget::IGNORE_PZ |
//         mavros_msgs::msg::PositionTarget::IGNORE_AFX |
//         mavros_msgs::msg::PositionTarget::IGNORE_AFY |
//         mavros_msgs::msg::PositionTarget::IGNORE_AFZ;
//     //mavros_msgs::msg::PositionTarget::IGNORE_VZ;
//     //mavros_msgs::msg::PositionTarget::IGNORE_YAW_RATE;
//     local_msg.velocity.x = local_velocity_[0];
//     local_msg.velocity.y = local_velocity_[1];
//     local_msg.velocity.z = local_velocity_[2];
//     local_msg.yaw = local_velocity_[4];
//     local_msg.yaw_rate = local_velocity_[5];
//     local_pub->publish(local_msg);
// }

// // /* -- Update Functions -- */

// // void OffboardMavros::update_arming_status(void) {
// //     request_arming_status(true, &OffboardMavros::arming_response_callback);
// // }

// // void OffboardMavros::update_disarming_status(void) {
// //     request_arming_status(false, &OffboardMavros::disarming_response_callback);
// // }

// // void OffboardMavros::update_transition_fixed_status(void) {
// //     // if (fcuState_.mode == vtol::MC) {
// //     request_transition_status(vtol::FW, &OffboardMavros::transition_response_callback);
// //     // } else {
// //     // request_transition_status_(vtol::MC, &OffboardMavros::transition_response_callback);
// // }

// // void OffboardMavros::update_transition_quad_status(void) {
// //     // if (fcuState_.mode == vtol::MC) {
// //     request_transition_status(vtol::MC, &OffboardMavros::transition_response_callback);
// //     // } else {
// //     // request_transition_status_(vtol::MC, &OffboardMavros::transition_response_callback);
// // }

// // void OffboardMavros::request_transition_status(const int input,
// //         void (OffboardMavros::*response_callback)
// //         (const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture)) {
// //     auto request = std::make_shared<mavros_msgs::srv::CommandVtolTransition::Request>();
// //     request->state = input;
// //     transition_client_->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
// //     last_request_ = this->now();
// // }


// // void OffboardMavros::request_arming_status(const bool& input,
// //         void (OffboardMavros::*response_callback)
// //         (const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture)) {
// //     auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
// //     request->value = input;
// //     arming_client_->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
// //     last_request_ = this->now();
// // }

// // void OffboardMavros::update_takeoff_status(void) {
// //     auto request = make_request_takeoff_land_message(
// //             vtol::GeographicCoordinate{vtol::INIT_UP, 0, 0, 0, 0});
// //     takeoff_client_->async_send_request(request,
// //             std::bind(&OffboardMavros::takeoff_response_callback, this, std::placeholders::_1));
// //     last_request_ = this->now();
// // }

// // void OffboardMavros::update_landing_status(void) {
// //     auto request = make_request_takeoff_land_message(
// //             vtol::GeographicCoordinate{0, 0, 0, 0, 0});
// //     landing_client_->async_send_request(request,
// //             std::bind(&OffboardMavros::land_response_callback, this, std::placeholders::_1));
// //     last_request_ = this->now();
// // }

// // std::shared_ptr<mavros_msgs::srv::CommandTOL::Request> OffboardMavros::make_request_takeoff_land_message(const vtol::GeographicCoordinate& input) {
// //     auto request = std::make_shared<mavros_msgs::srv::CommandTOL::Request>();
// //     request->altitude   = input.altitude;
// //     request->latitude   = input.latitude;
// //     request->longitude  = input.longitude;
// //     request->min_pitch  = input.min_pitch;
// //     request->yaw        = input.yaw;
// //     return request;
// // }

// // void OffboardMavros::update_location_(std::array<double, 3> input) {
// //     auto request = std::make_shared<mavros_msgs::srv::CommandLong::Request>();
// //     request->command = 16;
// //     request->param5 = input[vtol::NORTH]; // latitude
// //     request->param6 = input[vtol::EAST]; // longitude
// //     request->param7 = input[vtol::UP]; // altitude
// //     location_client_->async_send_request(request, 
// //             std::bind(&OffboardMavros::location_response_callback, this, std::placeholders::_1));
// //     last_request_ = this->now();
// // }

// void OffboardMavros::update_hold_mode(void) {
//     update_custom_mode("AUTO.LOITER", &OffboardMavros::hold_response_callback);
// }

// void OffboardMavros::update_offboard_mode(void) {
//     update_custom_mode("OFFBOARD", &OffboardMavros::offboard_response_callback);
// }

// void OffboardMavros::update_custom_mode(const std::string& input_mode,
//         void (OffboardMavros::*response_callback)(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture)) {
//     auto request = std::make_shared<mavros_msgs::srv::SetMode::Request>();
//     request->custom_mode = input_mode;
//     set_mode_client_->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
//     last_request_ = this->now();
// }



// /* -- Callback Functions -- */
// void OffboardMavros::offboard_response_callback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
//     const char* msg[] = {
//         "Offboard mode sent successfully", 
//         "Failed to send Offboard mode"
//     };
//     print_success_info(future.get()->mode_sent, msg);
// }

// void OffboardMavros::hold_response_callback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
//     const char* msg[] = {
//         "Hold mode sent successfully", 
//         "Failed to send Hold mode"
//     };
//     print_success_info(future.get()->mode_sent, msg);
// }

// void OffboardMavros::chatterCallback(const std_msgs::msg::String::SharedPtr msg) {
//     RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());


//     size_t i = 0;
//     for (; i < OffboardMavros::action_string_array_.size() && msg->data != OffboardMavros::action_string_array_[i]; ++i);

//     if (i == OffboardMavros::action_string_array_.size()) {
//         std::cout << "Invalid input" << std::endl;
//         return ;
//     }
//     OffboardMavros::action_func_[i]();
//     OffboardMavros::print_reference_input();
// }

// void OffboardMavros::arming_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future) {
//     const char* msg[] = {
//         "Vehicle armed", 
//         "Arming failed"
//     };
//     print_success_info(future.get()->success, msg);
// }

// void OffboardMavros::disarming_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future) {
//     const char* msg[] = {
//         "Vehicle disarmed", 
//         "Disarming failed"
//     };
//     print_success_info(future.get()->success, msg);
// }

// void OffboardMavros::transition_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture future) {
//     const char* msg[] = {
//         "Transition success", 
//         "Transition failed"
//     };
//     print_success_info(future.get()->success, msg);
// }


// void OffboardMavros::takeoff_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future) {
//     const bool success = future.get()->success;
//     const char* msg[] = {
//         "Takeoff command sent successfully", 
//         "Failed to send Takeoff command"
//     };
//     print_success_info(success, msg);
// }

// void OffboardMavros::land_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future) {
//     const bool success = future.get()->success;
//     const char* msg[] = {
//         "Land command sent successfully", 
//         "Failed to send land command"
//     };
//     print_success_info(success, msg);
// }
// // shared_future를 사용하는 이유는 비동기로 요청을 보내기 때문에 요청에 대한 응답을 받아야하기 때문이다.



// //return request

// void OffboardMavros::location_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future) {
//     const char* msg[] = {
//         "Location command sent successfully", 
//         "Failed to send location command"
//     };
//     print_success_info(future.get()->success, msg);
// }

// void OffboardMavros::currentpositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
//     cur_position_ = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};

//     if (cmdFlag_ == vtol::TAKEOFF) {
//         if (cur_position_[vtol::UP] > vtol::INIT_UP - 1) {
//             cmdFlag_ = vtol::FLY;
//             prev_position_[vtol::NORTH] = cur_position_[vtol::NORTH];
//             prev_position_[vtol::EAST] = cur_position_[vtol::EAST];
//             DEBUG::print("Landing point North :", prev_position_[vtol::NORTH], BOLDYELLOW);
//             DEBUG::print("Landing point East  :", prev_position_[vtol::EAST], BOLDYELLOW);
//         }
//     } else if (cmdFlag_ == vtol::TO_FIXED) {
//         DEBUG::print("North :", cur_position_[vtol::NORTH], WHITE);
//         DEBUG::print("East  :", cur_position_[vtol::EAST], WHITE);
//         if (cur_position_[vtol::NORTH] > prev_position_[vtol::NORTH] + 1 
//             || cur_position_[vtol::EAST] > prev_position_[vtol::EAST] + 1) {
//             DEBUG::print("Transition success North :", cur_position_[vtol::NORTH], BOLDYELLOW);
//             DEBUG::print("Transition success East  :", cur_position_[vtol::EAST], BOLDYELLOW);
//             cmdFlag_ = vtol::FIXED;
//         }
//     } else if (cmdFlag_ == vtol::TO_QUAD) {
//         if (cur_position_[vtol::NORTH] - prev_position_[vtol::NORTH] < 0.1
//             && cur_position_[vtol::EAST] - prev_position_[vtol::EAST] < 0.1) {
//             DEBUG::print("Transition success North :", cur_position_[vtol::NORTH], BOLDYELLOW);
//             DEBUG::print("Transition success East  :", cur_position_[vtol::EAST], BOLDYELLOW);
//             // cmdFlag_ &= ~vtol::BIT_TRANSITION;
//             cmdFlag_ = vtol::QUAD;
//         }
//         prev_position_[vtol::NORTH] = cur_position_[vtol::NORTH];
//         prev_position_[vtol::EAST] = cur_position_[vtol::EAST];
//     }
// }

// /* -- Utile Functions -- */

// bool OffboardMavros::is_five_seconds_passed() {
//     return (this->now() - last_request_).seconds() > 5.0;
// }

// void OffboardMavros::print_success_info(bool success, const char* msg[]) const {
//     if (success) {
//         RCLCPP_INFO(this->get_logger(), "%s", msg[vtol::SUCCESS]);
//     } else {
//         RCLCPP_INFO(this->get_logger(), "%s", msg[vtol::FAIL]);
//     }
// }

// void OffboardMavros::print_reference_input(void) {
//     std::cout << "Position Command {"
//         << local_position_[vtol::NORTH] << ", "
//         << local_position_[vtol::EAST] << ", "
//         << local_position_[vtol::UP] << "} (North, East, Up)"
//         << std::endl;
// }

    /* -- Is Functions -- */



   

    // bool is_fcu_offboard_mode_() {
    //     return (current_state_.mode == vtol::FCU_OFFBOARD);
    // }
    //
    // bool is_fcu_hold_mode_() {
    //     return (current_state_.mode == vtol::FCU_HOLD);
    // }
    //
    // bool is_fcu_takeoff_mode_() {
    //     return (current_state_.mode == vtol::FCU_TAKEOFF);
    // }
    //
    // bool is_fcu_takeoff_mode_() {
    //     return (current_state_.mode == vtol::FCU_TAKEOFF);
    // }
    //
    // bool is_fcu_disarming_status_() {
    //     return (!current_state_.armed);
    // }
    //
    // bool is_fcu_arming_status_() {
    //     return (current_state_.armed);
    // }

    /* -- Static Functions -- */

void OffboardMavros::action_go_north(void) {
    //TODO make threshold
    local_position_[vtol::NORTH] += offset_;
}

void OffboardMavros::action_go_east(void) {
    //TODO make threshold
    local_position_[vtol::EAST] += offset_;
}

void OffboardMavros::action_go_down(void) {
    //TODO make threshold
    local_position_[vtol::UP] -= offset_;
}

void OffboardMavros::action_go_south(void) {
    //TODO make threshold
    local_position_[vtol::NORTH] -= offset_;
}

void OffboardMavros::action_go_west(void) {
    //TODO make threshold
    local_position_[vtol::EAST] -= offset_;
}

void OffboardMavros::action_go_up(void) {
    //TODO make threshold
    local_position_[vtol::UP] += offset_;
}

void OffboardMavros::action_velocity_plus_x(void) {
    local_velocity_[0] += offset_;
}

void OffboardMavros::action_velocity_plus_y(void) {
    local_velocity_[1] += offset_;
}

void OffboardMavros::action_velocity_plus_z(void) {
    local_velocity_[2] += offset_;
}

void OffboardMavros::action_velocity_minus_x(void) {
    local_velocity_[0] -= offset_;
}

void OffboardMavros::action_velocity_minus_y(void) {
    local_velocity_[1] -= offset_;
}

void OffboardMavros::action_velocity_minus_z(void) {
    local_velocity_[2] -= offset_;
}

void OffboardMavros::action_velocity_plus_roll(void) {
    local_velocity_[3] += offset_;
}

void OffboardMavros::action_velocity_plus_pitch(void) {
    local_velocity_[4] += offset_;
}

void OffboardMavros::action_velocity_plus_yaw(void) {
    local_velocity_[5] += offset_;
}

void OffboardMavros::action_velocity_minus_roll(void) {
    local_velocity_[3] -= offset_;
}

void OffboardMavros::action_velocity_minus_pitch(void) {
    local_velocity_[4] -= offset_;
}

void OffboardMavros::action_velocity_minus_yaw(void) {
    local_velocity_[5] -= offset_;
}

void OffboardMavros::action_return_home(void) {
    //TODO make threshold
    // local_position_[vtol::NORTH] = 0.0;
    // local_position_[vtol::EAST] = 0.0;
    local_velocity_[0] = 0.0;
    local_velocity_[1] = 0.0;
    local_velocity_[2] = 0.0;
    local_velocity_[3] = 0.0;
    local_velocity_[4] = 0.0;
    local_velocity_[5] = 0.0;
}

void OffboardMavros::action_arming(void) {
    if (OffboardMavros::cmdFlag_ != vtol::READY) {
        std::cout << "Vehicle is NOT READY status" << std::endl;
        return ;
    }
    OffboardMavros::cmdFlag_ = vtol::ARMED;
}

void OffboardMavros::action_disarming(void) {
    if (OffboardMavros::cmdFlag_ != vtol::ARMED) {
        std::cout << "Vehicle is NOT ARMED status" << std::endl;
        return ;
    }
    OffboardMavros::cmdFlag_ = vtol::READY;
}

void OffboardMavros::action_takeoff(void) {

    // if (!(statusFlag & vtol::BIT_FLY)) {
    //     RCLCPP_INFO(this->get_logger(), "Vehicle is NOT ARMED status");
    //     return true;
    if (OffboardMavros::cmdFlag_ == vtol::READY) {
        std::cout << "Vehicle is NOT ARMED status" << std::endl;
        return ;
    } else if (OffboardMavros::cmdFlag_ == vtol::ARMED) {
        std::cout << "Calling takeoff service ..." << std::endl;
    }
    OffboardMavros::cmdFlag_ = vtol::TAKEOFF;
}

void OffboardMavros::action_landing(void) {
    OffboardMavros::cmdFlag_ = vtol::LAND;
}

void OffboardMavros::action_start(void) {
    if (OffboardMavros::cmdFlag_ == vtol::QUAD || OffboardMavros::cmdFlag_ == vtol::FIXED) {
        OffboardMavros::cmdFlag_ = vtol::START;
    } else if (OffboardMavros::cmdFlag_ == vtol::START) {
        OffboardMavros::cmdFlag_ = vtol::FLY;
    }
}

void OffboardMavros::action_hold(void) {
    OffboardMavros::cmdFlag_ = vtol::FLY;
}

void OffboardMavros::action_init(void) {
    OffboardMavros::cmdFlag_ = vtol::INIT;
}

void OffboardMavros::action_transition(void) {
    if (OffboardMavros::cmdFlag_ == vtol::QUAD) {
        OffboardMavros::cmdFlag_ = vtol::TO_FIXED;
    } else if (OffboardMavros::cmdFlag_ == vtol::FIXED) {
        OffboardMavros::cmdFlag_ = vtol::TO_QUAD;
    }
}


const std::array<std::string, vtol::ACTION_SIZE>	OffboardMavros::action_string_array_ = { 
    "2", "4", "6", "3", "5", "7", 
    "↑", "↓", "→", "←", "+", "-", 
    "h", "a", "d", "t", "l", "s", "0", "w",  
};

void (*OffboardMavros::action_func_[])(void) = {
    &OffboardMavros::action_velocity_plus_roll, // 2
    &OffboardMavros::action_velocity_plus_pitch,// 4
    &OffboardMavros::action_velocity_plus_yaw,  // 6
    &OffboardMavros::action_velocity_minus_roll,  // 3
    &OffboardMavros::action_velocity_minus_pitch,  // 5
    &OffboardMavros::action_velocity_minus_yaw,    // 7
    &OffboardMavros::action_velocity_plus_z,           // ↑ up
    &OffboardMavros::action_velocity_minus_z,          // ↓ down
    &OffboardMavros::action_velocity_minus_y,      // ← south
    &OffboardMavros::action_velocity_plus_y,       // → north
    &OffboardMavros::action_velocity_plus_x,       // + east
    &OffboardMavros::action_velocity_minus_x,      // - west
    &OffboardMavros::action_return_home,       // h
    &OffboardMavros::action_arming,            // a
    &OffboardMavros::action_disarming,         // d
    &OffboardMavros::action_takeoff,           // t
    &OffboardMavros::action_landing,           // l
    &OffboardMavros::action_start,             // s
    &OffboardMavros::action_init,              // 0
    &OffboardMavros::action_transition,        // w
};
