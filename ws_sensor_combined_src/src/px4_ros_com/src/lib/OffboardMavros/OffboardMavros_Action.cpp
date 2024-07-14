#include "px4_ros_com/OffboardMavros.hpp"

  
  /* -- Static Functions -- */

void OffboardMavros::_actionGoNorth(void) {
    //TODO make threshold
    _local_position[vtol::NORTH] += _offset;
}

void OffboardMavros::_actionGoEast(void) {
    //TODO make threshold
    _local_position[vtol::EAST] += _offset;
}

void OffboardMavros::_actionGoDown(void) {
    //TODO make threshold
    _local_position[vtol::UP] -= _offset;
}

void OffboardMavros::_actionGoSouth(void) {
    //TODO make threshold
    _local_position[vtol::NORTH] -= _offset;
}

void OffboardMavros::_actionGoWest(void) {
    //TODO make threshold
    _local_position[vtol::EAST] -= _offset;
}

void OffboardMavros::_actionGoUp(void) {
    //TODO make threshold
    _local_position[vtol::UP] += _offset;
}

void OffboardMavros::_actionVelocityPlusX(void) {
    _local_velocity[0] += _offset;
}

void OffboardMavros::_actionVelocityPlusY(void) {
    _local_velocity[1] += _offset;
}

void OffboardMavros::_actionVelocityPlusZ(void) {
    _local_velocity[2] += _offset;
}

void OffboardMavros::_actionVelocityMinusX(void) { 
    _local_velocity[0] -= _offset;
}

void OffboardMavros::_actionVelocityMinusY(void) {
    _local_velocity[1] -= _offset;
}

void OffboardMavros::_actionVelocityMinusZ(void) {
    _local_velocity[2] -= _offset;
}

void OffboardMavros::_actionVelocityPlusRoll(void) {
    _local_velocity[3] += _offset;
}

void OffboardMavros::_actionVelocityPlusPitch(void) {
    _local_velocity[4] += _offset;
}

void OffboardMavros::_actionVelocityPlusYaw(void) {
    _local_velocity[5] += _offset;
}

void OffboardMavros::_actionVelocityMinusRoll(void) {
    _local_velocity[3] -= _offset;
}

void OffboardMavros::_actionVelocityMinusPitch(void) {
    _local_velocity[4] -= _offset;
}

void OffboardMavros::_actionVelocityMinusYaw(void) {
    _local_velocity[5] -= _offset;
}

void OffboardMavros::_actionReturnHome(void) {
    //TODO make threshold
    // _local_position[vtol::NORTH] = 0.0;
    // _local_position[vtol::EAST] = 0.0;
    _local_velocity[0] = 0.0;
    _local_velocity[1] = 0.0;
    _local_velocity[2] = 0.0;
    _local_velocity[3] = 0.0;
    _local_velocity[4] = 0.0;
    _local_velocity[5] = 0.0;
}

void OffboardMavros::_actionArming(void) {
    if (OffboardMavros::_cmd_flag != vtol::READY) {
        std::cout << "Vehicle is NOT READY status" << std::endl;
        // return ;
    }
    OffboardMavros::_cmd_flag = vtol::ARMED;
}

void OffboardMavros::_actionDisarming(void) {
    if (OffboardMavros::_cmd_flag != vtol::ARMED) {
        std::cout << "Vehicle is NOT ARMED status" << std::endl;
        return ;
    }
    OffboardMavros::_cmd_flag = vtol::READY;
}

void OffboardMavros::_actionTakeoff(void) {

    // if (!(statusFlag & vtol::BIT_FLY)) {
    //     RCLCPP_INFO(this->get_logger(), "Vehicle is NOT ARMED status");
    //     return true;
    // if (OffboardMavros::_cmd_flag == vtol::READY) {
    //     std::cout << "Vehicle is NOT ARMED status" << std::endl;
    //     return ;
    // } else if (OffboardMavros::_cmd_flag == vtol::ARMED) {
    //     std::cout << "Calling takeoff service ..." << std::endl;
    // }
    OffboardMavros::_cmd_flag = vtol::TAKEOFF;
}

void OffboardMavros::_actionLanding(void) {
    OffboardMavros::_cmd_flag = vtol::LAND;
}

void OffboardMavros::_actionStart(void) {
    if (OffboardMavros::_cmd_flag == vtol::FLY || OffboardMavros::_cmd_flag == vtol::FIXED) {
        OffboardMavros::_cmd_flag = vtol::START;
    } else if (OffboardMavros::_cmd_flag == vtol::START 
            || OffboardMavros::_cmd_flag == vtol::MISSION) {
        OffboardMavros::_cmd_flag = vtol::FLY;
    }
}

void OffboardMavros::_actionMission(void) {
    OffboardMavros::_cmd_flag = vtol::MISSION;
}

void OffboardMavros::_actionHold(void) {
    OffboardMavros::_cmd_flag = vtol::FLY;
}

void OffboardMavros::_actionInit(void) {
    OffboardMavros::_cmd_flag = vtol::INIT;
}

void OffboardMavros::_actionTransition(void) {
    if (OffboardMavros::_cmd_flag == vtol::QUAD) {
        OffboardMavros::_cmd_flag = vtol::TO_FIXED;
    } else if (OffboardMavros::_cmd_flag == vtol::FIXED) {
        OffboardMavros::_cmd_flag = vtol::TO_QUAD;
    }
}


const std::array<std::string, vtol::ACTION_SIZE>	OffboardMavros::_action_string_array = { 
    "2", "4", "6", "3", "5", "7", 
    "↑", "↓", "→", "←", "+", "-", 
    "h", "a", "d", "t", "l", "s", "0", "w", "m",
};

void (*OffboardMavros::actionFunc[])(void) = {
    &OffboardMavros::_actionVelocityPlusRoll,       // 2
    &OffboardMavros::_actionVelocityPlusPitch,      // 4
    &OffboardMavros::_actionVelocityPlusYaw,        // 6
    &OffboardMavros::_actionVelocityMinusRoll,      // 3
    &OffboardMavros::_actionVelocityMinusPitch,     // 5
    &OffboardMavros::_actionVelocityMinusYaw,       // 7
    &OffboardMavros::_actionVelocityMinusY,         // ↑south
    &OffboardMavros::_actionVelocityPlusY,          // ↓north
    &OffboardMavros::_actionVelocityPlusX,          // → east
    &OffboardMavros::_actionVelocityMinusX,         // ← west
    &OffboardMavros::_actionVelocityPlusZ,          // + up
    &OffboardMavros::_actionVelocityMinusZ,         // - down
    &OffboardMavros::_actionReturnHome,       // h
    &OffboardMavros::_actionArming,            // a
    &OffboardMavros::_actionDisarming,         // d
    &OffboardMavros::_actionTakeoff,           // t
    &OffboardMavros::_actionLanding,           // l
    &OffboardMavros::_actionStart,             // s
    &OffboardMavros::_actionInit,              // 0
    &OffboardMavros::_actionTransition,        // w
    &OffboardMavros::_actionMission,           // m
};

