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



void OffboardMavros::sendFixedHeadingCommand(void) {
    auto request = std::make_shared<mavros_msgs::srv::CommandLong::Request>();
    request->command = vtol::MAV_CMD_CONDITION_YAW;
    request->param1 = yaw_current;  // 목표 yaw 각도
    request->param2 = 0;  // 회전 속도 (0이면 즉시 적용)
    request->param3 = 0;  // 1: CW, -1: CCW, 0: 가장 짧은 방향
    request->param4 = 0;  // 1: Relative, 0: Absolute
    request->confirmation = 0;

    cmd_client->async_send_request(request, std::bind(&OffboardMavros::cmdResponseCallback, this, std::placeholders::_1));
    last_request = this->now();
}

void OffboardMavros::cmdResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future) {
    const char* msg[] = {
        "CommandLong command sent successfully",
        "Failed to send CommandLong command"
    };
    printSuccessInfo(future.get()->success, msg);
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
            vtol::GeographicCoordinate{_global_position[vtol::ALT]+20, _global_position[vtol::LAT], _global_position[vtol::LON], 0, 0});
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
    updateCustomMode(vtol::FCU_HOLD, &OffboardMavros::holdResponseCallback);
}

void OffboardMavros::updateOffboardMode(void) {
    updateCustomMode(vtol::FCU_OFFBOARD, &OffboardMavros::offboardResponseCallback);
}

void OffboardMavros::updatePositionMode(void) {
    updateCustomMode(vtol::FCU_POSITION, &OffboardMavros::positionResponseCallback);
}

void OffboardMavros::updateMissionMode(void) {
    updateCustomMode(vtol::FCU_MISSION, &OffboardMavros::missionResponseCallback);
}


void OffboardMavros::updateCustomMode(const std::string& input_mode,
        void (OffboardMavros::*response_callback)(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture)) {
    auto request = std::make_shared<mavros_msgs::srv::SetMode::Request>();
    request->custom_mode = input_mode;
    set_mode_client->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
    last_request = this->now();
}

void OffboardMavros::updateWaypointClear(void) {
    if (!waypoint_clear_client->wait_for_service(std::chrono::seconds(10))) {
        RCLCPP_ERROR(this->get_logger(), "WaypointClear service not available");
        return;
    }
    auto request = std::make_shared<mavros_msgs::srv::WaypointClear::Request>();
    waypoint_clear_client->async_send_request(request, std::bind(&OffboardMavros::waypointClearResponseCallback, this, std::placeholders::_1));
    last_request = this->now();
}


void OffboardMavros::updateWaypointPush(void) {
    if (!waypoint_push_client->wait_for_service(std::chrono::seconds(10))) {
        RCLCPP_ERROR(this->get_logger(), "WaypointPush service not available");
        return;
    }
    auto request = std::make_shared<mavros_msgs::srv::WaypointPush::Request>();
    request->waypoints = waypoint_list.waypoints;
    waypoint_push_client->async_send_request(request, std::bind(&OffboardMavros::waypointPushResponseCallback, this, std::placeholders::_1));
    last_request = this->now();

}
