#include "px4_ros_com/OffboardMavros.hpp"

void OffboardMavros::statusReady(void) {
    if (OffboardMavros::_cmd_flag != vtol::READY) {
        return ;
    }
    if (fcu_state.mode != vtol::FCU_HOLD) {
        OffboardMavros::_cmd_flag = vtol::READY;
        updateCustomMode("AUTO.LOITER",
                    &OffboardMavros::holdResponseCallback);
        updateDisarmingStatus();
    }
}

void OffboardMavros::stateCommandInit(void){
    RCLCPP_INFO(this->get_logger(), "< State Command Init >");
    if (fcu_state.armed == true) {
        updateLandingStatus();
    } else {
        updateWaypointClear();
        OffboardMavros::_cmd_flag = vtol::READY;
    }
}

void OffboardMavros::stateCommandReady(void) {
    RCLCPP_INFO(this->get_logger(), "< State Command Ready >");
    if (fcu_state.mode != vtol::FCU_HOLD) {    
        updateDisarmingStatus();
        updateHoldMode();
        updateCustomMode(vtol::FCU_HOLD, &OffboardMavros::holdResponseCallback);
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
        OffboardMavros::_cmd_flag = vtol::READY;
    }
}

void OffboardMavros::localPositionCommandStart(void) {

    const Eigen::Vector4d cur_position_vector3d(
        _cur_position[0], 
        _cur_position[1], 
        _cur_position[2], 
        vtol::NaN
    );

    if (wp_manager.isArrived(cur_position_vector3d)) {
        wp_manager.pop();
    }
}