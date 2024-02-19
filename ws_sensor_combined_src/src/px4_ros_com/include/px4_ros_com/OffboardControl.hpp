#ifndef OFFBOARD_CONTROL_HPP
# define OFFBOARD_CONTROL_HPP

// #include <chrono>
// #include <iostream>
# include <px4_msgs/msg/offboard_control_mode.hpp>
# include <px4_msgs/msg/trajectory_setpoint.hpp>
# include <px4_msgs/msg/vehicle_command.hpp>
# include <px4_msgs/msg/vehicle_control_mode.hpp>
# include <px4_msgs/msg/vehicle_local_position.hpp>
# include <rclcpp/rclcpp.hpp>
# include <stdint.h>
# include <array>
# include <queue>
#include <std_msgs/msg/string.hpp>
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace px4_msgs::msg;


struct WayPoint {
    WayPoint(void) : north(0.0f), east(0.0f), down(0.0f), yaw(0.0f), is_dubins_path(false) {}

    WayPoint(float north, float east, float down, float yaw, bool is_dubins_path = false) 
        : north(north), east(east), down(down), yaw(yaw), is_dubins_path(is_dubins_path) {}

    float north;
    float east;
    float down;
    float yaw;
    bool is_dubins_path;
};


struct DubinsPathPoint : public WayPoint {
    DubinsPathPoint(float north, float east, float down, float yaw, double time_stamp = 0.0, bool is_dubins_path = true) 
        : WayPoint(north, east, down, yaw, is_dubins_path), time_stamp(time_stamp) { }
    double time_stamp;
};

enum e_coordinate {NORTH, EAST, DOWN, YAW};

class OffboardControl : public rclcpp::Node
{
private:
    typedef std::array<float, 4> LocalPosition;
public:
	OffboardControl(void);
	void        arm(void);
	void        disarm(void);

    void        dubins_path_planning(void);
    static void set_way_point(WayPoint way_point);
    static int set_dubins_path_point(double q[3], double x, void* user_data);
private:
    // static std::queue<std::array<float, 4>> way_points_;
    static std::queue<WayPoint>         _way_points;
    static std::queue<DubinsPathPoint>  _dubins_path_points;
    bool is_reach_way_point_with_square(std::array<float, 3> target);
    bool is_reach_way_point_with_norm(std::array<float, 3> target);
    bool is_reach_way_point_with_norm(std::array<float, 2> target);

	rclcpp::TimerBase::SharedPtr timer_;
	rclcpp::Publisher<OffboardControlMode>::SharedPtr offboard_control_mode_publisher_;
	rclcpp::Publisher<TrajectorySetpoint>::SharedPtr trajectory_setpoint_publisher_;
	rclcpp::Publisher<VehicleCommand>::SharedPtr vehicle_command_publisher_;

	rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>::SharedPtr vehicle_local_position_subscription_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr _key_event_subscription;

    std::array<float, 4> _local_position;
	std::atomic<uint64_t> _timestamp;       //!< common synced timestamped
	uint64_t _offboard_setpoint_counter;    //!< counter for the number of setpoints sent
    size_t _norm = 0;
                                            //
	void publish_offboard_control_mode(void);
	void publish_trajectory_setpoint(void);
    void publish_trajectory_setpoint_dubins_path(void);
	void publish_vehicle_command(uint16_t command, float param1 = 0.0, float param2 = 0.0);
    void make_general_trajectory_setpoint(TrajectorySetpoint& msg);
    void make_dubins_trajectory_setpoint(TrajectorySetpoint& msg);
    void chatterCallback(const std_msgs::msg::String::SharedPtr msg);

    static double turning_radius;
    static double sampling_interval;
    
};

#endif


