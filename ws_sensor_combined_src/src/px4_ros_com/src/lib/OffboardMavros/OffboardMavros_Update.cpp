#include "px4_ros_com/OffboardMavros.hpp"
/* -- Update Functions -- */

void OffboardMavros::updateArmingStatus(void) {
    requestArmingStatus(true, &OffboardMavros::armingResponseCallback);
}

void OffboardMavros::updateDisarmingStatus(void) {
    requestArmingStatus(false, &OffboardMavros::disarmingResponseCallback);
}

void OffboardMavros::updateTransitionFixedStatus(void) {
    // if (fcu_state.mode == vtol::MC) {
    requestTransitionStatus(vtol::FW, &OffboardMavros::transitionResponseCallback);
    // } else {
    // request_transition_status_(vtol::MC, &OffboardMavros::transitionResponseCallback);
}

void OffboardMavros::updateTransitionQuadStatus(void) {
    // if (fcu_state.mode == vtol::MC) {
    requestTransitionStatus(vtol::MC, &OffboardMavros::transitionResponseCallback);
    // } else {
    // request_transition_status_(vtol::MC, &OffboardMavros::transitionResponseCallback);
}

void OffboardMavros::requestTransitionStatus(const int input,
        void (OffboardMavros::*response_callback)
        (const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture)) {
    auto request = std::make_shared<mavros_msgs::srv::CommandVtolTransition::Request>();
    request->state = input;
    transition_client->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
    last_request = this->now();
}


void OffboardMavros::requestArmingStatus(const bool& input,
        void (OffboardMavros::*response_callback)
        (const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture)) {
    auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
    request->value = input;
    arming_client->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
    last_request = this->now();
}

void OffboardMavros::updateTakeoffStatus(void) {
    auto request = makeRequestTakeoffLandMessage(
            vtol::GeographicCoordinate{vtol::INIT_UP, 0, 0, 0, 0});
    takeoff_client->async_send_request(request,
            std::bind(&OffboardMavros::takeoffResponseCallback, this, std::placeholders::_1));
    last_request = this->now();
}

void OffboardMavros::updateLandingStatus(void) {
    auto request = makeRequestTakeoffLandMessage(
            vtol::GeographicCoordinate{0, 0, 0, 0, 0});
    landing_client->async_send_request(request,
            std::bind(&OffboardMavros::landResponseCallback, this, std::placeholders::_1));
    last_request = this->now();
}

std::shared_ptr<mavros_msgs::srv::CommandTOL::Request> OffboardMavros::makeRequestTakeoffLandMessage(const vtol::GeographicCoordinate& input) {
    auto request = std::make_shared<mavros_msgs::srv::CommandTOL::Request>();
    request->altitude   = input.altitude;
    request->latitude   = input.latitude;
    request->longitude  = input.longitude;
    request->min_pitch  = input.min_pitch;
    request->yaw        = input.yaw;
    return request;
}

void OffboardMavros::updateLocation(std::array<double, 3> input) {
    auto request = std::make_shared<mavros_msgs::srv::CommandLong::Request>();
    request->command = 16;
    request->param5 = input[vtol::NORTH]; // latitude
    request->param6 = input[vtol::EAST]; // longitude
    request->param7 = input[vtol::UP]; // altitude
    location_client->async_send_request(request, 
            std::bind(&OffboardMavros::locationResponseCallback, this, std::placeholders::_1));
    last_request = this->now();
}

void OffboardMavros::updateHoldMode(void) {
    updateCustomMode("AUTO.LOITER", &OffboardMavros::holdResponseCallback);
}

void OffboardMavros::updateOffboardMode(void) {
    updateCustomMode("OFFBOARD", &OffboardMavros::offboardResponseCallback);
}

void OffboardMavros::updateCustomMode(const std::string& input_mode,
        void (OffboardMavros::*response_callback)(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture)) {
    auto request = std::make_shared<mavros_msgs::srv::SetMode::Request>();
    request->custom_mode = input_mode;
    set_mode_client->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
    last_request = this->now();
}