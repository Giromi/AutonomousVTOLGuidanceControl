#include "px4_ros_com/OffboardMavros.hpp"

void OffboardMavros::stateCommandInit(void){
    RCLCPP_INFO(this->get_logger(), "< State Command Init >");
    if (fcu_state.armed == true) {
        updateLandingStatus();
    } else {
        updateWaypointClear();
        _cmd_flag = vtol::READY;
    }
}

void OffboardMavros::stateCommandReady(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Ready >");
    if (fcu_state.mode != vtol::FCU_HOLD) {    
        updateDisarmingStatus();
        updateHoldMode();
    }
}

void OffboardMavros::stateCommandArmed (void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Armed >");
    DEBUG::print("", ">> ARMED <<", BOLDGREEN);
    if (fcu_state.armed != true) {
        updateArmingStatus();
    } else {
        OffboardMavros::_cmd_flag = vtol::READY;
    }
}

void OffboardMavros::stateCommandMission(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Mission >");
    if (fcu_state.armed == false) {
        RCLCPP_ERROR(this->get_logger(), "Need to arm Mission mode");
        _cmd_flag = vtol::READY;
        return ;
    }

    if (fcu_state.mode != vtol::FCU_MISSION) {
        updateWaypointPush();
        updateMissionMode();
    }
}

void OffboardMavros::stateCommandFly (void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Fly >");
    if (fcu_state.mode != vtol::FCU_HOLD) {
        updateHoldMode();
    }
    std::cout << "Flying..." << std::endl;
}

void OffboardMavros::stateCommandTakeOff (void){
    RCLCPP_INFO(this->get_logger(), "< State Command Take Off >");
    DEBUG::print("", ">> Take Off <<", BOLDGREEN);

    // if (fcu_state.mode == vtol::FCU_HOLD && extended_msg.landed_state == vtol::LANDED_IN_AIR) {
    //     _cmd_flag = vtol::QUAD;
    if (fcu_state.armed == false) {
        updateArmingStatus();
    } else if (fcu_state.mode != vtol::FCU_TAKEOFF && fcu_state.armed == true) {
        updateTakeoffStatus();
    }
    // 순서 중요
    // if (fcu_state.mode != vtol::FCU_TAKEOFF && fcu_state.armed == true) {
    //     updateTakeoffStatus();
    // } else if (fcu_state.mode == vtol::FCU_TAKEOFF && fcu_state.armed == false) {
    //     updateArmingStatus();
    // }
}

void OffboardMavros::stateCommandFixed(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Fixed >");
    if (fcu_state.mode != vtol::FCU_POSITION) {
        updatePositionMode();
    }
}

void OffboardMavros::stateCommandStartMC(void) { 
    RCLCPP_INFO(this->get_logger(), "< State Command Start >");
    if (fcu_state.mode == vtol::FCU_HOLD 
        || fcu_state.mode == vtol::FCU_POSITION) {
        updateOffboardMode();
    }
    // if (fcu_state.mode != vtol::FCU_POSITION) {
        // updatePositionMode();
    // }
}

void OffboardMavros::stateCommandStartFW(void) { 
    RCLCPP_INFO(this->get_logger(), "< State Command Start >");
    if (fcu_state.mode == vtol::FCU_POSITION) {
        updateOffboardMode();
    }
}

void OffboardMavros::stateCommandToFixed(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command To Fixed >");
    updateTransitionFixedStatus();
}

void OffboardMavros::stateCommandToQuad(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command To Quad >");
    updateTransitionQuadStatus();
}

void OffboardMavros::stateCommandLand (void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Land >");
    if (fcu_state.mode != vtol::FCU_LAND && fcu_state.armed == true) {
        updateLandingStatus();
    } else if (fcu_state.mode == vtol::FCU_HOLD) {
        std::cout << "Landing success" << std::endl;
        OffboardMavros::_cmd_flag = vtol::INIT;
    }
}

void OffboardMavros::localPositionCommandStart(void) {
    if (reference_trajectory.empty()) {
        _cmd_flag = vtol::LAND;
        return;
    }

    Eigen::Vector3d UAV_position;
    UAV_position << _cur_position[vtol::EAST], _cur_position[vtol::NORTH], _cur_position[vtol::UP];
    
    _local_velocity[0] = reference_trajectory.front()->guidanceControl(UAV_position, 3.0).x();
    _local_velocity[1] = reference_trajectory.front()->guidanceControl(UAV_position, 3.0).y();
    _local_velocity[2] = reference_trajectory.front()->guidanceControl(UAV_position, 3.0).z();
    _local_velocity[4] = std::atan2(reference_trajectory.front()->guidanceControl(UAV_position, 3.0).y(), reference_trajectory.front()->guidanceControl(UAV_position, 3.0).x());
    
    DEBUG::printArray("Local Velocity Input : ", _local_velocity, 3, BOLDWHITE);
    DEBUG::printArray("Local Position       : ", _cur_position, 3, BOLDWHITE);
    DEBUG::print("Trajectory Size      : ", reference_trajectory.size(), BOLDWHITE);
    const bool check_arr = reference_trajectory.front()->isArrived(UAV_position, 3.0);
    if (check_arr){
        delete reference_trajectory.front();
        reference_trajectory.pop();
        std::cout << "\n\n\n\n\n\n\n\n" << std::endl;
    }
    // const Eigen::Vector4d cur_position_vector3d(
    //     _cur_position[0], _cur_position[1], _cur_position[2], vtol::NaN
    // );

    // if (wp_manager.isArrived(cur_position_vector3d)) {
    //     wp_manager.pop();
    // }
}
