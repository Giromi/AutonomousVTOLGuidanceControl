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

#include "px4_ros_com/OffboardControl.hpp"
#include "px4_ros_com/Dubins.hpp"

std::queue<WayPoint> OffboardControl::_way_points;
std::queue<DubinsPathPoint> OffboardControl::_dubins_path_points;

double OffboardControl::turning_radius = 10.0;
double OffboardControl::sampling_interval = 2.0;

OffboardControl::OffboardControl() : Node("offboard_control"), _pwm(800), _pwm_nomallize(-1.0) {
		offboard_control_mode_publisher_ = this->create_publisher<OffboardControlMode>("/fmu/in/offboard_control_mode", 10);
		trajectory_setpoint_publisher_ = this->create_publisher<TrajectorySetpoint>("/fmu/in/trajectory_setpoint", 10);
		vehicle_command_publisher_ = this->create_publisher<VehicleCommand>("/fmu/in/vehicle_command", 10);
        _publisher_arm = this->create_publisher<mavros_msgs::msg::ActuatorControl>( "/mavros/actuator_control", 10);

		rmw_qos_profile_t qos_profile = rmw_qos_profile_sensor_data;
		auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, 5), qos_profile);
		vehicle_local_position_subscription_ = this->create_subscription<px4_msgs::msg::VehicleLocalPosition>("/fmu/out/vehicle_local_position", qos, [this](const px4_msgs::msg::VehicleLocalPosition::UniquePtr msg) {
                _local_position = {msg->x, msg->y, msg->z, msg->heading};
                const int width = 10;
                std::cout << std::fixed << std::setprecision(2); // 소수점 이하 두 자리까지만 표시
                std::cout << "\n\n"
                          << "RECEIVED SENSOR COMBINED DATA\n"
                          << "===================================================\n"
                          << "  Local Position  |  Way Point\n"
                          << "===================================================\n"
                          << "North: " << std::setw(width) << _local_position[NORTH] << " ➔ " 
                              << std::setw(width) << _way_points.front().north << "\n"
                          << "East : " << std::setw(width) << _local_position[EAST]  << " ➔ " 
                              << std::setw(width) << _way_points.front().east << "\n"
                          << "Down : " << std::setw(width) << _local_position[UP]  << " ➔ " 
                              << std::setw(width) << _way_points.front().down << "\n"
                          << "Yaw  : " << std::setw(width) << _local_position[YAW]   << " | " 
                              << std::setw(width) << _way_points.front().yaw << "\n"
                          << "---------------------------------------------------\n"
                          << "(way point length) => " << sqrt(pow(_local_position[NORTH] - _way_points.front().north
                                , 2) + pow(_local_position[EAST] - _way_points.front().east, 2) 
                                + pow(_local_position[UP] - _way_points.front().down, 2)) << "\n"
                          << "(dubins path length) => " << sqrt(pow(_local_position[NORTH] - _dubins_path_points.front().north , 2) + pow(_local_position[EAST] - _dubins_path_points.front().east, 2)) << "\n"
                          << "===================================================\n"
                          << "Left way points         : " << _way_points.size() << "\n"
                          << "Left Dubins path points : " << _dubins_path_points.size() << "\n"
                          << "===================================================\n";
                });
        // _key_event_subscription = this->create_subscription<std_msgs::msg::String>(
        //     "chatter", 10, std::bind(
        //         &OffboardControl::chatterCallback, 
        //         this, 
        //         std::placeholders::_1
        // ));


		_offboard_setpoint_counter = 0;

		auto timer_callback = [this]() -> void {

			if (_offboard_setpoint_counter == 10) {
				// Change to Offboard mode after 10 setpoints
				this->publish_vehicle_command(VehicleCommand::VEHICLE_CMD_DO_SET_MODE, 1, 6);

				// Arm the vehicle
				this->arm();
			}

			// offboard_control_mode needs to be paired with trajectory_setpoint
			publish_offboard_control_mode();
			// publish_trajectory_setpoint();
            _publish_pwm_output_message();
			// stop the counter after reaching 11
			if (_offboard_setpoint_counter < 11) {
				_offboard_setpoint_counter++;
			}
		};
		timer_ = this->create_wall_timer(1000ms, timer_callback);
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
void OffboardControl::publish_trajectory_setpoint(void)
{
	TrajectorySetpoint msg{};
    std::array<float, 3> norm = {_way_points.front().north, _way_points.front().east, _way_points.front().down};
    if (_way_points.front().is_dubins_path 
            && !is_reach_way_point_with_norm(norm)) {
        make_dubins_trajectory_setpoint(msg);
    } else {
        make_general_trajectory_setpoint(msg);
    }
	trajectory_setpoint_publisher_->publish(msg);
}

void OffboardControl::make_general_trajectory_setpoint(TrajectorySetpoint& msg) {
	msg.position = {_way_points.front().north, _way_points.front().east, _way_points.front().down};
    msg.yaw = _way_points.front().yaw ? _way_points.front().yaw
                : atan2(_way_points.front().east - _local_position[EAST], 
                        _way_points.front().north - _local_position[NORTH]); // -pi ~ pi
	msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
    if (_way_points.size() == 1) {
        return ;
    }
    if (is_reach_way_point_with_norm(msg.position)) {
        std::cout << "way point reached" << std::endl;
        _way_points.pop();
    }
}

void OffboardControl::make_dubins_trajectory_setpoint(TrajectorySetpoint& msg) {
    if (_dubins_path_points.empty()) {
        std::cout << "dubins path planning" << std::endl;
        const std::array<double, 3> start = {_local_position[NORTH], _local_position[EAST], _local_position[YAW]};
        const std::array<double, 3> end = {_way_points.front().north, _way_points.front().east, _way_points.front().yaw};
        Dubins dubins(start, end, OffboardControl::turning_radius);
        dubins.shortest_path();
        dubins.path_sample_many(OffboardControl::sampling_interval, OffboardControl::set_dubins_path_point, &_local_position);
    }
	msg.position = {_dubins_path_points.front().north, _dubins_path_points.front().east, _dubins_path_points.front().down};
    msg.yaw = _dubins_path_points.front().yaw ? _dubins_path_points.front().yaw
                : atan2(_dubins_path_points.front().east - _local_position[EAST], 
                        _dubins_path_points.front().north - _local_position[NORTH]); // -pi ~ pi
	msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
    const std::array<float, 2> planar = {msg.position[NORTH], msg.position[EAST]};
    if (is_reach_way_point_with_norm(planar)) {
        std::cout << "way point reached" << std::endl;
        _dubins_path_points.pop();
    }
}

bool OffboardControl::is_reach_way_point_with_square(std::array<float, 3> target) {
    return (abs(_local_position[NORTH] - target[NORTH]) < 1.0 &&
        abs(_local_position[EAST] - target[EAST]) < 1.0 &&
        abs(_local_position[UP] - target[UP]) < 1.0);
}

bool OffboardControl::is_reach_way_point_with_norm(std::array<float, 3> target) {
    return sqrt(pow(_local_position[NORTH] - target[NORTH] , 2) +
                 pow(_local_position[EAST] - target[EAST], 2) +
                 pow(_local_position[UP] - target[UP], 2)) < 10.0;
}

bool OffboardControl::is_reach_way_point_with_norm(std::array<float, 2> target) {
    return sqrt(pow(_local_position[NORTH] - target[NORTH] , 2) +
                 pow(_local_position[EAST] - target[EAST], 2)) < 10.0;
}
        // abs(_local_position[YAW] - _way_points.front()[YAW]) < 1.0) {

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

// void OffboardControl::dubins_path_planning(float x, float y) {
//
// }

// void OffboardControl::set_way_point(std::array<float, 4> way_point) {
//     OffboardControl::_way_points.push(way_point); 
// }

void OffboardControl::set_way_point(WayPoint way_point) {
    OffboardControl::_way_points.push(way_point); 
}

int OffboardControl::set_dubins_path_point(double q[3], double x, void* user_data) {
    static_cast<void>(x); // for unused
    LocalPosition* local_position = static_cast<LocalPosition *>(user_data);  
    DubinsPathPoint dubins_path_point(q[0], q[1], (*local_position)[UP], q[2], x);
    OffboardControl::_dubins_path_points.push(dubins_path_point);
    return 0;
}

// void OffboardControl::chatterCallback(const std_msgs::msg::String::SharedPtr msg) {
//     RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());
// }

void OffboardControl::_publish_pwm_output_message(void) {
    _publish_arm_control_message();
    // _publish_disarm_control_message();
    // _publish_disarm_control_message2();
    // _publish_disarm_control_message_param();
}

void OffboardControl::_publish_arm_control_message(void) {
    auto actuator_control_msg = mavros_msgs::msg::ActuatorControl();
    actuator_control_msg.header.stamp = this->now();
    actuator_control_msg.header.frame_id = "camera_servo";
    actuator_control_msg.group_mix = 2;  // Use group 2 for AUX channels in PX4
    // actuator_control_msg->controls.resize(8);  // 사용할 채널 수에 따라 크기 조정
    actuator_control_msg.controls[0] = _pwm_nomallize;  // Example: set midpoint (1500 μs in PWM) for the first AUX channel
    _pwm_nomallize += (_pwm_nomallize < 0.9) * 0.1;
    std::cout << "Publishing arm control message" << _pwm_nomallize << std::endl;
    _publisher_arm->publish(actuator_control_msg);
}
