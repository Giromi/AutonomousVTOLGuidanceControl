#include "px4_ros_com/OffboardMavros.hpp"

  
  /* -- Static Functions -- */

void OffboardMavros::action_go_north(void) {
    //TODO make threshold
    local_position_[vtol::NORTH] += offset_;
}

void OffboardMavros::action_go_east(void) {
    //TODO make threshold
    local_position_[vtol::EAST] += offset_;
}

void OffboardMavros::action_go_down(void) {
    //TODO make threshold
    local_position_[vtol::UP] -= offset_;
}

void OffboardMavros::action_go_south(void) {
    //TODO make threshold
    local_position_[vtol::NORTH] -= offset_;
}

void OffboardMavros::action_go_west(void) {
    //TODO make threshold
    local_position_[vtol::EAST] -= offset_;
}

void OffboardMavros::action_go_up(void) {
    //TODO make threshold
    local_position_[vtol::UP] += offset_;
}

void OffboardMavros::action_velocity_plus_x(void) {
    local_velocity_[0] += offset_;
}

void OffboardMavros::action_velocity_minus_x(void) {
    local_velocity_[0] -= offset_;
}

void OffboardMavros::action_velocity_plus_y(void) {
    local_velocity_[1] += offset_;
}

void OffboardMavros::action_velocity_minus_y(void) {
    local_velocity_[1] -= offset_;
}
void OffboardMavros::action_velocity_plus_z(void) {
    local_velocity_[2] += offset_;
}

void OffboardMavros::action_velocity_minus_z(void) {
    local_velocity_[2] -= offset_;
}

void OffboardMavros::action_velocity_plus_roll(void) {
    local_velocity_[3] += offset_;
}

void OffboardMavros::action_velocity_plus_pitch(void) {
    local_velocity_[4] += offset_;
}

void OffboardMavros::action_velocity_plus_yaw(void) {
    local_velocity_[5] += offset_;
}

void OffboardMavros::action_velocity_minus_roll(void) {
    local_velocity_[3] -= offset_;
}

void OffboardMavros::action_velocity_minus_pitch(void) {
    local_velocity_[4] -= offset_;
}

void OffboardMavros::action_velocity_minus_yaw(void) {
    local_velocity_[5] -= offset_;
}

void OffboardMavros::action_return_home(void) {
    //TODO make threshold
    // local_position_[vtol::NORTH] = 0.0;
    // local_position_[vtol::EAST] = 0.0;
    local_velocity_[0] = 0.0;
    local_velocity_[1] = 0.0;
    local_velocity_[2] = 0.0;
    local_velocity_[3] = 0.0;
    local_velocity_[4] = 0.0;
    local_velocity_[5] = 0.0;
}

void OffboardMavros::action_arming(void) {
    if (OffboardMavros::cmdFlag_ != vtol::READY) {
        std::cout << "Vehicle is NOT READY status" << std::endl;
        return ;
    }
    OffboardMavros::cmdFlag_ = vtol::ARMED;
}

void OffboardMavros::action_disarming(void) {
    if (OffboardMavros::cmdFlag_ != vtol::ARMED) {
        std::cout << "Vehicle is NOT ARMED status" << std::endl;
        return ;
    }
    OffboardMavros::cmdFlag_ = vtol::READY;
}

void OffboardMavros::action_takeoff(void) {

    // if (!(statusFlag & vtol::BIT_FLY)) {
    //     RCLCPP_INFO(this->get_logger(), "Vehicle is NOT ARMED status");
    //     return true;
    if (OffboardMavros::cmdFlag_ == vtol::READY) {
        std::cout << "Vehicle is NOT ARMED status" << std::endl;
        return ;
    } else if (OffboardMavros::cmdFlag_ == vtol::ARMED) {
        std::cout << "Calling takeoff service ..." << std::endl;
    }
    OffboardMavros::cmdFlag_ = vtol::TAKEOFF;
}

void OffboardMavros::action_landing(void) {
    OffboardMavros::cmdFlag_ = vtol::LAND;
}

void OffboardMavros::action_start(void) {
    if (OffboardMavros::cmdFlag_ == vtol::QUAD || OffboardMavros::cmdFlag_ == vtol::FIXED) {
        OffboardMavros::cmdFlag_ = vtol::START;
    } else if (OffboardMavros::cmdFlag_ == vtol::START) {
        OffboardMavros::cmdFlag_ = vtol::FLY;
    }
}

void OffboardMavros::action_hold(void) {
    OffboardMavros::cmdFlag_ = vtol::FLY;
}

void OffboardMavros::action_init(void) {
    OffboardMavros::cmdFlag_ = vtol::INIT;
}

void OffboardMavros::action_transition(void) {
    if (OffboardMavros::cmdFlag_ == vtol::QUAD) {
        OffboardMavros::cmdFlag_ = vtol::TO_FIXED;
    } else if (OffboardMavros::cmdFlag_ == vtol::FIXED) {
        OffboardMavros::cmdFlag_ = vtol::TO_QUAD;
    }
}


const std::array<std::string, vtol::ACTION_SIZE>	OffboardMavros::action_string_array_ = { 
    "2", "4", "6", "3", "5", "7", 
    "↑", "↓", "→", "←", "+", "-", 
    "h", "a", "d", "t", "l", "s", "0", "w",  
};

void (*OffboardMavros::action_func_[])(void) = {
    &OffboardMavros::action_velocity_plus_roll, // 2
    &OffboardMavros::action_velocity_plus_pitch,// 4
    &OffboardMavros::action_velocity_plus_yaw,  // 6
    &OffboardMavros::action_velocity_minus_roll,  // 3
    &OffboardMavros::action_velocity_minus_pitch,  // 5
    &OffboardMavros::action_velocity_minus_yaw,    // 7
    &OffboardMavros::action_velocity_plus_y,       // ↑ west
    &OffboardMavros::action_velocity_minus_y,      // ↓ south
    &OffboardMavros::action_velocity_plus_x,       // → 
    &OffboardMavros::action_velocity_minus_x,      // ← east
    &OffboardMavros::action_velocity_plus_z,       // + up
    &OffboardMavros::action_velocity_minus_z,      // - down
    &OffboardMavros::action_return_home,       // h
    &OffboardMavros::action_arming,            // a
    &OffboardMavros::action_disarming,         // d
    &OffboardMavros::action_takeoff,           // t
    &OffboardMavros::action_landing,           // l
    &OffboardMavros::action_start,             // s
    &OffboardMavros::action_init,              // 0
    &OffboardMavros::action_transition,        // w
};
