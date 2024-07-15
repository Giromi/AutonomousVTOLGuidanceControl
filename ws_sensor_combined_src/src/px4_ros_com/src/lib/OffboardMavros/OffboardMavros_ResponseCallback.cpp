#include "px4_ros_com/OffboardMavros.hpp"

/* -- Callback Functions -- */
void OffboardMavros::offboardResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
    const char* msg[] = {
        "Offboard mode sent successfully", 
        "Failed to send Offboard mode"
    };
    printSuccessInfo(future.get()->mode_sent, msg);
}

void OffboardMavros::positionResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
    const char* msg[] = {
        "Position mode sent successfully", 
        "Failed to send Position mode"
    };
    printSuccessInfo(future.get()->mode_sent, msg);
}

void OffboardMavros::missionResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
    const char* msg[] = {
        "Mission mode sent successfully", 
        "Failed to send Mission mode"
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
void OffboardMavros::locationResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future) {
    const char* msg[] = {
        "Location command sent successfully", 
        "Failed to send location command"
    };
    printSuccessInfo(future.get()->success, msg);
}

void OffboardMavros::waypointPushResponseCallback(const rclcpp::Client<mavros_msgs::srv::WaypointPush>::SharedFuture future) {
    const char* msg[] = {
        "Waypoint push success", 
        "Waypoint push failed"
    };
    printSuccessInfo(future.get()->success, msg);
}

void OffboardMavros::waypointClearResponseCallback(const rclcpp::Client<mavros_msgs::srv::WaypointClear>::SharedFuture future) {
    const char* msg[] = {
        "Waypoint clear success", 
        "Waypoint clear failed"
    };
    printSuccessInfo(future.get()->success, msg);
}
