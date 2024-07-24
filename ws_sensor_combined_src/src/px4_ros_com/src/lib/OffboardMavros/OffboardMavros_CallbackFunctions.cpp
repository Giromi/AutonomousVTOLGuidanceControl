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

     if (msg->status.status >= sensor_msgs::msg::NavSatStatus::STATUS_FIX) {
            gps_locked = true;
        }

    if (isGlobalPositionGettingValue(init_global_position) == false) {
        init_global_position[vtol::ALT] = msg->altitude + 10.0f;
        init_global_position[vtol::LAT] = msg->latitude;
        init_global_position[vtol::LON] = msg->longitude;
        DEBUG::print("alt: ", init_global_position[vtol::ALT],BLUE);
        DEBUG::print("lat: ", init_global_position[vtol::LAT],BLUE);
        DEBUG::print("lon: ", init_global_position[vtol::LON],BLUE);
    }
  
    _global_position[vtol::ALT] = msg->altitude;
    _global_position[vtol::LAT] = msg->latitude;
    _global_position[vtol::LON] = msg->longitude;

    // DEBUG::print("gps_alt: ", _global_position[vtol::ALT], GREEN);
    // DEBUG::print("gps_lat: ", _global_position[vtol::LAT], GREEN);
    // DEBUG::print("gps_lon: ", _global_position[vtol::LON], GREEN);
}
    /* -- StateCommand Function*/

void OffboardMavros::localPositionCommandStart(void) {

    const Eigen::Vector4d cur_position_vector3d(
        _cur_position[0], _cur_position[1], _cur_position[2], vtol::NaN
    );

    if (wp_manager.isArrived(cur_position_vector3d)) {
        wp_manager.pop();
    }
}
//


// /* -- Callback Functions -- */
// void OffboardMavros::offboardResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2> msg) {
//     const bool result = future.get()->mode_sent;
//     printSuccessInfo(result, msg);
// }
//
// void OffboardMavros::positionResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2> msg) {
//     const bool result = future.get()->mode_sent;
//     printSuccessInfo(result, msg);
// }
//
// void OffboardMavros::missionResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2> msg) {
//     const bool result = future.get()->mode_sent;
//     printSuccessInfo(result, msg);
//     if (result == true) {
//         ;
//     } else {
//         ;
//     }
// }
//
// void	OffboardMavros::holdResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2> msg) {
//     const bool result = future.get()->mode_sent;
//     printSuccessInfo(result, msg);
//     // if (result == true) {
//     //     ;
//     // } else {
//     //     ;
//     // }
// }


// void OffboardMavros::turnOffBitIfPassedSeconds(const t_bit bit, const double timer) {
//     last_request = this->now();
// }

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
        if (_global_position[vtol::ALT] > init_global_position[vtol::ALT] - 1) {
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

    if (straight_trajectory.empty()) {
        _cmd_flag = vtol::LAND;

        return;
    }

    Eigen::Vector3d UAV_position;
    UAV_position << _cur_position[vtol::EAST], _cur_position[vtol::NORTH], _cur_position[vtol::UP];
    
    _local_velocity[0] = straight_trajectory.front().guidanceControl(UAV_position, 3.0).x();
    _local_velocity[1] = straight_trajectory.front().guidanceControl(UAV_position, 3.0).y();
    _local_velocity[2] = straight_trajectory.front().guidanceControl(UAV_position, 3.0).z();
    _local_velocity[4] = std::atan2(straight_trajectory.front().guidanceControl(UAV_position, 3.0).y(), straight_trajectory.front().guidanceControl(UAV_position, 3.0).x());
    DEBUG::printArray("Local Velocity Input : ", _local_velocity, 3, BOLDWHITE);
    DEBUG::printArray("Local Position       : ", _cur_position, 3, BOLDWHITE);
    DEBUG::print("Trajectory Size      : ", straight_trajectory.size(), BOLDWHITE);
    const bool check_arr = straight_trajectory.front().isArrived(UAV_position, 3.0);
    if (check_arr){
        straight_trajectory.pop();
    }
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

