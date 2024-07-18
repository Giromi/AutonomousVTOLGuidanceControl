
#include "px4_ros_com/OffboardMavros.hpp"


void OffboardMavros::statusCommandBitsHandler(void) {
    if (_stt_cmd_flag == vtol::STAND_BY) return ;
    // bitsCommandHandler();
    bitsStatusHandlerState();
    bitsStatusHandlerExtendedState();
}

void OffboardMavros::bitsStatusHandlerState(void) {
    if (fcu.state.first == fcu.state.second.header.stamp) {
        return ;
    }

    if (fcu.state.second.armed == true) {
        /** CMD_ARM_DISARM 비트를 끄고 ARMED 비트를 켠다 */
        _stt_cmd_flag |= vtol::STT_ARMED;
        _stt_cmd_flag &= ~vtol::CMD_ARM_DISARM;
    } else {
        /** FCU_TAKEOFF 상태일 때에는 
         *  안전상 이후 꺼지게 때문에 다시 아밍 필요 */
        if (fcu.state.second.mode != vtol::FCU_TAKEOFF) {
            _stt_cmd_flag &= ~vtol::STT_ARMED;
        }
    }

    switch (fcu.state.second.system_status) {
        case vtol::mavlink::State::UNINIT:
            break;
        case vtol::mavlink::State::BOOT:
            break;
        case vtol::mavlink::State::CALIBRATING:
            break;
        case vtol::mavlink::State::STANDBY:
            // if (_stt_cmd_flag == vtol::READY) {
            //     DEBUG::print("State Command : ", vtol::ARMING, YELLOW);
            //     _stt_cmd_flag |= vtol::CMD_ARM_DISARM;
            // }
            break;
        case vtol::mavlink::State::ACTIVE:
            break;
        case vtol::mavlink::State::CRITICAL:
            break;
        case vtol::mavlink::State::EMERGENCY:
            break;
        case vtol::mavlink::State::POWEROFF:
            break;
        case vtol::mavlink::State::FLIGHT_TERMINATION:
            break;
    }


}


void OffboardMavros::bitsStatusHandlerExtendedState(void) {
    if (fcu.extended_state.first == fcu.extended_state.second.header.stamp) {
        return ;
    }

    switch (fcu.extended_state.second.vtol_state) {
        case mavros_msgs::msg::ExtendedState::VTOL_STATE_UNDEFINED:
            break;
        case mavros_msgs::msg::ExtendedState::VTOL_STATE_TRANSITION_TO_FW:
            if (_stt_cmd_flag == vtol::MC_TO_FIXED) {
                _stt_cmd_flag |= vtol::CMD_TRANSITION;
            }
            break;
        case mavros_msgs::msg::ExtendedState::VTOL_STATE_TRANSITION_TO_MC:
            if (_stt_cmd_flag == vtol::FW_TO_QUAD) {
                _stt_cmd_flag |= vtol::CMD_TRANSITION;
            }
            break;
        case mavros_msgs::msg::ExtendedState::VTOL_STATE_MC:
            _stt_cmd_flag &= ~vtol::STT_FIXED;
            break;
        case mavros_msgs::msg::ExtendedState::VTOL_STATE_FW:
            _stt_cmd_flag |= vtol::STT_FIXED;
            break;
    }

    switch (fcu.extended_state.second.landed_state) {
        case mavros_msgs::msg::ExtendedState::LANDED_STATE_UNDEFINED:
            break;
        case mavros_msgs::msg::ExtendedState::LANDED_STATE_ON_GROUND:
            _stt_cmd_flag &= ~vtol::STT_FLY;
            break;
        case mavros_msgs::msg::ExtendedState::LANDED_STATE_IN_AIR:
            /* 이륙하는 중에는 Quad 상태 아님 */
            if (_stt_cmd_flag == vtol::TAKING_OFF) {
                return ;
            }
            _stt_cmd_flag |= vtol::STT_FLY;
            break;
        case mavros_msgs::msg::ExtendedState::LANDED_STATE_TAKEOFF:
            _stt_cmd_flag &= ~vtol::STT_FLY;
            _stt_cmd_flag |= vtol::CMD_TAKEOFF_LAND;
            break;
        case mavros_msgs::msg::ExtendedState::LANDED_STATE_LANDING:
            _stt_cmd_flag |= vtol::STT_FLY;
            _stt_cmd_flag |= vtol::CMD_TAKEOFF_LAND;
            break;
    }

}

