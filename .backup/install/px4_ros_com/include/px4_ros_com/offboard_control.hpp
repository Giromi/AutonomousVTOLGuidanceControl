#ifndef OFFBOARD_CONTROL_HPP
# define OFFBOARD_CONTROL_HPP

# include <px4_msgs/msg/offboard_control_mode.hpp>
# include <px4_msgs/msg/trajectory_setpoint.hpp>
# include <px4_msgs/msg/vehicle_command.hpp>
# include <px4_msgs/msg/vehicle_control_mode.hpp>
# include <px4_msgs/msg/vehicle_local_position.hpp>
# include <rclcpp/rclcpp.hpp>
# include <stdint.h>
# include <array>
# include <queue>

# include <chrono>
# include <iostream>

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

#endif


