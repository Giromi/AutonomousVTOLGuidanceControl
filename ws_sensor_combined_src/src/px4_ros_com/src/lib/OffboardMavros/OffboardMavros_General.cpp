#include "px4_ros_com/OffboardMavros.hpp"


unsigned char                           OffboardMavros::_cmd_flag = vtol::INIT;
std::array<double, 3>		            OffboardMavros::_local_position{vtol::INIT_NORTH, vtol::INIT_EAST, vtol::INIT_UP};
std::array<double, 6>		            OffboardMavros::_local_velocity{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};  
std::array<float, 3>		            OffboardMavros::_global_position{-1.0f, -1.0f, -1.0f}; 
std::array<double, 3>		            OffboardMavros::_cur_position{};
std::array<double, 3>		            OffboardMavros::_prev_position{};
double                                  OffboardMavros::_offset = 1.0;


/**
 * @brief OffboardMavros 생성자
 */
OffboardMavros::OffboardMavros(void) : Node("offboard_mavros") {
    // initializeConstant();
    // DEBUG::print("OffboardMavros Constant", true, BOLDGREEN);
    initializeVariables();
    DEBUG::print("OffboardMavros Variables", true, BOLDGREEN);
    initializeFunctionPointer();
    DEBUG::print("OffboardMavros FuctionPointer", true, BOLDGREEN);
    initializePublishers();
    DEBUG::print("OffboardMavros Publishers", true, BOLDGREEN);
    initializeSubscribers();
    DEBUG::print("OffboardMavros Subscribers", true, BOLDGREEN);
    initializeClients();
    DEBUG::print("OffboardMavros Clients", true, BOLDGREEN);
    initializeTimers(50); 
    DEBUG::print("OffboardMavros Timers", true, BOLDGREEN);
}


