#include "px4_ros_com/OffboardMavros.hpp"

t_bit                                   OffboardMavros::_stt_cmd_flag = vtol::CHECK;
std::array<double, 3>		            OffboardMavros::_local_position{vtol::INIT_NORTH, vtol::INIT_EAST, vtol::INIT_UP};
std::array<double, 6>		            OffboardMavros::_local_velocity{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};  
std::array<float, 3>		            OffboardMavros::_global_position{-1.0f, -1.0f, -1.0f}; 
std::array<double, 3>		            OffboardMavros::_cur_position{};
std::array<double, 3>		            OffboardMavros::_prev_position{};
double                                  OffboardMavros::_offset = 0.1;
std::array<double, 4>                   OffboardMavros::_manual_velocity = {10 , 0.0, 0.0, 0.0};

/**
 * @brief OffboardMavros 생성자
 */
    OffboardMavros::OffboardMavros(void) : Node("offboard_mavros") {
     // initializeConstant();
    // DEBUG::print("OffboardMavros Constant", true, BOLDGREEN);
    initializeVariables();
    DEBUG::print("OffboardMavros Variables", true, BOLDGREEN);
    initializeFunctionPointerArray();
    DEBUG::print("OffboardMavros FuctionPointer", true, BOLDGREEN);
    initializePublishers();
    DEBUG::print("OffboardMavros Publishers", true, BOLDGREEN);
    initializeSubscribers();
    DEBUG::print("OffboardMavros Subscribers", true, BOLDGREEN);
    initializeClients();
    DEBUG::print("OffboardMavros Clients", true, BOLDGREEN);
    initializeWaypoints();
    DEBUG::print("OffboardMavros Waypoints", true, BOLDGREEN);
    // initialIsConnectionSafe();
    // DEBUG::print("OffboardMavros Connection", true, BOLDGREEN);
    initializeTimers(30); 
    DEBUG::print("OffboardMavros Timers", true, BOLDGREEN);
}


void    OffboardMavros::vtolCommandCenter(void) {
    const std::array<t_bit, STATE_SIZE>::iterator  it = std::find(stt_cmd_value_array.begin(), stt_cmd_value_array.end(), _stt_cmd_flag);
    if (it == stt_cmd_value_array.end()) {
        RCLCPP_ERROR(this->get_logger(), " Invalid State ");
        // _stt_cmd_flag = vtol::CHECK;
    } else {
        const size_t i = std::distance(stt_cmd_value_array.begin(), it);
        statusCommandFunc[i]();
    }
    watchVtolStatus();
}


void    OffboardMavros::watchVtolStatus(void) {
    // if (isConnectionSafe() == false) {
    //     return ;
    // }
    // const bool is_state_recv = ifTimeNotSameInput(fcu.state.first, fcu.state.second.header.stamp);
    // const bool is_ext_state_recv = ifTimeNotSameInput(fcu.extended_state.first, fcu.extended_state.second.header.stamp);
    // const bool is_local_pos_recv = ifTimeNotSameInput(fcu.local_position.first, fcu.local_position.second.header.stamp);
    // const bool is_global_pos_recv = ifTimeNotSameInput(fcu.global_position.first, fcu.global_position.second.header.stamp);

    DEBUG::message("\n[DEBUG] -----------------", BOLDWHITE);
    DEBUG::printBinary("Command flag    : ", _stt_cmd_flag, BOLDYELLOW);
    DEBUG::message("[DEBUG] -----------------", BOLDWHITE);
    std::string color(WHITE);
    color = GREEN; // is_state_recv ? BOLDYELLOW : WHITE;
    DEBUG::print("System status   : ", static_cast<unsigned int>(fcu.state.second.system_status), color.c_str());
    DEBUG::print("State Mode      : ", fcu.state.second.mode, color.c_str());
    DEBUG::printBool("Arming          : ", fcu.state.second.armed, color.c_str());

    color = GREEN;// is_ext_state_recv ? BOLDYELLOW : WHITE;
    DEBUG::print("VTOL State      : ", static_cast<unsigned int>(fcu.extended_state.second.vtol_state), color.c_str());
    DEBUG::print("Landed State    : ", static_cast<unsigned int>(fcu.extended_state.second.landed_state), color.c_str());

    color = CYAN; //is_local_pos_recv ? BOLDYELLOW : WHITE;
    DEBUG::message("[DEBUG] -----------------", BOLDWHITE);
    DEBUG::print("Local Position  : ", fcu.local_position.second.pose.position.x, color.c_str());
    DEBUG::print("Local Position  : ", fcu.local_position.second.pose.position.y, color.c_str());
    DEBUG::print("Local Position  : ", fcu.local_position.second.pose.position.z, color.c_str());

    DEBUG::message("[DEBUG] -----------------", BOLDWHITE);
    color = WHITE; //is_global_pos_recv ? BOLDYELLOW : WHITE;
    DEBUG::print("Attitude        : ", fcu.global_position.second.altitude, color.c_str());
    DEBUG::print("Latitude        : ", fcu.global_position.second.latitude, color.c_str());
    DEBUG::print("Longitude       : ", fcu.global_position.second.longitude, color.c_str());
    DEBUG::message("[DEBUG] -----------------", BOLDWHITE);
    // DEBUG::printArray("local_position        : ", _local_position, 3, MAGENTA);
    DEBUG::printArray("init_global_position : ", init_global_position, 3, MAGENTA);
    DEBUG::printArray("local_velocity       : ", _local_velocity, 6, MAGENTA);
    DEBUG::message("[DEBUG] -----------------", BOLDWHITE);
    DEBUG::print("Waypoint Size            : ", wp_manager.getSize(), BOLDYELLOW);
    DEBUG::print("Waypoint Target          : ", wp_manager.getTarget().transpose(), BOLDYELLOW);
    DEBUG::message("[DEBUG] -----------------\n", BOLDWHITE);
    statusCommandBitsHandler();
}   

