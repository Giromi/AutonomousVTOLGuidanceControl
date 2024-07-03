#ifndef OFFBOARD_CONTROL_HPP
# define OFFBOARD_CONTROL_HPP

# include <px4_msgs/msg/offboard_control_mode.hpp>
# include <px4_msgs/msg/trajectory_setpoint.hpp>
# include <px4_msgs/msg/vehicle_command.hpp>
# include <px4_msgs/msg/vehicle_control_mode.hpp>
# include <px4_msgs/msg/vehicle_local_position.hpp>
# include <mavros_msgs/msg/actuator_control.hpp>
# include <rclcpp/rclcpp.hpp>
# include <stdint.h>
# include <array>
# include <queue>
# include <std_msgs/msg/string.hpp>
# include "px4_ros_com/convention.hpp"
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace px4_msgs::msg;


struct WayPoint {
    WayPoint(void) : north(0.0f), east(0.0f), down(0.0f), yaw(0.0f), is_dubins_path(false) { }

    WayPoint(float north, float east, float down, float yaw, bool is_dubins_path = false) 
        : north(north), east(east), down(down), yaw(yaw), is_dubins_path(is_dubins_path) { }

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

class OffboardControl : public rclcpp::Node {
private:
    typedef std::array<float, 4> LocalPosition;
public:
	OffboardControl(void);
	void        arm(void);
	void        disArm(void);

    void        dubinsPathPlanning(void);
    static void _setWayPoint(WayPoint way_point);
    static int _setDubinsPathPoint(double q[3], double x, void* user_data);
private:
    // static std::queue<std::array<float, 4>> way_points_;
    void	   initializeSubscribers(void);
    void	   initializePublishers(void);
    void	   initializeClients(void);

    static std::queue<WayPoint>         _way_points;
    static std::queue<DubinsPathPoint>  _dubins_path_points;
    bool isReachWayPointWithSquare(std::array<float, 3> target);
    bool isReachWayPointWithNorm(std::array<float, 3> target);
    bool isReachWayPointWithNorm(std::array<float, 2> target);

	rclcpp::TimerBase::SharedPtr timer;
	rclcpp::Publisher<OffboardControlMode>::SharedPtr offboard_control_mode_publisher;
	rclcpp::Publisher<TrajectorySetpoint>::SharedPtr trajectory_setpoint_publisher;
	rclcpp::Publisher<VehicleCommand>::SharedPtr vehicle_command_publisher;
    rclcpp::Publisher<mavros_msgs::msg::ActuatorControl>::SharedPtr     publisher_arm;

	rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>::SharedPtr vehicle_local_position_subscription;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr key_event_subscription;

    std::array<float, 4> local_position;
	std::atomic<uint64_t> timestamp;       //!< common synced timestamped
    size_t _norm = 0;
	uint64_t offboard_setpoint_counter = 0;    //!< counter for the number of setpoints sent
    int      pwm;
    float    pwm_nomallize;
                                            //
	void publishOffboardControlMode(void);
	void publishTrajectorySetpoint(void);
    void publishTrajectorySetpointDubinsPath(void);
	void publishVehicleCommand(uint16_t command, float param1 = 0.0, float param2 = 0.0);
    void makeGeneralTrajectorySetpoint(TrajectorySetpoint& msg);
    void makeDubinsTrajectorySetpoint(TrajectorySetpoint& msg);
    void publishArmControlMessage(void);
    void publishPwmOutputMessage(void);
    void chatterCallback(const std_msgs::msg::String::SharedPtr msg);

    static double _turning_radius;
    static double _sampling_interval;
    
};

#endif


