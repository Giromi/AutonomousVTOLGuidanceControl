#include "px4_ros_com/OffboardMavros.hpp"


/* -- Publish Functions -- */
void    OffboardMavros::publish(void) {

    if (_cmd_flag == vtol::INIT) {
        publishGpOrigin();
    }
    if (_cmd_flag != vtol::MC_START 
        && _cmd_flag != vtol::FW_START) {
        return ;
    } 

    publishRawLocalPosition();
    // publishPose();
    // publishCmdVel();
    // publishVelocity(); // 
    // publishManual();
    // publishRawAttitude();
    // publishAttitudePosition();
    // publishRawLocal();
    // if (fcu_state.mode == "AUTO.MISSION") {
    // publishWaypoint();
    // } else {
    //     publishRawLocal(); // publishLocalFixed();
    // }
}

void OffboardMavros::publishCmdVel(void) {
    RCLCPP_INFO(this->get_logger(), "publishing cmd_vel");
    auto msg = std::make_shared<geometry_msgs::msg::TwistStamped>();
    msg->header.stamp = this->now();
    msg->header.frame_id = "standard_vtol_0";
    msg->twist.linear.x = _local_velocity[0];
    msg->twist.linear.y = _local_velocity[1];
    msg->twist.linear.z = _local_velocity[2];
    msg->twist.angular.x = _local_velocity[3];
    msg->twist.angular.y = _local_velocity[4];
    msg->twist.angular.z = _local_velocity[5];
    att_pub->publish(*msg);
}

void OffboardMavros::publishManual(void) {
    RCLCPP_INFO(this->get_logger(), "publishing manual");
    auto msg = std::make_shared<mavros_msgs::msg::ManualControl>();
    msg->header.stamp = this->now();
    msg->header.frame_id = "standard_vtol_0";
    msg->x = _local_velocity[0];
    msg->y = _local_velocity[1];
    msg->z = _local_velocity[2];
    msg->r = _local_velocity[3];
    msg->buttons = 10;
    vc_manual_pub->publish(*msg);
}

void OffboardMavros::publishGpOrigin(void) {
    // RCLCPP_INFO(this->get_logger(), "publishing origin");
<<<<<<< Updated upstream
    auto msg = std::make_shared<geographic_msgs::msg::GeoPoseStamped>();
    msg->header.stamp = this->now();
    msg->header.frame_id = "standard_vtol_0";
    msg->pose.position.altitude = 47.6554;
    msg->pose.position.latitude = 47.3984;
    msg->pose.position.longitude = 8.54616;
    gp_origin_pub->publish(*msg);
=======
    geographic_msgs::msg::GeoPoseStamped origin;
    origin.header.stamp = this->now();
    origin.header.frame_id = "standard_vtol_0";
    origin.pose.position.altitude = 47.6554;
    origin.pose.position.latitude = 47.3984;
    origin.pose.position.longitude = 8.54616;
    gp_origin_pub->publish(origin);
>>>>>>> Stashed changes
    // DEBUG::print("publishing origin",     mavros_msgs::msg::State::MODE_PX4_MANUAL, BOLDGREEN);
}

void OffboardMavros::publishPose(void) {
<<<<<<< Updated upstream
    auto msg = std::make_shared<geometry_msgs::msg::PoseStamped>();
    msg->pose.position.x = _local_position[vtol::EAST];
    msg->pose.position.y = _local_position[vtol::NORTH];
    msg->pose.position.z = _local_position[vtol::UP];
    msg->pose.orientation.x = _local_velocity[3];
    msg->pose.orientation.y = _local_velocity[4];
    msg->pose.orientation.z = _local_velocity[5];
    local_pos_pub->publish(*msg);
=======
    geometry_msgs::msg::PoseStamped pose;
    // pose.pose.position.x = _local_position[vtol::EAST];
    // pose.pose.position.y = _local_position[vtol::NORTH];
    // pose.pose.position.z = _local_position[vtol::UP];
    pose.pose.orientation.x = _local_velocity[3];
    pose.pose.orientation.y = _local_velocity[4];
    pose.pose.orientation.z = _local_velocity[5];
    local_pos_pub->publish(pose);
>>>>>>> Stashed changes
}

void OffboardMavros::publishActuatorControls(void) {
    auto msg = std::make_shared<mavros_msgs::msg::ActuatorControl>();
    msg->group_mix = 2;
    msg->header.stamp = this->now();
    msg->header.frame_id = "standard_vtol_0";
    msg->controls[0] = 1.0f;
    msg->controls[1] = 1.0f;
    msg->controls[2] = 1.0f;
    msg->controls[7] = 1.0f;
    RCLCPP_INFO(this->get_logger(), "publishing actuator controls");
    actuator_control_pub->publish(*msg);
}

void OffboardMavros::publishVelocity(void) {
<<<<<<< Updated upstream
    auto msg = std::make_shared<geometry_msgs::msg::Twist>();
    msg->linear.x = _local_velocity[0];
    msg->linear.y = _local_velocity[1];
    msg->linear.z = _local_velocity[2];
    msg->angular.x = _local_velocity[3];
    msg->angular.y = _local_velocity[4];
    msg->angular.z = _local_velocity[5];
    local_vel_pub->publish(*msg);
}

void OffboardMavros::publishRawLocal(void) {
    std::cout << "Publishing Raw local..." << std::endl;
    auto msg = std::make_shared<mavros_msgs::msg::PositionTarget>();

    msg->header.stamp = this->now();
    msg->header.frame_id = "standard_vtol_0";
    msg->coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_LOCAL_NED;
    msg->type_mask = mavros_msgs::msg::PositionTarget::IGNORE_PX	|
                          mavros_msgs::msg::PositionTarget::IGNORE_PY	|
                          mavros_msgs::msg::PositionTarget::IGNORE_PZ	|
                          mavros_msgs::msg::PositionTarget::IGNORE_AFX	|
                          mavros_msgs::msg::PositionTarget::IGNORE_AFY	|
                          mavros_msgs::msg::PositionTarget::IGNORE_AFZ  ;
    // msg->velocity.x    = _local_velocity[0]; // East
    // msg->velocity.y    = _local_velocity[1]; // North
    // msg->velocity.z    = _local_velocity[2]; // Up
    msg->yaw           = _local_velocity[4];
    msg->yaw_rate      = _local_velocity[5];
    local_pub->publish(*msg);
}


=======
    geometry_msgs::msg::Twist vel;
    vel.linear.x = _local_velocity[0];
    // vel.linear.y = _local_velocity[1];
    // vel.linear.z = _local_velocity[2];
    // vel.angular.x = _local_velocity[3];
    // vel.angular.y = _local_velocity[4];
    // vel.angular.z = _local_velocity[5];
    local_vel_pub->publish(vel);
}

// void OffboardMavros::publishRawLocal(void) {
//     std::cout << "Publishing Raw local..." << std::endl;
//     mavros_msgs::msg::PositionTarget local_msg;
//     local_msg.header.stamp = this->now();
//     local_msg.header.frame_id = "standard_vtol_0";
//     local_msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_LOCAL_NED;
//     local_msg.type_mask = mavros_msgs::msg::PositionTarget::IGNORE_PX	|
//                           mavros_msgs::msg::PositionTarget::IGNORE_PY	|
//                           mavros_msgs::msg::PositionTarget::IGNORE_PZ	|
//                           mavros_msgs::msg::PositionTarget::IGNORE_AFX	|
//                           mavros_msgs::msg::PositionTarget::IGNORE_AFY	|
//                           mavros_msgs::msg::PositionTarget::IGNORE_AFZ  ;
//     // local_msg.velocity.x    = _local_velocity[0]; // East
//     // local_msg.velocity.y    = _local_velocity[1]; // North
//     // local_msg.velocity.z    = _local_velocity[2]; // Up
//     local_msg.yaw           = _local_velocity[4];
//     local_msg.yaw_rate      = _local_velocity[5];
//     local_pub->publish(local_msg);
// }

/* Thrust NaN 넣어도 알아서 작동 안됨 */
>>>>>>> Stashed changes
void OffboardMavros::publishRawAttitude(void) {
    // std::cout << "Publishing Raw Attitude ..." << std::endl;
    auto msg = std::make_shared<mavros_msgs::msg::AttitudeTarget>();
    msg->header.stamp = this->now();
    msg->header.frame_id = "standard_vtol_0";
    msg->type_mask =  \
                            mavros_msgs::msg::AttitudeTarget::IGNORE_ROLL_RATE  |
                            mavros_msgs::msg::AttitudeTarget::IGNORE_PITCH_RATE |
                            mavros_msgs::msg::AttitudeTarget::IGNORE_YAW_RATE;
                            // mavros_msgs::msg::AttitudeTarget::IGNORE_ATTITUDE;
                            // mavros_msgs::msg::AttitudeTarget::IGNORE_THRUST;
    tf2::Quaternion q;
    q.setRPY(_local_velocity[3],_local_velocity[4],_local_velocity[5]);
<<<<<<< Updated upstream
    msg->orientation.w    =  q.w();      //
    msg->orientation.x    =  q.x();      // roll : 키보드 2, 3  (2 : 반시계, 3 : 시계)
    msg->orientation.y    =  q.y();      //
    msg->orientation.z    =  q.z();      // pitch : 키보드 4, 5 (4 : 하강,   5 : 상승)
    // msg->body_rate.x    =  _local_velocity[3];    
    // msg->body_rate.y    =  _local_velocity[4]; 
    // msg->body_rate.z    =  _local_velocity[5]; 
    msg->thrust = _local_velocity[2];
    raw_attitude_pub->publish(*msg);
=======
    target_msg.orientation.w    =  q.w();      //
    target_msg.orientation.x    =  q.x();      
    // roll : 키보드 2, 3  (2 : 반시계, 3 : 시계)
    target_msg.orientation.y    =  q.y();      //
    target_msg.orientation.z    =  q.z();      
    // pitch : 키보드 4, 5 (4 : 하강,   5 : 상승)
    // target_msg.body_rate.x    =  _local_velocity[3];    
    // target_msg.body_rate.y    =  _local_velocity[4]; 
    // target_msg.body_rate.z    =  _local_velocity[5]; 

    target_msg.thrust = 0.9;
    raw_attitude_pub->publish(target_msg);
>>>>>>> Stashed changes
}

/**
 * @brief Publish local velocity
 * 
 */
void OffboardMavros::publishRawLocalPosition(void) {
    // DEBUG::msg("Publishing local raw...");
    auto msg = std::make_shared<mavros_msgs::msg::PositionTarget>();
    msg->header.stamp = this->now();
    msg->header.frame_id = "cau_vtol";
    msg->coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_LOCAL_NED;
    msg->type_mask = mavros_msgs::msg::PositionTarget::IGNORE_VX  |
                     mavros_msgs::msg::PositionTarget::IGNORE_VY  |
                     mavros_msgs::msg::PositionTarget::IGNORE_VZ  |
                     mavros_msgs::msg::PositionTarget::IGNORE_AFX |
                     mavros_msgs::msg::PositionTarget::IGNORE_AFY |
                     mavros_msgs::msg::PositionTarget::IGNORE_AFZ |   
                     mavros_msgs::msg::PositionTarget::IGNORE_VZ  |
                     mavros_msgs::msg::PositionTarget::IGNORE_YAW_RATE;
                     
    // FW일 때에는 yaw를 무시한다.
    msg->type_mask |= (_cmd_flag & vtol::BIT_FIXED) 
                                * mavros_msgs::msg::PositionTarget::IGNORE_YAW;

    DEBUG::print("type mask: ", msg->type_mask, BOLDWHITE);
    msg->position.x = wp_manager.getTarget()[vtol::EAST]; // East
    msg->position.y = wp_manager.getTarget()[vtol::NORTH]; // North
    msg->position.z = wp_manager.getTarget()[vtol::UP]; // Up
    msg->yaw        = static_cast<float>(wp_manager.getTarget()[vtol::YAW]);
    local_pub->publish(*msg);
}

void OffboardMavros::publishAttitudePosition(void) {
    DEBUG::message("Publishing attitude position", "", BOLDGREEN);
    auto msg = std::make_shared<geometry_msgs::msg::PoseStamped>();
    msg->header.stamp = this->now();
    msg->header.frame_id = "cau_vtol";

    tf2::Quaternion q;
    q.setRPY(_local_velocity[3],_local_velocity[4],_local_velocity[5]);
    msg->pose.position.x = _local_velocity[0]; // East
    msg->pose.position.y = _local_velocity[1]; // North
    msg->pose.position.z = _local_velocity[2]; // Up
    msg->pose.orientation.w    =  q.w();
    msg->pose.orientation.x    =  q.x();      
    msg->pose.orientation.y    =  q.y();
    msg->pose.orientation.z    =  q.z();      
    att_pos_pub->publish(*msg);
}
