#include "px4_ros_com/OffboardMavros.hpp"


/* -- Publish Functions -- */
void    OffboardMavros::publish(void) {

    if (_cmd_flag == vtol::INIT) {
        publishGpOrigin();
    }
    if (_cmd_flag != vtol::START && _cmd_flag != vtol::MISSION) {
        return ;
    } 
    publishRawAttitude();

    // publishCmdVel();
    // publishVelocity();
    // 
    // publishManual();
    // publishRawAttitude();
    // publishRawLocal();
    // if (fcu_state.mode == "AUTO.MISSION") {
    // publishWaypoint();
    // } else {
    //     publishRawLocal(); // publishLocalFixed();
    // }
}

void OffboardMavros::publishCmdVel(void) {
    RCLCPP_INFO(this->get_logger(), "publishing cmd_vel");
    geometry_msgs::msg::TwistStamped cmd_vel;
    cmd_vel.header.stamp = this->now();
    cmd_vel.header.frame_id = "standard_vtol_0";
    cmd_vel.twist.linear.x = _local_velocity[0];
    cmd_vel.twist.linear.y = _local_velocity[1];
    cmd_vel.twist.linear.z = _local_velocity[2];
    cmd_vel.twist.angular.x = _local_velocity[3];
    cmd_vel.twist.angular.y = _local_velocity[4];
    cmd_vel.twist.angular.z = _local_velocity[5];
    att_pub->publish(cmd_vel);


}

void OffboardMavros::publishManual(void) {
    RCLCPP_INFO(this->get_logger(), "publishing manual");
    mavros_msgs::msg::ManualControl manual;
    manual.header.stamp = this->now();
    manual.header.frame_id = "standard_vtol_0";
    manual.x = _local_velocity[0];
    manual.y = _local_velocity[1];
    manual.z = _local_velocity[2];
    manual.r = _local_velocity[3];
    manual.buttons = 10;
    vc_manual_pub->publish(manual);
    

}

void OffboardMavros::publishGpOrigin(void) {
    // RCLCPP_INFO(this->get_logger(), "publishing origin");
    geographic_msgs::msg::GeoPoseStamped origin;
    origin.header.stamp = this->now();
    origin.header.frame_id = "standard_vtol_0";
    origin.pose.position.altitude = 47.6554;
    origin.pose.position.latitude = 47.3984;
    origin.pose.position.longitude = 8.54616;

    gp_origin_pub->publish(origin);
}

void OffboardMavros::publishPose(void) {
    geometry_msgs::msg::PoseStamped pose;
    pose.pose.position.x = _local_position[vtol::EAST];
    pose.pose.position.y = _local_position[vtol::NORTH];
    pose.pose.position.z = _local_position[vtol::UP];
    local_pos_pub->publish(pose);
}

void OffboardMavros::publishActuatorControls(void) {
    mavros_msgs::msg::ActuatorControl actuator_control_msg;
    actuator_control_msg.group_mix = 2;
    actuator_control_msg.header.stamp = this->now();
    actuator_control_msg.header.frame_id = "standard_vtol_0";
    actuator_control_msg.controls[0] = 1.0f;
    actuator_control_msg.controls[1] = 1.0f;
    actuator_control_msg.controls[2] = 1.0f;
    actuator_control_msg.controls[7] = 1.0f;
    RCLCPP_INFO(this->get_logger(), "publishing actuator controls");
    actuator_control_pub->publish(actuator_control_msg);
}


void OffboardMavros::publishVelocity(void) {
    geometry_msgs::msg::Twist vel;
    vel.linear.x = _local_velocity[0];
    vel.linear.y = _local_velocity[1];
    vel.linear.z = _local_velocity[2];
    vel.angular.x = _local_velocity[3];
    vel.angular.y = _local_velocity[4];
    vel.angular.z = _local_velocity[5];
    local_vel_pub->publish(vel);
}

void OffboardMavros::publishRawLocal(void) {
    std::cout << "Publishing Raw local..." << std::endl;
    mavros_msgs::msg::PositionTarget local_msg;
    local_msg.header.stamp = this->now();
    local_msg.header.frame_id = "standard_vtol_0";
    local_msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_LOCAL_NED;
    local_msg.type_mask = mavros_msgs::msg::PositionTarget::IGNORE_PX	|
                          mavros_msgs::msg::PositionTarget::IGNORE_PY	|
                          mavros_msgs::msg::PositionTarget::IGNORE_PZ	|
                          mavros_msgs::msg::PositionTarget::IGNORE_AFX	|
                          mavros_msgs::msg::PositionTarget::IGNORE_AFY	|
                          mavros_msgs::msg::PositionTarget::IGNORE_AFZ  ;
    local_msg.velocity.x    = _local_velocity[0]; // East
    local_msg.velocity.y    = _local_velocity[1]; // North
    local_msg.velocity.z    = _local_velocity[2]; // Up
    local_msg.yaw           = _local_velocity[4];
    local_msg.yaw_rate      = _local_velocity[5];
    local_pub->publish(local_msg);
}

void OffboardMavros::publishRawAttitude(void) {
    // std::cout << "Publishing Raw Attitude ..." << std::endl;
    mavros_msgs::msg::AttitudeTarget target_msg;
    target_msg.header.stamp = this->now();
    target_msg.header.frame_id = "standard_vtol_0";
    target_msg.type_mask =  \
                            mavros_msgs::msg::AttitudeTarget::IGNORE_ROLL_RATE  |
                            mavros_msgs::msg::AttitudeTarget::IGNORE_PITCH_RATE |
                            mavros_msgs::msg::AttitudeTarget::IGNORE_YAW_RATE;
                            // mavros_msgs::msg::AttitudeTarget::IGNORE_ATTITUDE;
                             // mavros_msgs::msg::AttitudeTarget::IGNORE_THRUST;

    tf2::Quaternion q;
    q.setRPY(_local_velocity[3],_local_velocity[4],_local_velocity[5]);
    target_msg.orientation.w    =  q.w();      //
    target_msg.orientation.x    =  q.x();      // roll : 키보드 2, 3  (2 : 반시계, 3 : 시계)
    target_msg.orientation.y    =  q.y();      //
    target_msg.orientation.z    =  q.z();      // pitch : 키보드 4, 5 (4 : 하강,   5 : 상승)
    // target_msg.body_rate.x    =  _local_velocity[3];    
    // target_msg.body_rate.y    =  _local_velocity[4]; 
    // target_msg.body_rate.z    =  _local_velocity[5]; 

    target_msg.thrust = _local_velocity[2];
    raw_attitude_pub->publish(target_msg);
}




/**
 * @brief Publish local velocity
 * fixed wing은 velocity로 제어가 불가능해 보임
 * 왜인지 모르겟찌만, 아래처럼 하면 가능하긴 함. 다만 늦음
 */
void OffboardMavros::publishLocalFixed(void) {
    std::cout << "Publishing local fixed..." << std::endl;
    mavros_msgs::msg::PositionTarget local_msg;
    local_msg.header.stamp = this->now();
    local_msg.header.frame_id = "standard_vtol_0";
    local_msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_LOCAL_NED;
    local_msg.type_mask = \
                          //mavros_msgs::msg::PositionTarget::IGNORE_PX |
                          //mavros_msgs::msg::PositionTarget::IGNORE_PY |
                          //mavros_msgs::msg::PositionTarget::IGNORE_PZ |
        mavros_msgs::msg::PositionTarget::IGNORE_AFX |
        mavros_msgs::msg::PositionTarget::IGNORE_AFY |
        mavros_msgs::msg::PositionTarget::IGNORE_AFZ;
    //mavros_msgs::msg::PositionTarget::IGNORE_VZ;
    //mavros_msgs::msg::PositionTarget::IGNORE_YAW_RATE;
    local_msg.velocity.x = _local_velocity[0]; // EAST
    local_msg.velocity.y = _local_velocity[1]; // North
    local_msg.velocity.z = _local_velocity[2]; // UP
    local_msg.yaw =      _local_velocity[4];
    local_msg.yaw_rate = _local_velocity[5];

    local_pub->publish(local_msg);
}


void OffboardMavros::publishWaypoint(void) {
    // mavros_msgs::msg::WaypointList  msg_waypoint_list;
    // if (waypoints.size()) { // 넣을게 없으면 그냥 나가기
    //     std::cout << "Publishing WaypointList..." << std::endl;
    //     float_t
    //     mavros_msgs::msg::Waypoint      msg_waypoint;
    //     msg_waypoint.frame = mavros_msgs::msg::Waypoint::FRAME_LOCAL_NED;
    //     msg_waypoint.command = mavros_msgs::msg::CommandCode::NAV_WAYPOINT;
    //     msg_waypoint.is_current = true;
    //     msg_waypoint.autocontinue = true;
    //     msg_waypoint.param1 = 0;
    //     msg_waypoint.param2 = 0;
    //     msg_waypoint.param3 = 0;
    //     msg_waypoint.param4 = 0;
    //     msg_waypoint.x_lat = waypoints.front().x;
    //     msg_waypoint.y_long = waypoints.front().y;
    //     msg_waypoint.z_alt = waypoints.front().z;
    //
    //     msg_waypoint_list.waypoints.push_back(msg_waypoint);
    //     waypoints.pop();
    // }
    //
    // // if (waypoints.size()) { // 넣을게 있으면 publish 나중에 하기
    // //     return ;
    // // }
    // waypoints_pub->publish(msg_waypoint_list);
}
