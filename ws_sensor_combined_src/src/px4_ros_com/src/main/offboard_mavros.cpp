#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <mavros_msgs/srv/command_bool.hpp>
#include <mavros_msgs/srv/set_mode.hpp>
#include <mavros_msgs/msg/state.hpp>
#include <mavros_msgs/msg/actuator_control.hpp>
#include <mavros_msgs/msg/override_rc_in.hpp>
#include <std_msgs/msg/string.hpp>

class OffboardMavros : public rclcpp::Node {
public:
    OffboardMavros() : Node("offboard_mavros") {
        initializePublishern();
        initializeSubscribers();
        initializeClients();
        initializeTimers(50);
    }

private:
    void initializePublishern(void) {
        local_pos_pub = create_publisher<geometry_msgs::msg::PoseStamped>("/mavros/setpoint_position/local", 10);
        actuator_control_pub = this->create_publisher<mavros_msgs::msg::ActuatorControl>( "/mavros/actuator_control", 10);
    }

    void initializeSubscribers(void) {
        state_sub = create_subscription<mavros_msgs::msg::State>(
                "mavros/state", 10, std::bind(&OffboardMavros::stateCallback, this, std::placeholders::_1));
        _subscription = this->create_subscription<std_msgs::msg::String>( "chatter", 10,
                std::bind( &OffboardMavros::chatterCallback, this, std::placeholders::_1
        ));
    }

    void initializeClients(void) {
        arming_client = create_client<mavros_msgs::srv::CommandBool>("mavros/cmd/arming");
        set_mode_client = create_client<mavros_msgs::srv::SetMode>("mavros/set_mode");
    }

    void initializeTimers(const int rate_hz) {
        const int rate_ms = 1000 / rate_hz;
        timer = this->create_wall_timer(
                std::chrono::milliseconds(rate_ms),
                std::bind(&OffboardMavros::publishPose, this));
    }

    void stateCallback(const mavros_msgs::msg::State::SharedPtr msg) {
        current_state = *msg;
        updateOffboardMode();
        updateArmingStatus();
    }

    void updateOffboardMode() {
        if (current_state.mode != "OFFBOARD" && (this->now() - last_request).seconds() > 5.0) {
            auto request = std::make_shared<mavros_msgs::srv::SetMode::Request>();
            request->custom_mode = "OFFBOARD";
            set_mode_client->async_send_request(request, std::bind(&OffboardMavros::offboardModeResponseCallback, this, std::placeholders::_1));
            last_request = this->now();
        }
    }

    void updateArmingStatus() {
        if (!current_state.armed && (this->now() - last_request).seconds() > 5.0) {
            auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
            request->value = true;
            arming_client->async_send_request(request, std::bind(&OffboardMavros::armingResponseCallback, this, std::placeholders::_1));
            last_request = this->now();
        }
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

    void publishPose() {
        geometry_msgs::msg::PoseStamped pose;
        pose.pose.position.x = 0;
        pose.pose.position.y = 0;
        pose.pose.position.z = 2;
        local_pos_pub->publish(pose);
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
        actuator_control_pub->publish(actuator_control_msg);
    }


    // void chatterCallback(const std_msgs::msg::String::SharedPtr msg) {
    //     RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());
    //
    //     switch (msg->data.c_str()[0]) {
    //
    //         // what is arrow up key event
    //         //
    //         //
    //         // what is arrow down key event?
    //
    //         // what is arrow right key event
    //         // what is w key event
    //         //
    //         case 'w':
    //             break;
    //         case 's':
    //             break;
    //         default:
    //             break;
    //     }
    // }
    //
    /* -- Members Variables -- */
    rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr            state_sub;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr       local_pos_pub;
    rclcpp::Publisher<mavros_msgs::msg::ActuatorControl>::SharedPtr     actuator_control_pub;
    rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr            arming_client;
    rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr                set_mode_client;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr              _subscription;
    rclcpp::TimerBase::SharedPtr                                        timer;
    mavros_msgs::msg::State                                             current_state;
    rclcpp::Time                                                        last_request{0, 0, RCL_ROS_TIME};
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<OffboardMavros>());
    rclcpp::shutdown();
    return 0;
}
