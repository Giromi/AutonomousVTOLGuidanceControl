#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/offboard/offboard.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <future>
#include <iostream>
#include <chrono>
#include <thread>

using namespace mavsdk;
using std::chrono::seconds;
using std::this_thread::sleep_for;

class MinimalSubscriber : public rclcpp::Node {
public:
    MinimalSubscriber() : Node("minimal_subscriber"), current_position{0.0, 0.0, -2.5} {
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "topic", 10, std::bind(&MinimalSubscriber::listener_callback, this, std::placeholders::_1));

        // Drone initialization
        std::string connection_url = "udp://:14540";
        mavsdk_.add_any_connection(connection_url);

        // Wait for the system to connect via heartbeat
        while (!mavsdk_.is_connected()) {
            std::cout << "Waiting for drone to connect..." << std::endl;
            sleep_for(seconds(1));
        }

        std::cout << "Drone discovered!" << std::endl;
        system_ = std::make_shared<System>(mavsdk_.system());

        // Set up plugins
        action_ = std::make_shared<Action>(system_);
        offboard_ = std::make_shared<Offboard>(system_);
        telemetry_ = std::make_shared<Telemetry>(system_);

        // Initialization routine
        init_drone();
    }

private:
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    Mavsdk mavsdk_;
    std::shared_ptr<System> system_;
    std::shared_ptr<Action> action_;
    std::shared_ptr<Offboard> offboard_;
    std::shared_ptr<Telemetry> telemetry_;

    struct Position {
        double north;
        double east;
        double down;
    } current_position;

    void listener_callback(const std_msgs::msg::String::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());
        
        if (msg->data == "w") {
            drone_move(5.0, 0.0, 0.0);
        } else if (msg->data == "s") {
            drone_move(-5.0, 0.0, 0.0);
        } else if (msg->data == "a") {
            drone_move(0.0, -5.0, 0.0);
        } else if (msg->data == "d") {
            drone_move(0.0, 5.0, 0.0);
        }
    }

    void drone_move(double north, double east, double down) {
        auto target_position = Offboard::PositionNedYaw { 
            float(current_position.north + north), 
            float(current_position.east + east), 
            float(current_position.down + down), 
            0.0 
        };

        offboard_->set_position_ned(target_position);

        current_position.north += north;
        current_position.east += east;
        current_position.down += down;
    }

    void init_drone() {
        // Check if drone is ready to arm
        auto is_ready = telemetry_->health_all_ok();
        if (!is_ready) {
            std::cout << "Drone is not ready to arm!" << std::endl;
            return;
        }

        // Arm the drone
        auto arm_result = action_->arm();
        if (arm_result != Action::Result::Success) {
            std::cerr << "Failed to arm: " << arm_result << std::endl;
            return;
        }

        // Take off
        auto takeoff_result = action_->takeoff();
        if (takeoff_result != Action::Result::Success) {
            std::cerr << "Failed to take off: " << takeoff_result << std::endl;
            return;
        }

        // Wait for takeoff to complete
        sleep_for(seconds(10));

        // Start offboard mode
        auto offboard_result = offboard_->start();
        if (offboard_result != Offboard::Result::Success) {
            std::cerr << "Failed to start offboard mode" << std::endl;
            return;
        }

        std::cout << "Offboard mode started" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto minimal_subscriber = std::make_shared<MinimalSubscriber>();
    rclcpp::spin(minimal_subscriber);
    rclcpp::shutdown();
    return 0;
}

