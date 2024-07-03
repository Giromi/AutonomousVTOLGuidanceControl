#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <mavros_msgs/srv/command_bool.hpp>
#include <mavros_msgs/srv/set_mode.hpp>
#include <mavros_msgs/msg/state.hpp>
#include <mavros_msgs/msg/actuator_control.hpp>
#include <mavros_msgs/msg/override_rc_in.hpp>

using namespace std::placeholders;

class OffbNode : public rclcpp::Node {
public:
    OffbNode() : Node("offb_node"), rate(20) {
        initializeSubscribers();
        initializePublishers();
        initializeClients();
    }

    void start() {
        waitForFCUConnection();
        sendInitialSetpoints();
        run();
    }

private:
    void run() {
        rclcpp::Rate loop_rate(100);
        while (rclcpp::ok()) {
            updateOffboardMode();
            updateArmingStatus();
            publishPose();
            publishActuatorControls(); // Call the actuator control publish method
            // _publishOverrideRCIn(); // Call the RC Override publish method
            rclcpp::spin_some(shared_from_this());
            loop_rate.sleep();
        }
    }

    void initializeSubscribers() {
        _state_sub = create_subscription<mavros_msgs::msg::State>(
                "mavros/state", 10, std::bind(&OffbNode::stateCallback, this, _1));
    }

    void initializePublishers() {
        _local_pos_pub = create_publisher<geometry_msgs::msg::PoseStamped>("/mavros/setpoint_position/local", 10);
        _actuator_control_pub = this->create_publisher<mavros_msgs::msg::ActuatorControl>( "/mavros/actuator_control", 10);
        _override_rcin_pub = this->create_publisher<mavros_msgs::msg::OverrideRCIn>("/mavros/rc/override", 10);
    }

    void initializeClients() {
        _arming_client = create_client<mavros_msgs::srv::CommandBool>("mavros/cmd/arming");
        _set_mode_client = create_client<mavros_msgs::srv::SetMode>("mavros/set_mode");
    }

    void waitForFCUConnection() {
        while (rclcpp::ok() && !current_state.connected) {
            rclcpp::spin_some(shared_from_this());
            rate.sleep();
        }
    }

    void sendInitialSetpoints() {
        for (int i = 100; rclcpp::ok() && i > 0; --i) {
            publishPose();
            rclcpp::spin_some(shared_from_this());
            rate.sleep();
        }
    }

    void updateOffboardMode() {
        if (current_state.mode != "OFFBOARD" && (this->now() - last_request).seconds() > 5.0) {
            auto request = std::make_shared<mavros_msgs::srv::SetMode::Request>();
            request->custom_mode = "OFFBOARD";
            _set_mode_client->async_send_request(request, std::bind(&OffbNode::offboardModeResponseCallback, this, _1));
            last_request = this->now();
        }
    }

    void updateArmingStatus() {
        if (!current_state.armed && (this->now() - last_request).seconds() > 5.0) {
            auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
            request->value = true;
            _arming_client->async_send_request(request, std::bind(&OffbNode::armingResponseCallback, this, _1));
            last_request = this->now();
        }
    }

    void publishPose() {
        geometry_msgs::msg::PoseStamped pose;
        pose.pose.position.x = 0;
        pose.pose.position.y = 0;
        pose.pose.position.z = 1;
        _local_pos_pub->publish(pose);
    }

    void publishActuatorControls() {
        mavros_msgs::msg::ActuatorControl actuator_control_msg;
        actuator_control_msg.group_mix = 6; // Adjust this according to your needs
        // Set your control values here. For example, to set all to 0.5:
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
        _actuator_control_pub->publish(actuator_control_msg);
    }

    void _publishOverrideRCIn() {
        mavros_msgs::msg::OverrideRCIn override_rcin_msg;
        override_rcin_msg.channels[0] = 1800;
        override_rcin_msg.channels[1] = 1800;
        override_rcin_msg.channels[2] = 1800;
        override_rcin_msg.channels[3] = 1800;
        override_rcin_msg.channels[4] = 1800;
        override_rcin_msg.channels[5] = 1800;
        override_rcin_msg.channels[6] = 1800;
        override_rcin_msg.channels[7] = 1800;
        _override_rcin_pub->publish(override_rcin_msg);
        RCLCPP_INFO(this->get_logger(), "Publishing RC Override message");
    }

    void stateCallback(const mavros_msgs::msg::State::SharedPtr msg) {
        current_state = *msg;
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

    rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr _state_sub;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr       _local_pos_pub;
    rclcpp::Publisher<mavros_msgs::msg::ActuatorControl>::SharedPtr     _actuator_control_pub;
    rclcpp::Publisher<mavros_msgs::msg::OverrideRCIn>::SharedPtr        _override_rcin_pub;
    rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr _arming_client;
    rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr _set_mode_client;
    rclcpp::Rate rate;
    mavros_msgs::msg::State current_state;
    rclcpp::Time last_request{0, 0, RCL_ROS_TIME};
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<OffbNode>();
    node->start(); // Now 'start' is called after the node is fully constructed.
    rclcpp::shutdown();
    return 0;
}
