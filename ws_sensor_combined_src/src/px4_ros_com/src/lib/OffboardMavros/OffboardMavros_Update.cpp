#include "px4_ros_com/OffboardMavros.hpp"
/* -- Update Functions -- */

void OffboardMavros::update_arming_status(void) {
    request_arming_status(true, &OffboardMavros::arming_response_callback);
}

void OffboardMavros::update_disarming_status(void) {
    request_arming_status(false, &OffboardMavros::disarming_response_callback);
}

void OffboardMavros::update_transition_fixed_status(void) {
    // if (fcuState_.mode == vtol::MC) {
    request_transition_status(vtol::FW, &OffboardMavros::transition_response_callback);
    // } else {
    // request_transition_status_(vtol::MC, &OffboardMavros::transition_response_callback);
}

void OffboardMavros::update_transition_quad_status(void) {
    // if (fcuState_.mode == vtol::MC) {
    request_transition_status(vtol::MC, &OffboardMavros::transition_response_callback);
    // } else {
    // request_transition_status_(vtol::MC, &OffboardMavros::transition_response_callback);
}

void OffboardMavros::sendFixedHeadingCommand(void) {
    auto request = std::make_shared<mavros_msgs::srv::CommandLong::Request>();
    request->command = vtol::MAV_CMD_CONDITION_YAW;
    request->param1 = yaw_current;  // 목표 yaw 각도
    request->param2 = 0;  // 회전 속도 (0이면 즉시 적용)
    request->param3 = 0;  // 1: CW, -1: CCW, 0: 가장 짧은 방향
    request->param4 = 0;  // 1: Relative, 0: Absolute
    request->confirmation = 0;

    cmd_client->async_send_request(request, std::bind(&OffboardMavros::cmdResponseCallback, this, std::placeholders::_1));

}

void OffboardMavros::cmdResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future) {
    const char* msg[] = {
        "CommandLong command sent successfully",
        "Failed to send CommandLong command"
    };
    print_success_info(future.get()->success, msg);
}


void OffboardMavros::request_transition_status(const int input,
        void (OffboardMavros::*response_callback)
        (const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture)) {
    auto request = std::make_shared<mavros_msgs::srv::CommandVtolTransition::Request>();
    request->state = input;
    // auto result_future = transition_client_->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
    auto result_future = transition_client_->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
    last_request_ = this->now();

//       // 결과를 비동기적으로 처리하고자 할 때
//     result_future.then([this](rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture future) {
//     try {
//         if (future.get()->success) {
//             RCLCPP_INFO(this->get_logger(), "Transition command executed successfully.");
//         } else {
//             RCLCPP_ERROR(this->get_logger(), "Failed to execute transition command.");
//         }
//     } catch (const std::exception& e) {
//         RCLCPP_ERROR(this->get_logger(), "Exception while getting future result: %s", e.what());
//     }
// });
}


void OffboardMavros::request_arming_status(const bool& input,
        void (OffboardMavros::*response_callback)
        (const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture)) {
    auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
    request->value = input;
    arming_client_->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
    last_request_ = this->now();
}

void OffboardMavros::update_takeoff_status(void) {
    auto request = make_request_takeoff_land_message(
            vtol::GeographicCoordinate{global_position_[vtol::ALT]+20, global_position_[vtol::LAT], global_position_[vtol::LON], 0, 0});
    takeoff_client_->async_send_request(request,
            std::bind(&OffboardMavros::takeoff_response_callback, this, std::placeholders::_1));
    last_request_ = this->now();
}

void OffboardMavros::update_landing_status(void) {
    auto request = make_request_takeoff_land_message(
            vtol::GeographicCoordinate{0, 0, 0, 0, 0});
    landing_client_->async_send_request(request,
            std::bind(&OffboardMavros::land_response_callback, this, std::placeholders::_1));
    last_request_ = this->now();
}

std::shared_ptr<mavros_msgs::srv::CommandTOL::Request> OffboardMavros::make_request_takeoff_land_message(const vtol::GeographicCoordinate& input) {
    auto request = std::make_shared<mavros_msgs::srv::CommandTOL::Request>();
    request->altitude   = input.altitude;
    request->latitude   = input.latitude;
    request->longitude  = input.longitude;
    request->min_pitch  = input.min_pitch;
    request->yaw        = input.yaw;
    return request;
}

void OffboardMavros::update_location_(std::array<double, 3> input) {
    auto request = std::make_shared<mavros_msgs::srv::CommandLong::Request>();
    request->command = 16;
    request->param5 = input[vtol::NORTH]; // latitude
    request->param6 = input[vtol::EAST]; // longitude
    request->param7 = input[vtol::UP]; // altitude
    location_client_->async_send_request(request, 
            std::bind(&OffboardMavros::location_response_callback, this, std::placeholders::_1));
    last_request_ = this->now();
}

void OffboardMavros::update_hold_mode(void) {
    update_custom_mode("AUTO.LOITER", &OffboardMavros::hold_response_callback);
}

void OffboardMavros::update_offboard_mode(void) {
    update_custom_mode("OFFBOARD", &OffboardMavros::offboard_response_callback);
}

void OffboardMavros::update_custom_mode(const std::string& input_mode,
        void (OffboardMavros::*response_callback)(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture)) {
    auto request = std::make_shared<mavros_msgs::srv::SetMode::Request>();
    request->custom_mode = input_mode;
    set_mode_client_->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
    last_request_ = this->now();
}
