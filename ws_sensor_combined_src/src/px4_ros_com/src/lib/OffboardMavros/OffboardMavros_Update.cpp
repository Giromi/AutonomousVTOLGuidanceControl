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

void OffboardMavros::request_transition_status(const int input,
        void (OffboardMavros::*response_callback)
        (const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture)) {
    auto request = std::make_shared<mavros_msgs::srv::CommandVtolTransition::Request>();
    request->state = input;
    transition_client_->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
    last_request_ = this->now();
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