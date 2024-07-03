#ifndef OFFBOARD_MAVROS_HPP
# define OFFBOARD_MAVROS_HPP

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <mavros_msgs/srv/command_bool.hpp>
#include <mavros_msgs/srv/set_mode.hpp>
#include <mavros_msgs/msg/state.hpp>
#include <mavros_msgs/msg/actuator_control.hpp>
#include <mavros_msgs/msg/override_rc_in.hpp>
#include <mavros_msgs/msg/position_target.hpp>
#include <mavros_msgs/srv/command_tol.hpp>
#include <mavros_msgs/srv/command_long.hpp>
#include <mavros_msgs/srv/command_vtol_transition.hpp>
#include <mavros_msgs/msg/command_code.hpp>
#include <std_msgs/msg/string.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <array>
#include "px4_ros_com/convention.hpp"
#include <limits>
//#include <nav_msgs/msg/odometry.hpp>
#include "DEBUG.hpp"
#include <cmath>


class OffboardMavros : public rclcpp::Node {
public:
    OffboardMavros(void);

private:
    typedef std::array<double, 3> t_position;

    /* -- Initialize Functions -- */
    void    initializePublishers(void);
    void    initializeSubscribers(void);
    void    initializeClients(void); 
    void    initializeTimers(const int rate_hz);

    /* -- Callback Functions -- */
    void    poseCallBack(const geometry_msgs::msg::PoseStamped::SharedPtr msg);
    void    stateCallBack(const mavros_msgs::msg::State::SharedPtr msg);
    void    StatusReady(void);

    /* -- Publish Functions -- */
    void    publish(void);
    void    publishPose(void);
    void    publishActuatorControls(void);
    void    publish_velocity(void); 
    void    publish_attitude(void);
    void    publish_local(void);

    /* -- Update Functions -- */
    void    update_arming_status(void); 
    void    update_disarming_status(void); 
    void	update_transition_fixed_status(void); 
    void	update_transition_quad_status(void); 
    
    void    request_transition_status(const int input,
            void (OffboardMavros::*response_callback)
            (const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture));

    void    request_arming_status(const bool& input,
            void (OffboardMavros::*response_callback)
            (const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture));

    void    update_takeoff_status(void);
    void    update_landing_status(void);
    void    update_location_(std::array<double, 3> input);
    void    update_hold_mode(void);
    void    update_offboard_mode(void); 
    void    update_custom_mode(const std::string& input_mode,
            void (OffboardMavros::*response_callback)(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture));
    std::shared_ptr<mavros_msgs::srv::CommandTOL::Request>  
            make_request_takeoff_land_message(const vtol::GeographicCoordinate& input);
    void    sendFixedHeadingCommand(void);

    /* -- Callback Functions -- */
    void	offboard_response_callback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future);
    void	hold_response_callback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future);
    void	chatterCallback(const std_msgs::msg::String::SharedPtr msg);
    void	arming_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future);
    void	disarming_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future);
    void	transition_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture future);
    void    takeoff_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future);
    void    land_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future);
    void    location_response_callback(const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future);
    void    currentpositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg);
    void    cmdResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future);

    /* -- Action Functions -- */
    static void	    action_go_north(void);
    static void	    action_go_east(void);
    static void	    action_go_down(void);
    static void	    action_go_south(void);
    static void	    action_go_west(void);
    static void	    action_go_up(void);
    static void	    action_velocity_plus_x(void);
    static void	    action_velocity_plus_y(void);
    static void	    action_velocity_plus_z(void);
    static void	    action_velocity_minus_x(void);
    static void	    action_velocity_minus_y(void);
    static void	    action_velocity_minus_z(void);
    static void	    action_velocity_plus_roll(void);
    static void	    action_velocity_plus_pitch(void);
    static void	    action_velocity_plus_yaw(void);
    static void	    action_velocity_minus_roll(void);
    static void	    action_velocity_minus_pitch(void);
    static void	    action_velocity_minus_yaw(void);
    static void	    action_return_home(void);
    static void	    action_arming(void);
    static void	    action_disarming(void);
    static void	    action_takeoff(void);
    static void	    action_landing(void);
    static void	    action_start(void);
    static void	    action_hold(void);
    static void	    action_init(void);
    static void	    action_transition(void);

    /* -- Utile Functions -- */
    bool            is_five_seconds_passed();
    void            print_success_info(bool success, const char* msg[]) const;
    static void     print_reference_input(void); 

    /* -- Members Variables -- */
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr       local_pos_pub_;
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr      local_vel_pub;
    rclcpp::Publisher<mavros_msgs::msg::PositionTarget>::SharedPtr      local_pub;
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr      att_pub;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    current_pos_sub_;
    rclcpp::Publisher<mavros_msgs::msg::ActuatorControl>::SharedPtr     actuator_control_pub_;

    rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr            arming_client_;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr             takeoff_client_;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr             landing_client_;
    rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr                set_mode_client_;
    rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedPtr            location_client_;
    rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedPtr  transition_client_;
    rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedPtr            cmd_client;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    pose_sub_; 
    rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr            state_sub_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr              subscription_;
    rclcpp::TimerBase::SharedPtr                                        timer_;
    mavros_msgs::msg::State                                             fcuState_;
    rclcpp::Time                                                        last_request_{0, 0, RCL_ROS_TIME};

    double yaw_current;
    // static const std::array<std::string, vtol::ACTION_SIZE>        action_string_array_;

    //TODO: static 지워서 멤버변수로 변경
    static unsigned char                                                cmdFlag_;
    static std::array<double, 3>		                                local_position_;
    static std::array<double, 6>		                                local_velocity_;
    static std::array<double, 3>		                                cur_position_;
    static std::array<double, 3>		                                prev_position_;
    static const std::string				                            arrow_string_;
    static double                                                       offset_;
    static const std::array<std::string, vtol::ACTION_SIZE>             action_string_array_;
    static void                                                         (*action_func_[])(void);
};

#endif
