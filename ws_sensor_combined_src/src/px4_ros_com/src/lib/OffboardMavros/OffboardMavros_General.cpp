#include "px4_ros_com/OffboardMavros.hpp"


unsigned char                           OffboardMavros::cmdFlag_ = vtol::INIT;
std::array<double, 3>		            OffboardMavros::local_position_{vtol::INIT_NORTH, vtol::INIT_EAST, vtol::INIT_UP};
std::array<double, 6>		            OffboardMavros::local_velocity_{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};  
std::array<double, 3>		            OffboardMavros::cur_position_{};
std::array<double, 3>		            OffboardMavros::prev_position_{};
double                                  OffboardMavros::offset_ = 0.5;

/**
 * @brief OffboardMavros 생성자
 */
OffboardMavros::OffboardMavros(void) : Node("offboard_mavros") {
    initializePublishers();
    initializeSubscribers();
    initializeClients();
    initializeTimers(50); 
}
