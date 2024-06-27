#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <mavros_msgs/srv/command_bool.hpp>
#include <mavros_msgs/srv/set_mode.hpp>
#include <mavros_msgs/msg/state.hpp>
#include <mavros_msgs/msg/actuator_control.hpp>
#include <mavros_msgs/msg/override_rc_in.hpp>
#include <mavros_msgs/msg/position_target.hpp>
#include <mavros_msgs/srv/command_tol.hpp>
#include <mavros_msgs/srv/command_long.hpp>
#include <mavros_msgs/srv/command_vtol_transition.hpp>
#include <std_msgs/msg/string.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <array>
#include "px4_ros_com/convention.hpp"
#include "DEBUG.hpp"
#include <limits>
//#include <nav_msgs/msg/odometry.hpp>

class OffboardMavros : public rclcpp::Node {
public:
    OffboardMavros() : Node("offboard_mavros") {
        initializePublishers();
        initializeSubscribers();
        initializeClients();
        // initializeArrays();
        // Ready
        initializeTimers(50); 
    }

private:
    void initializePublishers(void) {
        local_pos_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("/mavros/setpoint_position/local", 10);

        local_vel_pub = this->create_publisher<geometry_msgs::msg::TwistStamped>("/mavros/setpoint_velocity/cmd_vel", 10);
        local_pub = this->create_publisher<mavros_msgs::msg::PositionTarget>("mavros/setpoint_raw/local", 10);
        att_pub = this->create_publisher<geometry_msgs::msg::TwistStamped>("mavros/setpoint_attitude/cmd_vel", 10);
        actuator_control_pub_ = this->create_publisher<mavros_msgs::msg::ActuatorControl>( "/mavros/actuator_control", 10);
    }

    void initializeSubscribers(void) {
        state_sub_ = create_subscription<mavros_msgs::msg::State>(
                "mavros/state", 10, std::bind(&OffboardMavros::state_call_back_, this, std::placeholders::_1));

        subscription_ = this->create_subscription<std_msgs::msg::String>("/chatter", 10,
                std::bind( &OffboardMavros::chatterCallback, this, std::placeholders::_1
        ));
        auto default_qos = rclcpp::QoS(rclcpp::SystemDefaultsQoS());
        current_pos_sub_ = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/local_position/pose", default_qos,
        std::bind(&OffboardMavros::currentpositionCallback, this, std::placeholders::_1
        ));
    }

    void initializeClients(void) {
        set_mode_client_ = create_client<mavros_msgs::srv::SetMode>("/mavros/set_mode");
        arming_client_ = create_client<mavros_msgs::srv::CommandBool>("/mavros/cmd/arming");
        takeoff_client_ = create_client<mavros_msgs::srv::CommandTOL>("/mavros/cmd/takeoff");
        landing_client_ = create_client<mavros_msgs::srv::CommandTOL>("/mavros/cmd/land");
        location_client_ = this->create_client<mavros_msgs::srv::CommandLong>("/mavros/cmd/command");
        transition_client_ = this->create_client<mavros_msgs::srv::CommandVtolTransition>("/mavros/cmd/vtol_transition");
    }


    void initializeTimers(const int rate_hz) {
        const int rate_ms = 1000 / rate_hz;
        timer_ = this->create_wall_timer(
                std::chrono::milliseconds(rate_ms),
                std::bind(&OffboardMavros::publish, this));
    }

    // void initializeArrays(void) {
    //     const std::string
    // }


    void state_call_back_(const mavros_msgs::msg::State::SharedPtr msg) {
        fcuState_ = *msg;

        DEBUG::msg("\n[DEBUG] ", "-----------------");
        DEBUG::print("Mode : ", msg->mode, CYAN);
        DEBUG::print_bool("Arming : ", msg->armed, RED);
        DEBUG::print_binary("Command flag : ", cmdFlag_, YELLOW);
        DEBUG::print("System status : ", fcuState_.system_status, BLUE);
        DEBUG::msg("[DEBUG] ", "-----------------\n");
        
        // if ((statusFlag == vtol::LAND) && is_real_arming_status_() && is_five_seconds_passed()) {
        // if ((statusFlag == vol::TAKEOFF) && is_fcu_arming_status_() && is_five_seconds_passed()) {
        // }
        // TODO: status_XXX_() 함수를 만들어서 사용


        // TODO: 생성자에서 초기화
        if (OffboardMavros::cmdFlag_ == vtol::INIT) {
            if (fcuState_.armed == true) {
                update_landing_status();
            } else {
                OffboardMavros::cmdFlag_ = vtol::READY;
            }
        }
        if (OffboardMavros::cmdFlag_ == vtol::READY) {
    
            if (fcuState_.mode != vtol::FCU_HOLD) {
                update_disarming_status_();
                update_hold_mode_();
                update_custom_mode_(vtol::FCU_HOLD, &OffboardMavros::hold_response_callback_);
            }
        }
        if (OffboardMavros::cmdFlag_ == vtol::ARMED) {
            DEBUG::print("", ">> ARMED <<", BOLDGREEN);
            if (fcuState_.armed != true) {
                update_arming_status_();
            }
        }

        if (OffboardMavros::cmdFlag_ == vtol::FLY) {
            if (fcuState_.mode != vtol::FCU_HOLD) {
                update_hold_mode_();
            }
            std::cout << "Flying..." << std::endl;
        }
        if (OffboardMavros::cmdFlag_ == vtol::TAKEOFF) {
            DEBUG::print("", ">> Take Off <<", BOLDGREEN);
            // 순서 중요
            if (fcuState_.mode != vtol::FCU_TAKEOFF && fcuState_.armed == true) {
                update_takeoff_status_();
            } else if (fcuState_.mode == vtol::FCU_TAKEOFF && fcuState_.armed == false) {
                update_arming_status_();
            }
        }

        if (OffboardMavros::cmdFlag_ == vtol::START) {
            if (fcuState_.mode == vtol::FCU_HOLD) {
                update_offboard_mode_();
            }
        }

        if (OffboardMavros::cmdFlag_ == vtol::TO_FIXED) {
            update_transition_fixed_status_();
        }

        if (OffboardMavros::cmdFlag_ == vtol::TO_QUAD) {
            update_transition_quad_status_();
        }

        if (OffboardMavros::cmdFlag_ == vtol::LAND) {
            if (fcuState_.mode != vtol::FCU_LAND && fcuState_.armed == true) {
                update_landing_status();
            } else if (fcuState_.mode == vtol::FCU_HOLD) {
                std::cout << "Landing success" << std::endl;
                OffboardMavros::cmdFlag_ = vtol::READY;
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
        // update_disarming_status();
        // update_takeoff_status_();
        // update_landing_status();
        // if ((statusFlag & vtol::BIT_ARMED) && is_state_disarming()) {
        //     statusFlag = vtol::READY;
        // }
    }

    // void status_init_(void) {
    // }

    void status_ready_(void) {
        if (OffboardMavros::cmdFlag_ != vtol::READY) {
            return ;
        }

        if (fcuState_.mode != vtol::FCU_HOLD) {
            OffboardMavros::cmdFlag_ = vtol::READY;
            update_custom_mode_("AUTO.LOITER", &OffboardMavros::hold_response_callback_);
            update_disarming_status_();
        }
    }

    /* -- Publish Functions -- */

    void publish(void) {
        if (cmdFlag_ != vtol::START) {
            return ;
        }
        std::cout << "Publishing..." << std::endl;
        // publishPose();
        // publish_velocity_();
        publish_local_();
        // publish_attitude_(); // orbit 안사라짐
    }

    void publishPose() {
        geometry_msgs::msg::PoseStamped pose;
        pose.pose.position.x = local_position_[vtol::EAST];
        pose.pose.position.y = local_position_[vtol::NORTH];
        pose.pose.position.z = local_position_[vtol::UP];
        local_pos_pub_->publish(pose);
    }

    void publishActuatorControls() {
        mavros_msgs::msg::ActuatorControl actuator_control_msg;
        actuator_control_msg.group_mix = 2;
        actuator_control_msg.header.stamp = this->now();
        actuator_control_msg.header.frame_id = "standard_vtol_0";
        actuator_control_msg.controls[0] = 1.0f;
        actuator_control_msg.controls[1] = 1.0f;
        actuator_control_msg.controls[2] = 1.0f;
        actuator_control_msg.controls[7] = 1.0f;
        RCLCPP_INFO(this->get_logger(), "publishing actuator controls");
        actuator_control_pub_->publish(actuator_control_msg);
    }


    void publish_velocity_(void) {
        geometry_msgs::msg::TwistStamped vel;
        vel.twist.linear.x = local_velocity_[0];
        vel.twist.linear.y = local_velocity_[1];
        vel.twist.linear.z = local_velocity_[2];
        vel.twist.angular.x = local_velocity_[3];
        vel.twist.angular.y = local_velocity_[4];
        vel.twist.angular.z = local_velocity_[5];
        local_vel_pub->publish(vel);
    }


    void publish_attitude_(void) {
        geometry_msgs::msg::TwistStamped att;
        att.twist.linear.x = local_velocity_[0];
        att.twist.linear.y = local_velocity_[1];
        att.twist.linear.z = local_velocity_[2];
        att.twist.angular.x = local_velocity_[3];
        att.twist.angular.y = local_velocity_[4];
        att.twist.angular.z = local_velocity_[5];
        att_pub->publish(att);
    }


    void publish_local_(void) {
        mavros_msgs::msg::PositionTarget local_msg;

        local_msg.header.stamp = this->now();
        local_msg.header.frame_id = "standard_vtol_0";
        local_msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_LOCAL_NED;
        local_msg.type_mask = //mavros_msgs::msg::PositionTarget::IGNORE_PX |
                              //mavros_msgs::msg::PositionTarget::IGNORE_PY |
                              //mavros_msgs::msg::PositionTarget::IGNORE_PZ |
                              mavros_msgs::msg::PositionTarget::IGNORE_AFX |
                              mavros_msgs::msg::PositionTarget::IGNORE_AFY |
                              mavros_msgs::msg::PositionTarget::IGNORE_AFZ;
                              //mavros_msgs::msg::PositionTarget::IGNORE_VZ;
                              //mavros_msgs::msg::PositionTarget::IGNORE_YAW_RATE;
        local_msg.velocity.x = local_velocity_[0];
        local_msg.velocity.y = local_velocity_[1];
        local_msg.velocity.z = local_velocity_[2];
        local_msg.yaw = local_velocity_[4];
        local_msg.yaw_rate = local_velocity_[5];
        local_pub->publish(local_msg);
    }


    /* -- Update Functions -- */
    void update_arming_status_(void) {
        request_arming_status_(true, &OffboardMavros::arming_response_callback);
    }

    void update_disarming_status_(void) {
        request_arming_status_(false, &OffboardMavros::disarming_response_callback);
    }

    void update_transition_fixed_status_(void) {
        // if (fcuState_.mode == vtol::MC) {
        request_transition_status_(vtol::FW, &OffboardMavros::transition_response_callback);
        // } else {
            // request_transition_status_(vtol::MC, &OffboardMavros::transition_response_callback);
    }

    void update_transition_quad_status_(void) {
        // if (fcuState_.mode == vtol::MC) {
        request_transition_status_(vtol::MC, &OffboardMavros::transition_response_callback);
        // } else {
            // request_transition_status_(vtol::MC, &OffboardMavros::transition_response_callback);
    }

    void request_transition_status_(const int input,
            void (OffboardMavros::*response_callback)
            (const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture)) {
        auto request = std::make_shared<mavros_msgs::srv::CommandVtolTransition::Request>();
        request->state = input;
        transition_client_->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
        last_request_ = this->now();
    }


    void request_arming_status_(const bool& input,
            void (OffboardMavros::*response_callback)
            (const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture)) {
        auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
        request->value = input;
        arming_client_->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
        last_request_ = this->now();
    }

    void update_takeoff_status_(void) {
        auto request = make_request_takeoff_land_message_(
                        vtol::GeographicCoordinate{vtol::INIT_UP, 0, 0, 0, 0});
        takeoff_client_->async_send_request(request,
                std::bind(&OffboardMavros::takeoff_response_callback_, this, std::placeholders::_1));
        last_request_ = this->now();
    }

    void update_landing_status(void) {
        auto request = make_request_takeoff_land_message_(
                                    vtol::GeographicCoordinate{0, 0, 0, 0, 0});
        landing_client_->async_send_request(request,
                std::bind(&OffboardMavros::land_response_callback_, this, std::placeholders::_1));
        last_request_ = this->now();
    }
    std::shared_ptr<mavros_msgs::srv::CommandTOL::Request>
        make_request_takeoff_land_message_(const vtol::GeographicCoordinate& input) {
        auto request = std::make_shared<mavros_msgs::srv::CommandTOL::Request>();
        request->altitude   = input.altitude;
        request->latitude   = input.latitude;
        request->longitude  = input.longitude;
        request->min_pitch  = input.min_pitch;
        request->yaw        = input.yaw;
        return request;
    }

    void update_location_(std::array<double, 3> input) {
        auto request = std::make_shared<mavros_msgs::srv::CommandLong::Request>();

        request->command = 16;
        request->param5 = input[vtol::NORTH]; // latitude
        request->param6 = input[vtol::EAST]; // longitude
        request->param7 = input[vtol::UP]; // altitude
        location_client_->async_send_request(request, 
                std::bind(&OffboardMavros::location_response_callback_, this, std::placeholders::_1));
        last_request_ = this->now();
    }

    void takeoff_response_callback_(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future) {
        const bool success = future.get()->success;
        const char* msg[] = {
            "Takeoff command sent successfully", 
            "Failed to send Takeoff command"
        };
        print_success_info_(success, msg);
    }

    void land_response_callback_(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future) {
        const bool success = future.get()->success;
        const char* msg[] = {
            "Land command sent successfully", 
            "Failed to send land command"
        };
        print_success_info_(success, msg);
    }
    // shared_future를 사용하는 이유는 비동기로 요청을 보내기 때문에 요청에 대한 응답을 받아야하기 때문이다.
    
    //return request
    //
    void location_response_callback_(const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future) {
        const char* msg[] = {
            "Location command sent successfully", 
            "Failed to send location command"
        };
        print_success_info_(future.get()->success, msg);
    }

    void update_hold_mode_(void) {
        update_custom_mode_("AUTO.LOITER", &OffboardMavros::hold_response_callback_);
    }

    void update_offboard_mode_(void) {
        update_custom_mode_("OFFBOARD", &OffboardMavros::offboard_response_callback_);
    }

    void update_custom_mode_(const std::string& input_mode,
            void (OffboardMavros::*response_callback)(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture)) {
        auto request = std::make_shared<mavros_msgs::srv::SetMode::Request>();
        request->custom_mode = input_mode;
        set_mode_client_->async_send_request(request, std::bind(response_callback, this, std::placeholders::_1));
        last_request_ = this->now();
    }

    void offboard_response_callback_(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
        const char* msg[] = {
            "Offboard mode sent successfully", 
            "Failed to send Offboard mode"
        };
        print_success_info_(future.get()->mode_sent, msg);
    }

    void hold_response_callback_(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
        const char* msg[] = {
            "Hold mode sent successfully", 
            "Failed to send Hold mode"
        };
        print_success_info_(future.get()->mode_sent, msg);
    }

    void chatterCallback(const std_msgs::msg::String::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());


        size_t i = 0;
        for (; i < OffboardMavros::action_string_array_.size() && msg->data != OffboardMavros::action_string_array_[i]; ++i);

        if (i == OffboardMavros::action_string_array_.size()) {
            std::cout << "Invalid input" << std::endl;
            return ;
        }
        OffboardMavros::action_func_[i]();
        OffboardMavros::print_reference_input();
    }




    void arming_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future) {
        const char* msg[] = {
            "Vehicle armed", 
            "Arming failed"
        };
        print_success_info_(future.get()->success, msg);
    }

    void disarming_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future) {
        const char* msg[] = {
            "Vehicle disarmed", 
            "Disarming failed"
        };
        print_success_info_(future.get()->success, msg);
    }

    void transition_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture future) {
        const char* msg[] = {
            "Transition success", 
            "Transition failed"
        };
        print_success_info_(future.get()->success, msg);
    }

    void print_success_info_(bool success, const char* msg[]) const {
        if (success) {
            RCLCPP_INFO(this->get_logger(), "%s", msg[vtol::SUCCESS]);
        } else {
            RCLCPP_INFO(this->get_logger(), "%s", msg[vtol::FAIL]);
        }
    }

    /* -- Is Functions -- */



    bool is_five_seconds_passed_() {
        return (this->now() - last_request_).seconds() > 5.0;
    }

    // bool is_fcu_offboard_mode_() {
    //     return (current_state_.mode == vtol::FCU_OFFBOARD);
    // }
    //
    // bool is_fcu_hold_mode_() {
    //     return (current_state_.mode == vtol::FCU_HOLD);
    // }
    //
    // bool is_fcu_takeoff_mode_() {
    //     return (current_state_.mode == vtol::FCU_TAKEOFF);
    // }
    //
    // bool is_fcu_takeoff_mode_() {
    //     return (current_state_.mode == vtol::FCU_TAKEOFF);
    // }
    //
    // bool is_fcu_disarming_status_() {
    //     return (!current_state_.armed);
    // }
    //
    // bool is_fcu_arming_status_() {
    //     return (current_state_.armed);
    // }

    /* -- Static Functions -- */
    void currentpositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        cur_position_ = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};

        if (cmdFlag_ == vtol::TAKEOFF) {
            if (cur_position_[vtol::UP] > vtol::INIT_UP - 1) {
                cmdFlag_ = vtol::FLY;
                prev_position_[vtol::NORTH] = cur_position_[vtol::NORTH];
                prev_position_[vtol::EAST] = cur_position_[vtol::EAST];
                DEBUG::print("Landing point North :", prev_position_[vtol::NORTH], BOLDYELLOW);
                DEBUG::print("Landing point East  :", prev_position_[vtol::EAST], BOLDYELLOW);
            }
        } else if (cmdFlag_ == vtol::TO_FIXED) {
            DEBUG::print("North :", cur_position_[vtol::NORTH], WHITE);
            DEBUG::print("East  :", cur_position_[vtol::EAST], WHITE);
            if (cur_position_[vtol::NORTH] > prev_position_[vtol::NORTH] + 1 
                || cur_position_[vtol::EAST] > prev_position_[vtol::EAST] + 1) {
                DEBUG::print("Transition success North :", cur_position_[vtol::NORTH], BOLDYELLOW);
                DEBUG::print("Transition success East  :", cur_position_[vtol::EAST], BOLDYELLOW);
                cmdFlag_ = vtol::FIXED;
            }
        } else if (cmdFlag_ == vtol::TO_QUAD) {
            if (cur_position_[vtol::NORTH] - prev_position_[vtol::NORTH] < 0.1
                && cur_position_[vtol::EAST] - prev_position_[vtol::EAST] < 0.1) {
                DEBUG::print("Transition success North :", cur_position_[vtol::NORTH], BOLDYELLOW);
                DEBUG::print("Transition success East  :", cur_position_[vtol::EAST], BOLDYELLOW);
                // cmdFlag_ &= ~vtol::BIT_TRANSITION;
                cmdFlag_ = vtol::QUAD;
            }
            prev_position_[vtol::NORTH] = cur_position_[vtol::NORTH];
            prev_position_[vtol::EAST] = cur_position_[vtol::EAST];
        }
    }




    static void action_go_north_(void) {
        //TODO make threshold
        local_position_[vtol::NORTH] += offset_;
    }

    static void action_go_east_(void) {
        //TODO make threshold
        local_position_[vtol::EAST] += offset_;
    }

    static void action_go_down_(void) {
        //TODO make threshold
        local_position_[vtol::UP] -= offset_;
    }

    static void action_go_south_(void) {
        //TODO make threshold
        local_position_[vtol::NORTH] -= offset_;
    }

    static void action_go_west_(void) {
        //TODO make threshold
        local_position_[vtol::EAST] -= offset_;
    }

    static void action_go_up_(void) {
        //TODO make threshold
        local_position_[vtol::UP] += offset_;
    }

    static void action_velocity_plus_x_(void) {
        local_velocity_[0] += offset_;
    }

    static void action_velocity_plus_y_(void) {
        local_velocity_[1] += offset_;
    }

    static void action_velocity_plus_z_(void) {
        local_velocity_[2] += offset_;
    }

    static void action_velocity_minus_x_(void) {
        local_velocity_[0] -= offset_;
    }

    static void action_velocity_minus_y_(void) {
        local_velocity_[1] -= offset_;
    }

    static void action_velocity_minus_z_(void) {
        local_velocity_[2] -= offset_;
    }

    static void action_velocity_plus_roll_(void) {
        local_velocity_[3] += offset_;
    }

    static void action_velocity_plus_pitch_(void) {
        local_velocity_[4] += offset_;
    }

    static void action_velocity_plus_yaw_(void) {
        local_velocity_[5] += offset_;
    }

    static void action_velocity_minus_roll_(void) {
        local_velocity_[3] -= offset_;
    }

    static void action_velocity_minus_pitch_(void) {
        local_velocity_[4] -= offset_;
    }

    static void action_velocity_minus_yaw_(void) {
        local_velocity_[5] -= offset_;
    }

    // static void action_velocity_minus_yaw_(void) {
    //     local_velocity_[5] -= offset_;
    // }

    static void action_return_home_(void) {
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

    static void action_arming_(void) {
        if (OffboardMavros::cmdFlag_ != vtol::READY) {
            std::cout << "Vehicle is NOT READY status" << std::endl;
            return ;
        }
        OffboardMavros::cmdFlag_ = vtol::ARMED;
        // } else if (!(statusFlag & vtol::BIT_READY)) {
        //     RCLCPP_INFO(this->get_logger(), "Vehicle is NOT OFFBOARD status");
        //     return true;
        // } else {
        //     RCLCPP_INFO(this->get_logger(), "Calling arming service ...");
        // }
    }

    static void action_disarming_(void) {
        if (OffboardMavros::cmdFlag_ != vtol::ARMED) {
            std::cout << "Vehicle is NOT ARMED status" << std::endl;
            return ;
        }
        OffboardMavros::cmdFlag_ = vtol::READY;
    }

    static void action_takeoff_(void) {
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

    static void action_landing_(void) {
        OffboardMavros::cmdFlag_ = vtol::LAND;
    }


    static void action_start_(void) {
        if (OffboardMavros::cmdFlag_ == vtol::QUAD || OffboardMavros::cmdFlag_ == vtol::FIXED) {
            OffboardMavros::cmdFlag_ = vtol::START;
        } else if (OffboardMavros::cmdFlag_ == vtol::START) {
            OffboardMavros::cmdFlag_ = vtol::FLY;
        }
    }

    static void action_hold_(void) {
        OffboardMavros::cmdFlag_ = vtol::FLY;
    }

    static void action_init_(void) {
        OffboardMavros::cmdFlag_ = vtol::INIT;
    }

    static void action_transition_(void) {

        if (OffboardMavros::cmdFlag_ == vtol::QUAD) {
            OffboardMavros::cmdFlag_ = vtol::TO_FIXED;
        } else if (OffboardMavros::cmdFlag_ == vtol::FIXED) {
            OffboardMavros::cmdFlag_ = vtol::TO_QUAD;
        }

    }

    /* -- Print Functions -- */

    static void print_reference_input(void) {
        std::cout << "Position Command {"
                  << local_position_[vtol::NORTH] << ", "
                  << local_position_[vtol::EAST] << ", "
                  << local_position_[vtol::UP] << "} (North, East, Up)"
                  << std::endl;
    }

    /* -- Members Variables -- */
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr       local_pos_pub_;
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr      local_vel_pub;
    rclcpp::Publisher<mavros_msgs::msg::PositionTarget>::SharedPtr      local_pub;
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr      att_pub;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    current_pos_sub_;
    rclcpp::Publisher<mavros_msgs::msg::ActuatorControl>::SharedPtr     actuator_control_pub_;

    rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr            arming_client_;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr             takeoff_client_;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr             landing_client_;
    rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr                set_mode_client_;
    rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedPtr            location_client_;
    rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedPtr  transition_client_;

    rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr            state_sub_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr              subscription_;
    rclcpp::TimerBase::SharedPtr                                        timer_;
    mavros_msgs::msg::State                                             fcuState_;
    rclcpp::Time                                                        last_request_{0, 0, RCL_ROS_TIME};

    // static const std::array<std::string, vtol::ACTION_SIZE>        action_string_array_;

    static unsigned char                                                       cmdFlag_;

    //TODO: static 지워서 멤버변수로 변경
    static std::array<double, 3>		        local_position_;
    static std::array<double, 6>		    local_velocity_;
    static std::array<double, 3>		    cur_position_;
    static std::array<double, 3>		    prev_position_;
    static const std::string				arrow_string_;
    static double                            offset_;

    static const std::array<std::string, vtol::ACTION_SIZE>        action_string_array_;
    static void                                   (*action_func_[])(void);


};


unsigned char                           OffboardMavros::cmdFlag_ = vtol::INIT;
std::array<double, 3>		            OffboardMavros::local_position_{vtol::INIT_NORTH, vtol::INIT_EAST, vtol::INIT_UP};
std::array<double, 6>		            OffboardMavros::local_velocity_{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};  
std::array<double, 3>		            OffboardMavros::cur_position_{};
std::array<double, 3>		            OffboardMavros::prev_position_{};

const std::array<std::string, vtol::ACTION_SIZE>	OffboardMavros::action_string_array_ = { 
    "2", "4", "6", "3", "5", "7", 
    "↑", "↓", "→", "←", "+", "-", 
    "h", "a", "d", "t", "l", "s", "0", "w",  
};

void (*OffboardMavros::action_func_[])(void) = {
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

double                    OffboardMavros::offset_ = 0.5;

int main(int argc, char* argv[]) {



    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OffboardMavros>());
    rclcpp::shutdown();
    return 0;
}


