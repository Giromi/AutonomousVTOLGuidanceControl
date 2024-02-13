/****************************************************************************
 *
 * Copyright 2020 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @brief Offboard control example
 * @file offboard_control.cpp
 * @addtogroup examples
 * @author Mickey Cowden <info@cowden.tech>
 * @author Nuno Marques <nuno.marques@dronesolutions.io>
 */

#include <px4_msgs/msg/offboard_control_mode.hpp>
#include <px4_msgs/msg/trajectory_setpoint.hpp>
#include <px4_msgs/msg/vehicle_command.hpp>
#include <px4_msgs/msg/vehicle_control_mode.hpp>
#include <px4_msgs/msg/vehicle_local_position.hpp>
#include <rclcpp/rclcpp.hpp>
#include <stdint.h>
#include <array>
#include <queue>

#include <chrono>
#include <iostream>

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace px4_msgs::msg;

enum e_coordinate {NORTH, EAST, DOWN, YAW};

class OffboardControl : public rclcpp::Node
{
public:
	OffboardControl() : Node("offboard_control")
	{

		offboard_control_mode_publisher_ = this->create_publisher<OffboardControlMode>("/fmu/in/offboard_control_mode", 10);
		trajectory_setpoint_publisher_ = this->create_publisher<TrajectorySetpoint>("/fmu/in/trajectory_setpoint", 10);
		vehicle_command_publisher_ = this->create_publisher<VehicleCommand>("/fmu/in/vehicle_command", 10);

		rmw_qos_profile_t qos_profile = rmw_qos_profile_sensor_data;
		auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, 5), qos_profile);
		vehicle_local_position_subscription_ = this->create_subscription<px4_msgs::msg::VehicleLocalPosition>("/fmu/out/vehicle_local_position", qos, [this](const px4_msgs::msg::VehicleLocalPosition::UniquePtr msg) {
                local_position_ = {msg->x, msg->y, msg->z, msg->heading};
			std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                        << "RECEIVED SENSOR COMBINED DATA"   << std::endl
                        << "============================="   << std::endl
                        << "local_position_x  : " << local_position_[NORTH] << std::endl
                        << "local_position_y  : " << local_position_[EAST] << std::endl
                        << "local_position_z  : " << local_position_[DOWN] << std::endl
                        << "local_position_yaw: " << local_position_[YAW] << std::endl
                        << "============================="   << std::endl
                        << "current_target x, y, z, yaw: " 
                                        << way_points_.front()[NORTH] << ", "
                                        << way_points_.front()[EAST] << ", "
                                        << way_points_.front()[DOWN] << ", "
                                        << way_points_.front()[YAW] << std::endl
                        << "left_way_points: " << way_points_.size() << std::endl
                        << "============================="   << std::endl;

                });

		offboard_setpoint_counter_ = 0;

		auto timer_callback = [this]() -> void {

			if (offboard_setpoint_counter_ == 10) {
				// Change to Offboard mode after 10 setpoints
				this->publish_vehicle_command(VehicleCommand::VEHICLE_CMD_DO_SET_MODE, 1, 6);

				// Arm the vehicle
				this->arm();
			}

			// offboard_control_mode needs to be paired with trajectory_setpoint
			publish_offboard_control_mode();
			publish_trajectory_setpoint();

			// stop the counter after reaching 11
			if (offboard_setpoint_counter_ < 11) {
				offboard_setpoint_counter_++;
			}
		};
		timer_ = this->create_wall_timer(100ms, timer_callback);
	}

	void arm();
	void disarm();

    static void set_way_point(std::array<float, 4> way_point);

private:
    static std::queue<std::array<float, 4>> way_points_;
	rclcpp::TimerBase::SharedPtr timer_;

	rclcpp::Publisher<OffboardControlMode>::SharedPtr offboard_control_mode_publisher_;
	rclcpp::Publisher<TrajectorySetpoint>::SharedPtr trajectory_setpoint_publisher_;
	rclcpp::Publisher<VehicleCommand>::SharedPtr vehicle_command_publisher_;
	rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>::SharedPtr vehicle_local_position_subscription_;

    std::array<float, 4> local_position_;

	std::atomic<uint64_t> timestamp_;   //!< common synced timestamped

	uint64_t offboard_setpoint_counter_;   //!< counter for the number of setpoints sent

	void publish_offboard_control_mode();
	void publish_trajectory_setpoint();
	void publish_vehicle_command(uint16_t command, float param1 = 0.0, float param2 = 0.0);
};

std::queue<std::array<float, 4>> OffboardControl::way_points_;

void OffboardControl::set_way_point(std::array<float, 4> way_point) {
    OffboardControl::way_points_.push(way_point); 
}

/**
 * @brief Send a command to Arm the vehicle
 */
void OffboardControl::arm()
{
	publish_vehicle_command(VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 1.0);

	RCLCPP_INFO(this->get_logger(), "Arm command send");
}

/**
 * @brief Send a command to Disarm the vehicle
 */
void OffboardControl::disarm()
{
	publish_vehicle_command(VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 0.0);

	RCLCPP_INFO(this->get_logger(), "Disarm command send");
}

/**
 * @brief Publish the offboard control mode.
 *        For this example, only position and altitude controls are active.
 */
void OffboardControl::publish_offboard_control_mode()
{
	OffboardControlMode msg{};
	msg.position = true;
	msg.velocity = false;
	msg.acceleration = false;
	msg.attitude = false;
	msg.body_rate = false;
	msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
	offboard_control_mode_publisher_->publish(msg);
}

/**
 * @brief Publish a trajectory setpoint
 *        For this example, it sends a trajectory setpoint to make the
 *        vehicle hover at 5 meters with a yaw angle of 180 degrees.
 */
void OffboardControl::publish_trajectory_setpoint()
{
	TrajectorySetpoint msg{};
	msg.position = {way_points_.front()[NORTH], way_points_.front()[EAST], way_points_.front()[DOWN]};
    msg.yaw = way_points_.front()[YAW] ? way_points_.front()[YAW]
                : atan2(way_points_.front()[EAST] - local_position_[EAST], 
                        way_points_.front()[NORTH] - local_position_[NORTH]); // -pi ~ pi
	msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
	trajectory_setpoint_publisher_->publish(msg);
    if (way_points_.size() == 1) {
        return ;
    }
    if (abs(local_position_[NORTH] - way_points_.front()[NORTH]) < 1.0 &&
        abs(local_position_[EAST] - way_points_.front()[EAST]) < 1.0 &&
        abs(local_position_[DOWN] - way_points_.front()[DOWN]) < 1.0) {
        std::cout << "way point reached" << std::endl;
        way_points_.pop();
    }
}

        // abs(local_position_[YAW] - way_points_.front()[YAW]) < 1.0) {
/**
 * @brief Publish vehicle commands
 * @param command   Command code (matches VehicleCommand and MAVLink MAV_CMD codes)
 * @param param1    Command parameter 1
 * @param param2    Command parameter 2
 */
void OffboardControl::publish_vehicle_command(uint16_t command, float param1, float param2)
{
	VehicleCommand msg{};
	msg.param1 = param1;
	msg.param2 = param2;
	msg.command = command;
	msg.target_system = 1;
	msg.target_component = 1;
	msg.source_system = 1;
	msg.source_component = 1;
	msg.from_external = true;
	msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
	vehicle_command_publisher_->publish(msg);
}

static void test_square(void) {
    OffboardControl::set_way_point({0.0, 0.0, -5.0, -3.14});

    OffboardControl::set_way_point({100.0, .0, -10.0, 0});
    OffboardControl::set_way_point({100.0, 100.0, -15.0, 0});
    OffboardControl::set_way_point({0.0, 100.0, -5.0, 0});
    OffboardControl::set_way_point({0.0, 0.0, -5.0, 0});

    OffboardControl::set_way_point({0.0, 0.0, -5.0, -3.14});

}

static void test_star(void) {
    OffboardControl::set_way_point({0.0, 0.0, -5.0, -3.14});

    OffboardControl::set_way_point({100.0, 0.0, -5.0, 0});
    OffboardControl::set_way_point({40.45, 29.39, -5.0, 0});
    OffboardControl::set_way_point({30.9, 95.11, -5.0, 0});
    OffboardControl::set_way_point({-15.45, 47.55, -5.0, 0});
    OffboardControl::set_way_point({-80.9, 58.78, -5.0, 0});
    OffboardControl::set_way_point({-50.0, 0.0, -5.0, 0});
    OffboardControl::set_way_point({-80.9, -58.78, -5.0, 0});
    OffboardControl::set_way_point({-15.45, -47.55, -5.0, 0});
    OffboardControl::set_way_point({30.9, -95.11, -5.0, 0});
    OffboardControl::set_way_point({40.45, -29.39, -5.0, 0});
    OffboardControl::set_way_point({100.0, 0.0, -5.0, 0});
    
    OffboardControl::set_way_point({0.0, 0.0, -5.0, -3.14});
}

int main(int argc, char *argv[])
{


	std::cout << "Starting offboard control node..." << std::endl;
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    test_star();

	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<OffboardControl>());

	rclcpp::shutdown();
	return 0;
}
