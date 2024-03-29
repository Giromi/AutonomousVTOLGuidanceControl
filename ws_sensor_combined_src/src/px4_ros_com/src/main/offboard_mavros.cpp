#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <mavros_msgs/srv/command_bool.hpp>
#include <mavros_msgs/srv/set_mode.hpp>
#include <mavros_msgs/msg/state.hpp>
#include <mavros_msgs/msg/actuator_control.hpp>
#include <mavros_msgs/msg/override_rc_in.hpp>
#include <mavros_msgs/srv/command_tol.hpp>
#include <mavros_msgs/srv/command_long.hpp>
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
        actuator_control_pub_ = this->create_publisher<mavros_msgs::msg::ActuatorControl>( "/mavros/actuator_control", 10);
    }

    void initializeSubscribers(void) {
        state_sub_ = create_subscription<mavros_msgs::msg::State>(
                "mavros/state", 10, std::bind(&OffboardMavros::state_call_back_, this, std::placeholders::_1));

        subscription_ = this->create_subscription<std_msgs::msg::String>("/chatter", 10,
                std::bind( &OffboardMavros::chatterCallback, this, std::placeholders::_1
        ));
        auto default_qos = rclcpp::QoS(rclcpp::SystemDefaultsQoS());
        current_pos_sub_ = create_subscription<geometry_msgs::msg::PoseStamped>("/mavros/current_position_sub", default_qos,
        std::bind(&OffboardMavros::currentpositionCallback, this, std::placeholders::_1
        ));
    }

    void initializeClients(void) {
        set_mode_client_ = create_client<mavros_msgs::srv::SetMode>("/mavros/set_mode");
        arming_client_ = create_client<mavros_msgs::srv::CommandBool>("/mavros/cmd/arming");
        takeoff_client_ = create_client<mavros_msgs::srv::CommandTOL>("/mavros/cmd/takeoff");
        landing_client_ = create_client<mavros_msgs::srv::CommandTOL>("/mavros/cmd/land");
        location_client_ = this->create_client<mavros_msgs::srv::CommandLong>("/mavros/cmd/command");
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


    /* -- Publish Functions -- */

    void publish(void) {
        if (cmdFlag_ != vtol::START) {
            return ;
        }
        std::cout << "Publishing..." << std::endl;
        publishPose();
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


    /* -- Update Functions -- */

    void update_arming_status_(void) {
        request_arming_status_(true, &OffboardMavros::arming_response_callback);
    }

    void update_disarming_status_(void) {
        request_arming_status_(false, &OffboardMavros::disarming_response_callback);
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

        size_t i = 0, j = 0;

        for (; i < OffboardMavros::action_string_array_.size() && msg->data != OffboardMavros::action_string_array_[i]; ++i);
        
        if (i != OffboardMavros::action_string_array_.size()) {
            OffboardMavros::action_func_[i]();
            OffboardMavros::print_reference_input();
            return ;
        }

        for (; j < OffboardMavros::status_string_array_.size() && msg->data != OffboardMavros::status_string_array_[j]; ++j)

        if (j != OffboardMavros::status_string_array_.size()) {
            OffboardMavros::status_func_[j]();
            return ;
        }

        std::cout << "Invalid input" << std::endl;
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


    void currentpositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        current_position_ = {msg->pose.position.x, msg->pose.position.y, msg->pose.position.z};

        if (cmdFlag_ == vtol::TAKEOFF) {
            if (current_position_[vtol::UP] > 0.5) {
                cmdFlag_ = vtol::FLY;
            }
        }
    }


    /* -- Static Functions -- */


    /* -- Action Functions -- */
    
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

    static void action_return_home_(void) {
        //TODO make threshold
        local_position_[vtol::NORTH] = 0.0;
        local_position_[vtol::EAST] = 0.0;
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
        if (OffboardMavros::cmdFlag_ == vtol::FLY) {
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


    /* -- Action Functions -- */

    static void status_ready_(void) {
        if (OffboardMavros::cmdFlag_ != vtol::READY) {
            return ;
        }

        if (fcuState_.mode != vtol::FCU_HOLD) {
            OffboardMavros::cmdFlag_ = vtol::READY;
            update_custom_mode_("AUTO.LOITER", &OffboardMavros::hold_response_callback_);
            update_disarming_status_();
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
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    current_pos_sub_;
    rclcpp::Publisher<mavros_msgs::msg::ActuatorControl>::SharedPtr     actuator_control_pub_;

    rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr            arming_client_;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr             takeoff_client_;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr             landing_client_;
    rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr                set_mode_client_;
    rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedPtr            location_client_;

    rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr            state_sub_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr              subscription_;
    rclcpp::TimerBase::SharedPtr                                        timer_;
    mavros_msgs::msg::State                                             fcuState_;
    rclcpp::Time                                                        last_request_{0, 0, RCL_ROS_TIME};

    static unsigned char                                                       cmdFlag_;

    //TODO: static 지워서 멤버변수로 변경
    static std::array<float, 3>		        local_position_;
    static std::array<double, 3>		    current_position_;
    static const std::string				arrow_string_;
    static float                            offset_;

    static const std::array<std::string, vtol::ACTION_SIZE>        action_string_array_;
    static const std::array<std::string, vtol::STATUS_SIZE>		   status_string_array_;
    static void                                   (*action_func_[])(void);
    static void                                   (*status_func_[])(void);


};


unsigned char                           OffboardMavros::cmdFlag_ = vtol::INIT;
std::array<float, 3>		            OffboardMavros::local_position_{vtol::INIT_NORTH, vtol::INIT_EAST, vtol::INIT_UP};
std::array<double, 3>		            OffboardMavros::current_position_{};
const std::array<std::string, vtol::ACTION_SIZE>		OffboardMavros::action_string_array_ = { 
    "8", "6", "↓", "4", "2", "↑", "h", "a", "d", "t", "l", "s", "0"
};
const std::array<std::string, vtol::STATUS_SIZE>		OffboardMavros::status_string_array_ = { 
    "r"
};

void (*OffboardMavros::action_func_[])(void) = {
    &OffboardMavros::action_go_north_,
    &OffboardMavros::action_go_east_,
    &OffboardMavros::action_go_down_,
    &OffboardMavros::action_go_west_,
    &OffboardMavros::action_go_south_,
    &OffboardMavros::action_go_up_,
    &OffboardMavros::action_return_home_,
    &OffboardMavros::action_arming_,
    &OffboardMavros::action_disarming_,
    &OffboardMavros::action_takeoff_,
    &OffboardMavros::action_landing_,
    &OffboardMavros::action_start_,
    &OffboardMavros::action_init_
};

void (*OffboardMavros::status_func_[])(void) = {
    &OffboardMavros::status_ready_

}

float                    OffboardMavros::offset_ = 0.5f;

int main(int argc, char* argv[]) {



    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OffboardMavros>());
    rclcpp::shutdown();
    return 0;
}


