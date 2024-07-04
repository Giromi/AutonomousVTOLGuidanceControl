
#include "px4_ros_com/OffboardMavros.hpp"

unsigned char                           OffboardMavros::_cmd_flag = vtol::INIT;
std::array<double, 3>		            OffboardMavros::_local_position{vtol::INIT_NORTH, vtol::INIT_EAST, vtol::INIT_UP};
std::array<double, 6>		            OffboardMavros::_local_velocity{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};  
std::array<double, 3>		            OffboardMavros::_cur_position{};
std::array<double, 3>		            OffboardMavros::_prev_position{};

const std::array<std::string, vtol::ACTION_SIZE>	OffboardMavros::_action_string_array = { 
    "2", "4", "6", "3", "5", "7", 
    "↑", "↓", "→", "←", "+", "-", 
    "h", "a", "d", "t", "l", "s", "0", "w",  
};

void (*OffboardMavros::actionFunc[])(void) = {
    &OffboardMavros::action_velocity_plus_roll_, // 2
    &OffboardMavros::action_velocity_plus_pitch_,// 4
    &OffboardMavros::action_velocity_plus_yaw_,  // 6
    &OffboardMavros::action_velocity_minus_roll_,  // 3
    &OffboardMavros::action_velocity_minus_pitch_,  // 5
    &OffboardMavros::action_velocity_minus_yaw_,    // 7
    &OffboardMavros::action_velocity_plus_z_,           // ↑ up
    &OffboardMavros::action_velocity_minus_z_,          // ↓ down
    &OffboardMavros::action_velocity_minus_y_,      // ← south
    &OffboardMavros::action_velocity_plus_y_,       // → north
    &OffboardMavros::action_velocity_plus_x_,       // + east
    &OffboardMavros::action_velocity_minus_x_,      // - west
    &OffboardMavros::action_return_home_,       // h
    &OffboardMavros::action_arming_,            // a
    &OffboardMavros::action_disarming_,         // d
    &OffboardMavros::action_takeoff_,           // t
    &OffboardMavros::action_landing_,           // l
    &OffboardMavros::action_start_,             // s
    &OffboardMavros::action_init_,              // 0
    &OffboardMavros::action_transition_,        // w
};

double                    OffboardMavros::_offset = 0.5;


/**
 * @brief OffboardMavros 생성자
 */
OffboardMavros::OffboardMavros(void) : Node("offboard_mavros") {
    initializePublishers();
    initializeSubscribers();
    initializeClients();
    // initializeArrays();
    // Ready
    initializeTimers(50); 
}

void OffboardMavros::initializePublishers(void) {
    local_pos_pub = create_publisher<geometry_msgs::msg::PoseStamped>("/mavros/setpoint_position/local", 10);

    local_vel_pub = this->create_publisher<geometry_msgs::msg::TwistStamped>("/mavros/setpoint_velocity/cmd_vel", 10);
    local_pub = this->create_publisher<mavros_msgs::msg::PositionTarget>("mavros/setpoint_raw/local", 10);
    att_pub = this->create_publisher<geometry_msgs::msg::TwistStamped>("mavros/setpoint_attitude/cmd_vel", 10);
    actuator_control_pub = this->create_publisher<mavros_msgs::msg::ActuatorControl>( "/mavros/actuator_control", 10);
}

void OffboardMavros::initializeSubscribers(void) {
    state_sub = create_subscription<mavros_msgs::msg::State>(
            "mavros/state", 10, std::bind(&OffboardMavros::state_call_back_, this, std::placeholders::_1));

    subscription = this->create_subscription<std_msgs::msg::String>("/chatter", 10,
            std::bind( &OffboardMavros::chatterCallback, this, std::placeholders::_1
                ));
    auto default_qos = rclcpp::QoS(rclcpp::SystemDefaultsQoS());
    current_pos_sub = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/local_position/pose", default_qos,
            std::bind(&OffboardMavros::currentPositionCallback, this, std::placeholders::_1
                ));
}

void    OffboardMavros::initializeClients(void) {
    set_mode_client = create_client<mavros_msgs::srv::SetMode>("/mavros/set_mode");
    arming_client = create_client<mavros_msgs::srv::CommandBool>("/mavros/cmd/arming");
    takeoff_client = create_client<mavros_msgs::srv::CommandTOL>("/mavros/cmd/takeoff");
    landing_client = create_client<mavros_msgs::srv::CommandTOL>("/mavros/cmd/land");
    location_client = this->create_client<mavros_msgs::srv::CommandLong>("/mavros/cmd/command");
    transition_client = this->create_client<mavros_msgs::srv::CommandVtolTransition>("/mavros/cmd/vtol_transition");
}

void OffboardMavros::initializeTimers(const int rate_hz) {
    const int rate_ms = 1000 / rate_hz;
    timer = this->create_wall_timer(
            std::chrono::milliseconds(rate_ms),
            std::bind(&OffboardMavros::publish, this));
}

void OffboardMavros::state_call_back_(const mavros_msgs::msg::State::SharedPtr msg) {
    fcu_state = *msg;

    DEBUG::msg("\n[DEBUG] ", "-----------------");
    DEBUG::print("Mode : ", msg->mode, CYAN);
    DEBUG::printBool("Arming : ", msg->armed, RED);
    DEBUG::printBinary("Command flag : ", _cmd_flag, YELLOW);
    DEBUG::print("System status : ", fcu_state.system_status, BLUE);
    DEBUG::msg("[DEBUG] ", "-----------------\n");

    // if ((statusFlag == vtol::LAND) && is_real_arming_status_() && isFiveSecondsPassed()) {
    // if ((statusFlag == vol::TAKEOFF) && is_fcu_arming_status_() && isFiveSecondsPassed()) {
    // }
    // TODO: status_XXX_() 함수를 만들어서 사용


    // TODO: 생성자에서 초기화
    if (OffboardMavros::_cmd_flag == vtol::INIT) {
        if (fcu_state.armed == true) {
            updateLandingStatus();
        } else {
            OffboardMavros::_cmd_flag = vtol::READY;
        }
    }
    if (OffboardMavros::_cmd_flag == vtol::READY) {

        if (fcu_state.mode != vtol::FCU_HOLD) {
            update_disarming_status_();
            update_hold_mode_();
            update_custom_mode_(vtol::FCU_HOLD, &OffboardMavros::hold_response_callback_);
        }
    }
    if (OffboardMavros::_cmd_flag == vtol::ARMED) {
        DEBUG::print("", ">> ARMED <<", BOLDGREEN);
        if (fcu_state.armed != true) {
            update_arming_status_();
        }
    }

    if (OffboardMavros::_cmd_flag == vtol::FLY) {
        if (fcu_state.mode != vtol::FCU_HOLD) {
            update_hold_mode_();
        }
        std::cout << "Flying..." << std::endl;
    }
    if (OffboardMavros::_cmd_flag == vtol::TAKEOFF) {
        DEBUG::print("", ">> Take Off <<", BOLDGREEN);
        // 순서 중요
        if (fcu_state.mode != vtol::FCU_TAKEOFF && fcu_state.armed == true) {
            update_takeoff_status_();
        } else if (fcu_state.mode == vtol::FCU_TAKEOFF && fcu_state.armed == false) {
            update_arming_status_();
        }
    }

    if (OffboardMavros::_cmd_flag == vtol::START) {
        if (fcu_state.mode == vtol::FCU_HOLD) {
            update_offboard_mode_();
        }
    }

    if (OffboardMavros::_cmd_flag == vtol::TO_FIXED) {
        update_transition_fixed_status_();
    }

    if (OffboardMavros::_cmd_flag == vtol::TO_QUAD) {
        update_transition_quad_status_();
    }

    if (OffboardMavros::_cmd_flag == vtol::LAND) {
        if (fcu_state.mode != vtol::FCU_LAND && fcu_state.armed == true) {
            updateLandingStatus();
        } else if (fcu_state.mode == vtol::FCU_HOLD) {
            std::cout << "Landing success" << std::endl;
            OffboardMavros::_cmd_flag = vtol::READY;
        }
    } // if (statusFlag == vtol::READY) {
      //     update_takeoff_status_();
      // }
      // if (statusFlag == vtol::START && !is_real_offboard_mode_()) {
      //     update_custom_mode_("OFFBOARD");
      // }
      // if  ((statusFlag == vtol::ARMED) && is_real_disarming_status_()) {
      //     update_arming_status_(true);
      // } else if ((statusFlag == vtol::READY) && is_real_arming_status_()) {
      //     update_arming_status_(false);
      // }
      // if (is_state_mode_hold_() && is_real_arming_status_()) {
      //     update_custom_mode_("AUTO.LOITER");
      // // update_arming_status_();
      // }
      // if (statusFlag == vtol::READY) {
      //     return ;
      // }
      // updateDisarmingStatus();
      // update_takeoff_status_();
      // updateLandingStatus();
      // if ((statusFlag & vtol::BIT_ARMED) && is_state_disarming()) {
      //     statusFlag = vtol::READY;
      // }
}

