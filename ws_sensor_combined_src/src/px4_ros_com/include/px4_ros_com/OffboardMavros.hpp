#ifndef OFFBOARD_MAVROS_HPP
# define OFFBOARD_MAVROS_HPP


# include <rclcpp/rclcpp.hpp>
# include <tf2/LinearMath/Quaternion.h>
# include <geometry_msgs/msg/pose_stamped.hpp>
# include <geometry_msgs/msg/twist_stamped.hpp>
# include <geographic_msgs/msg/geo_pose_stamped.hpp>
# include <mavros_msgs/msg/state.hpp>
# include <mavros_msgs/msg/actuator_control.hpp>
# include <mavros_msgs/msg/override_rc_in.hpp>
# include <mavros_msgs/msg/position_target.hpp>
# include <mavros_msgs/msg/attitude_target.hpp>
# include <mavros_msgs/msg/extended_state.hpp>
# include <mavros_msgs/srv/command_bool.hpp>
# include <mavros_msgs/srv/command_tol.hpp>
# include <mavros_msgs/srv/command_long.hpp>
# include <mavros_msgs/msg/command_code.hpp>
# include <mavros_msgs/srv/set_mode.hpp>
# include <mavros_msgs/msg/waypoint_list.hpp>

# include <mavros_msgs/msg/waypoint.hpp>
# include <mavros_msgs/msg/waypoint.hpp>
# include <mavros_msgs/msg/manual_control.hpp>
# include <geometry_msgs/msg/twist_stamped.hpp>


# include <mavros_msgs/msg/waypoint_reached.hpp>
# include <mavros_msgs/srv/command_vtol_transition.hpp>
# include <mavros_msgs/srv/waypoint_push.hpp>
# include <mavros_msgs/srv/waypoint_clear.hpp>
# include <std_msgs/msg/string.hpp>
# include <nav_msgs/msg/odometry.hpp>
# include <sensor_msgs/msg/nav_sat_fix.hpp>
# include <mavlink/v2.0/common/common.hpp>

//#include <nav_msgs/msg/odometry.hpp>
# include <functional>
# include <algorithm>
# include <array>
# include <limits>
# include <cmath>
# include <cstdio>
# include <queue>
# include <deque>
# include <Eigen/Dense>
# include "px4_ros_com/convention.hpp"
# include "px4_ros_com/WaypointManager.hpp"
# include "DEBUG.hpp"

typedef unsigned int            t_bit;
typedef std::array<double, 3>   t_position;
typedef std::array<float, 3>    t_global_position;

struct Quaternion {
    double w, x, y, z;
};

class OffboardMavros : public rclcpp::Node {
public:
    OffboardMavros(void);

private:
    /* -- Initialize Functions -- */

    void        initializePublishers(void);
    void        initializeSubscribers(void);
    void        initializeClients(void); 
    void        initializeTimers(const int rate_hz);
    void        initializeConstant(void);
    void        initializeFunctionPointerArray(void);
    void        initializeVariables(void);
    void        initializeStateFuncPointerArray(const std::array<std::function <void(void)>, 
                                                vtol::STATE_SIZE>& input);
    void        initializeWaypoints(void);
    void        setWaypoint(const uint8_t frame, const uint16_t command, 
                                 const bool is_current, const bool autocontinue, 
                                 const std::array<float, 4> param, 
                                 const std::array<double, 3> pos);
    void        triangleScenarioMC(const std::array<double, 3>& target_pos);
    void        triangleScenarioFW(const std::array<double, 3>& target_pos);


        
    /* -- Callback Functions -- */
    void    gpsCallBack(const sensor_msgs::msg::NavSatFix::SharedPtr msg);
    void    poseCallBack(const geometry_msgs::msg::PoseStamped::SharedPtr msg);
    void    stateCallBack(const mavros_msgs::msg::State::SharedPtr msg);
    void    statusReady(void);


    /* -- Publish Functions -- */
    void    publish(void);
    void    publishPose(void);
    void    publishActuatorControls(void);
    void    publishVelocity(void); 
    void    publishAttitude(void);
    void    publishLocal(void);
    void    publishRawLocalPosition(void);
    void    publishRawLocal(void);
    void    publishRawAttitude(void);

    void    publishWaypoint(void);
    void    publishGpOrigin(void);
    void    publishCmdVel(void);
    void    publishManual(void);
    void    publishAttitudePosition(void);


    /* -- Update Functions -- */
    void    updateArmingStatus(void); 
    void    updateDisarmingStatus(void); 
    void    updateTransitionFixedStatus(void); 
    void    updateTransitionQuadStatus(void); 
    void    requestTransitionStatus(const int input,
            void (OffboardMavros::*response_callback)
            (const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture));

    void    requestArmingStatus(const bool& input,
            void (OffboardMavros::*response_callback)
            (const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture));

    void    updateTakeoffStatus(void);
    void    updateHoldMode(void);
    void    updateOffboardMode(void); 
    void    updateMissionMode(void);
    void    updatePositionMode(void);
    void    updateCustomMode(
                const std::string& input_mode,
                void (OffboardMavros::*responseCallback) (const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture, const std::array<const std::string, 2>), 
                const std::array<const std::string, 2>& msg);

    void    updateLandingStatus(void);
    void    updateLocation(std::array<double, 3> input);

    std::shared_ptr<mavros_msgs::srv::CommandTOL::Request>  
            makeRequestTakeoffLandMessage(const vtol::GeographicCoordinate& input);
    void    sendFixedHeadingCommand(void);
    void    updateWaypointPush(void);
    void    updateWaypointClear(void);
    void    sendChangeSpeedCommand(const double& speed);

    /* -- StateCommand Function*/
    void    stateCommandInit(void);
    void    stateCommandReady(void);
    void    stateCommandArmed(void);
    void    stateCommandFly(void);
    void    stateCommandTakeOff(void);
    void    stateCommandStartMC(void);
    void    stateCommandStartFW(void);
    void    stateCommandMission(void);
    void    stateCommandFixed(void);
    void    stateCommandToFixed(void);
    void    stateCommandToQuad(void);
    void    stateCommandLand(void);


    void    localPositionCommandStart(void);
    // void    is_arrived_waypoint(const std::array<float, 3> target);

    /* -- Callback Functions -- */
    void   modeSentResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2> msg);
    // void   successResponseCallback(const rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future, const std::array<const std::string, 2> msg);

    void	chatterCallback(const std_msgs::msg::String::SharedPtr msg);
    void	armingResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future);
    void	disarmingResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture future);
    void	transitionResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedFuture future);
    void    takeoffResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future);
    void    landResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future);
    void    locationResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future);
    void    localPositionCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg);
    void    cmdResponseCallback(const rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedFuture future);
    void    waypointPushResponseCallback(const rclcpp::Client<mavros_msgs::srv::WaypointPush>::SharedFuture future);
    void    waypointClearResponseCallback(const rclcpp::Client<mavros_msgs::srv::WaypointClear>::SharedFuture future);

    /* -- Action Functions -- */
    static void	    _actionGoNorth(void);
    static void	    _actionGoEast(void);
    static void	    _actionGoDown(void);
    static void	    _actionGoSouth(void);
    static void	    _actionGoWest(void);
    static void	    _actionGoUp(void);
    static void	    _actionVelocityPlusX(void);
    static void	    _actionVelocityPlusY(void);
    static void	    _actionVelocityPlusZ(void);
    static void	    _actionVelocityMinusX(void);
    static void	    _actionVelocityMinusY(void);
    static void	    _actionVelocityMinusZ(void);
    static void	    _actionVelocityPlusRoll(void);
    static void	    _actionVelocityPlusPitch(void);
    static void	    _actionVelocityPlusYaw(void);
    static void	    _actionVelocityMinusRoll(void);
    static void	    _actionVelocityMinusPitch(void);
    static void	    _actionVelocityMinusYaw(void);
    static void	    _actionReturnHome(void);
    static void	    _actionArming(void);
    static void	    _actionDisarming(void);
    static void	    _actionTakeoff(void);
    static void	    _actionLanding(void);
    static void	    _actionStart(void);
    static void     _actionMission(void);
    static void	    _actionHold(void);
    static void	    _actionInit(void);
    static void	    _actionTransition(void);



    /* -- Utile Functions -- */
    bool            isPassedSeconds(const double timer);
    void            printSuccessInfo(const bool success, const std::array<const std::string, 2>& msg) const;
    void            handleCommandFlag(const t_bit flag);
    static void     printReferenceInput(void); 
    bool            isGlobalPositionGettingValue(const t_global_position&) const;
    void            commandFlagTurnOff(const t_bit& flag);
    void            commandFlagTurnOn(const t_bit& flag);
    const Quaternion rpy_to_quat(const double roll, const double pitch, const double yaw);

    /* -- Members Variables -- */
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr       local_pos_pub;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr             local_vel_pub;
    rclcpp::Publisher<mavros_msgs::msg::PositionTarget>::SharedPtr      local_pub;
    rclcpp::Publisher<mavros_msgs::msg::PositionTarget>::SharedPtr      target_local_pub;
    rclcpp::Publisher<mavros_msgs::msg::ManualControl>::SharedPtr       vc_manual_pub;
    rclcpp::Publisher<mavros_msgs::msg::AttitudeTarget>::SharedPtr      raw_attitude_pub;
    rclcpp::Publisher<mavros_msgs::msg::ActuatorControl>::SharedPtr     actuator_control_pub;
    rclcpp::Publisher<mavros_msgs::msg::WaypointList>::SharedPtr        waypoints_pub;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr       att_pos_pub;
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr      att_vel_pub;
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr      cmd_vel_pub;
    rclcpp::Publisher<geographic_msgs::msg::GeoPoseStamped>::SharedPtr  gp_origin_pub;

    rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr            arming_client;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr             takeoff_client;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr             landing_client;
    rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedPtr            cmd_client;
    rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr                set_mode_client;
    rclcpp::Client<mavros_msgs::srv::CommandLong>::SharedPtr            location_client;
    rclcpp::Client<mavros_msgs::srv::CommandVtolTransition>::SharedPtr  transition_client;
    rclcpp::Client<mavros_msgs::srv::WaypointPush>::SharedPtr           waypoint_push_client;
    rclcpp::Client<mavros_msgs::srv::WaypointClear>::SharedPtr          waypoint_clear_client;

    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    local_position_sub;
    rclcpp::Subscription<sensor_msgs::msg::NavSatFix>::SharedPtr        global_posistion_sub;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr    pose_sub; 
    rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr            state_sub;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr              subscription;
    rclcpp::TimerBase::SharedPtr                                        timer;
    rclcpp::Time                                                        last_request{0, 0, RCL_ROS_TIME};
    mavros_msgs::msg::State                                             fcu_state;
    mavros_msgs::msg::WaypointList                                      waypoint_list;
    double yaw_current;
    t_global_position		                                        init_global_position;
  
    // static const std::array<std::string, vtol::ACTION_SIZE>        _action_string_array;
    //

    //TODO: static 지워서 멤버변수로 변경

    static t_bit                                                _cmd_flag;
    static std::array<double, 3>		                            _local_position;
    static std::array<float, 3>		                              _global_position;
    static std::array<double, 6>		                            _local_velocity;
    static std::array<double, 3>		                            _cur_position;
    static std::array<double, 3>		                            _prev_position;
    static const std::string				                        _arrow_string;
    static std::array<double, 4>                                    _manual_velocity;

    static double                                                       _offset;
    static const std::array<std::string, vtol::ACTION_SIZE>             _action_string_array;
    static void                                                         (*actionFunc[])(void);
    std::array<t_bit, vtol::STATE_SIZE>                           state_value_array;
    std::array<std::function <void(void)> ,vtol::STATE_SIZE>            stateFunc;

    /* 일바적인 모든 용*/

    /* mavros mission send 용*/
    std::queue<vtol::ReferenceWaypoint>                                 ref_waypoints;
    bool                                                                gps_locked{false};
    
    WaypointManager<Eigen::Vector4d>                               wp_manager;
    static const std::array<Eigen::Vector4d, 4>                    _square_path;
    static const std::array<Eigen::Vector4d, 4>                    _triangle_path;
    static const std::array<Eigen::Vector4d, 11>                   _star_path;
};




#endif
