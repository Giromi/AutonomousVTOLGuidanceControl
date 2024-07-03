#include "px4_ros_com/OffboardMavros.hpp"


unsigned char                           OffboardMavros::_cmd_flag = vtol::INIT;
std::array<double, 3>		            OffboardMavros::_local_position{vtol::INIT_NORTH, vtol::INIT_EAST, vtol::INIT_UP};
std::array<double, 6>		            OffboardMavros::_local_velocity{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};  
std::array<double, 3>		            OffboardMavros::_cur_position{};
std::array<double, 3>		            OffboardMavros::_prev_position{};
double                                  OffboardMavros::_offset = 0.5;

/**
 * @brief OffboardMavros 생성자
 */
OffboardMavros::OffboardMavros(void) : Node("offboard_mavros") {
    initializePublishers();
    initializeSubscribers();
    initializeClients();
    initializeTimers(50); 
}
