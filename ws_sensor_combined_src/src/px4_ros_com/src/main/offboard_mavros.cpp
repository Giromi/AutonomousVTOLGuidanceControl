#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <mavros_msgs/srv/command_bool.hpp>
#include <mavros_msgs/srv/set_mode.hpp>
#include <mavros_msgs/msg/state.hpp>
#include <mavros_msgs/msg/actuator_control.hpp>
#include <mavros_msgs/msg/override_rc_in.hpp>
#include <mavros_msgs/srv/command_tol.hpp>
#include <std_msgs/msg/string.hpp>
#include <array>
#include <limits>
#include "px4_ros_com/setting/coordinate.hpp"

class OffboardMavros : public rclcpp::Node {
public:
    OffboardMavros() : Node("offboard_mavros") {
        initializePublishers();
        initializeSubscribers();
        initializeClients();
        initializeArrays();
        initializeTimers(50);
    }

private:
    void initializePublishers(void) {
        local_pos_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("/mavros/setpoint_position/local", 10);
        actuator_control_pub_ = this->create_publisher<mavros_msgs::msg::ActuatorControl>( "/mavros/actuator_control", 10);
    }

    void initializeSubscribers(void) {
        state_sub_ = create_subscription<mavros_msgs::msg::State>(
                "mavros/state", 10, std::bind(&OffboardMavros::stateCallback, this, std::placeholders::_1));
        subscription_ = this->create_subscription<std_msgs::msg::String>( "chatter", 10,
                std::bind( &OffboardMavros::chatterCallback, this, std::placeholders::_1
        ));
    }

    void initializeClients(void) {
        arming_client_ = create_client<mavros_msgs::srv::CommandBool>("mavros/cmd/arming");
        landing_client_ = create_client<mavros_msgs::srv::CommandTOL>("mavros/cmd/land");
        set_mode_client_ = create_client<mavros_msgs::srv::SetMode>("mavros/set_mode");
    }

    void initializeArrays() {
        //TODO: local_position_ 초기화
    }

    void initializeTimers(const int rate_hz) {
        const int rate_ms = 1000 / rate_hz;
        timer_ = this->create_wall_timer(
                std::chrono::milliseconds(rate_ms),
                std::bind(&OffboardMavros::publish, this));
    }

    void stateCallback(const mavros_msgs::msg::State::SharedPtr msg) {
        current_state_ = *msg;
        updateOffboardMode();
        updateArmingStatus();
    }

    void updateOffboardMode() {
        if (current_state_.mode != "OFFBOARD" && (this->now() - last_request_).seconds() > 5.0) {
            auto request = std::make_shared<mavros_msgs::srv::SetMode::Request>();
            request->custom_mode = "OFFBOARD";
            set_mode_client_->async_send_request(request, std::bind(&OffboardMavros::offboardModeResponseCallback, this, std::placeholders::_1));
            last_request_ = this->now();
        }
    }

    bool is_state_disarming() {
        return (!current_state_.armed && (this->now() - last_request_).seconds() > 5.0);
    }

    bool is_state_arming() {
        return (current_state_.armed && (this->now() - last_request_).seconds() > 5.0);
    }

    void updateArmingStatus() {
        if  (is_state_disarming()) {
            auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
            request->value = true;
            arming_client_->async_send_request(request, std::bind(&OffboardMavros::armingResponseCallback, this, std::placeholders::_1));
            last_request_ = this->now();
        }
    }

    void updateDisarmingStatus() {
        if  (is_state_arming()) {
            auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
            request->value = false;
            arming_client_->async_send_request(request, std::bind(&OffboardMavros::armingResponseCallback, this, std::placeholders::_1));
            last_request_ = this->now();
        }
    }

    void callLanding_(void) {
        if (local_position_[UP] != -0.5f) {
            return ;
        }
        auto request = std::make_shared<mavros_msgs::srv::CommandTOL::Request>();
        request->altitude = 0;
        request->latitude = 0;
        request->longitude = 0;
        request->min_pitch = 0;
        request->yaw = 0;
        landing_client_->async_send_request(request,
                std::bind(&OffboardMavros::landingResponseCallback, this, std::placeholders::_1));
    }

    void offboardModeResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
        auto response = future.get();
        if (response->mode_sent) {
            RCLCPP_INFO(this->get_logger(), "Offboard enabled");
        } else {
            RCLCPP_ERROR(this->get_logger(), "Offboard failed");
        }
    }

    void armingResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future) {
        auto response = future.get();
        if (response->success) {
            RCLCPP_INFO(this->get_logger(), "Vehicle armed");
        } else {
            RCLCPP_ERROR(this->get_logger(), "Arming failed");
        }
    }

    void landingResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future) {
        auto response = future.get();
        if (response->success) {
            RCLCPP_INFO(this->get_logger(), "Land command sent successfully");
            updateDisarmingStatus();
        } else {
            RCLCPP_INFO(this->get_logger(), "Failed to send land command");
        }
    }

    void publish(void) {
        publishPose();
        callLanding_();
    }

    void publishPose() {
        geometry_msgs::msg::PoseStamped pose;
        pose.pose.position.x = local_position_[EAST];
        pose.pose.position.y = local_position_[NORTH];
        pose.pose.position.z = local_position_[UP];
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
        actuator_control_msg.controls[3] = 1.0f;
        actuator_control_msg.controls[4] = 1.0f;
        actuator_control_msg.controls[5] = 1.0f;
        actuator_control_msg.controls[6] = 1.0f;
        actuator_control_msg.controls[7] = 1.0f;
        RCLCPP_INFO(this->get_logger(), "publishing actuator controls");
        actuator_control_pub_->publish(actuator_control_msg);
    }

    void chatterCallback(const std_msgs::msg::String::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());

        size_t i = 0;
        for (; msg->data != OffboardMavros::action_string_array_[i]; ++i);

        if (i == OffboardMavros::action_string_array_.size()) {
            return ;
        }
        OffboardMavros::action_func_[i]();
    }

    static void action_go_north_(void) {
        //TODO make threshold
        local_position_[NORTH] += offset_;
        OffboardMavros::print_reference_input();
    }

    static void action_go_east_(void) {
        //TODO make threshold
        local_position_[EAST] += offset_;
        OffboardMavros::print_reference_input();
    }

    static void action_go_down_(void) {
        //TODO make threshold
        local_position_[UP] -= offset_;
        if (local_position_[UP] < 0.0){
            local_position_[UP] = 0.0;
        }
        OffboardMavros::print_reference_input();
    }

    static void action_go_south_(void) {
        //TODO make threshold
        local_position_[NORTH] -= offset_;
        OffboardMavros::print_reference_input();
    }

    static void action_go_west_(void) {
        //TODO make threshold
        local_position_[EAST] -= offset_;
        OffboardMavros::print_reference_input();
    }

    static void action_go_up_(void) {
        //TODO make threshold
        local_position_[UP] += offset_;
        OffboardMavros::print_reference_input();
    }

    static void action_landing_(void) {
        local_position_[UP] = -1.0f;
    }
    //TODO: 현재 위치를 확인해서 도달했을 disarm하는 함수를 만들어야함

    static void action_return_home(void) {
        //TODO make threshold
        local_position_[NORTH] = 0.0;
        local_position_[EAST] = 0.0;
        OffboardMavros::print_reference_input();
    }

    static void print_reference_input(void) {
        std::cout << "Position Command {"
                  << local_position_[NORTH] << ", "
                  << local_position_[EAST] << ", "
                  << local_position_[UP] << "} (North, East, Up)"
                  << std::endl;
    }

    //
    /* -- Members Variables -- */
    rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr            state_sub_;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr       local_pos_pub_;
    rclcpp::Publisher<mavros_msgs::msg::ActuatorControl>::SharedPtr     actuator_control_pub_;
    rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr            arming_client_;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr             landing_client_;
    rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr                set_mode_client_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr              subscription_;
    rclcpp::TimerBase::SharedPtr                                        timer_;
    mavros_msgs::msg::State                                             current_state_;
    rclcpp::Time                                                        last_request_{0, 0, RCL_ROS_TIME};


    //TODO: static 지워서 멤버변수로 변경
    static std::array<float, 3>		        local_position_;

    static const std::string				arrow_string_;
    static float                            offset_;

    static const std::array<std::string, 16>      action_string_array_;
    static void                                   (*action_func_[])(void);
};


std::array<float, 3>		            OffboardMavros::local_position_{};
const std::array<std::string, 16>		OffboardMavros::action_string_array_
    = { "8", "6", "↓", "4", "2", "↑", "h", "l" };
rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr             landing_client_ = nullptr;


void (*OffboardMavros::action_func_[])(void) = {
    &OffboardMavros::action_go_north_,
    &OffboardMavros::action_go_east_,
    &OffboardMavros::action_go_down_,
    &OffboardMavros::action_go_west_,
    &OffboardMavros::action_go_south_,
    &OffboardMavros::action_go_up_,
    &OffboardMavros::action_return_home,
    &OffboardMavros::action_landing_
};

float                    OffboardMavros::offset_ = 0.5f;

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OffboardMavros>());
    rclcpp::shutdown();
    return 0;
}
