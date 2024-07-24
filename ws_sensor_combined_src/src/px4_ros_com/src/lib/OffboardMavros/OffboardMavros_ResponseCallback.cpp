#include "px4_ros_com/OffboardMavros.hpp"

/* -- Callback Functions -- */

void OffboardMavros::modeSentResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2> msg) {
    const bool result = future.get()->mode_sent;
    printSuccessInfo(result, msg);
    if (result == true) {
        ;
    } else {
        ;
    }
}

void OffboardMavros::armingResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future) {
    const bool result = future.get()->success;
    const std::array<const std::string, 2> msg = {
        "Vehicle armed", 
        "Arming failed"
    };
    printSuccessInfo(result, msg);
    if (result == true) {
        ;
    } else {
        ;
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
        ;
    } else {
        ;
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
        ;
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
        ;
    } else {
        ;
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
        ;
    } else {
        ;
    }
}


// shared_future를 사용하는 이유는 비동기로 요청을 보내기 때문에 요청에 대한 응답을 받아야하기 때문이다.
void OffboardMavros::locationResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future) {
    const bool result = future.get()->success;
    const std::array<const std::string, 2> msg = {
        "Location command sent successfully", 
        "Failed to send location command"
    };
    printSuccessInfo(result, msg);
    if (result == true) {
        ;
    } else {
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
        ;
    } else {
        ;
    }
}

