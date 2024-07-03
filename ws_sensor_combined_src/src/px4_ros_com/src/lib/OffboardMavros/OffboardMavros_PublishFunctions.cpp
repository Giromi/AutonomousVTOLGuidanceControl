#include "px4_ros_com/OffboardMavros.hpp"


/* -- Publish Functions -- */
void    OffboardMavros::publish(void) {
    if (cmdFlag_ != vtol::START) {
        return ;
    }
    // publishPose();
    // publish_velocity();
    publish_local_fixed();
    // publish_local();
    // std::cout << "Publishing..." << std::endl;
    // publish_attitude_(); // orbit 안사라짐
}

void OffboardMavros::publishPose(void) {
    geometry_msgs::msg::PoseStamped pose;
    pose.pose.position.x = local_position_[vtol::EAST];
    pose.pose.position.y = local_position_[vtol::NORTH];
    pose.pose.position.z = local_position_[vtol::UP];
    local_pos_pub_->publish(pose);
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
    actuator_control_pub_->publish(actuator_control_msg);
}

void OffboardMavros::publish_velocity(void) {
    geometry_msgs::msg::Twist vel;
    vel.linear.x = local_velocity_[0];
    vel.linear.y = local_velocity_[1];
    vel.linear.z = local_velocity_[2];
    vel.angular.x = local_velocity_[3];
    vel.angular.y = local_velocity_[4];
    vel.angular.z = local_velocity_[5];
    local_vel_pub->publish(vel);
}

void OffboardMavros::publish_attitude(void) {
    geometry_msgs::msg::TwistStamped att;
    att.twist.linear.x = local_velocity_[0];
    att.twist.linear.y = local_velocity_[1];
    att.twist.linear.z = local_velocity_[2];
    att.twist.angular.x = local_velocity_[3];
    att.twist.angular.y = local_velocity_[4];
    att.twist.angular.z = local_velocity_[5];
    att_pub->publish(att);
}

// void OffboardMavros::publish_local(void) {
//     mavros_msgs::msg::PositionTarget local_msg;
//     local_msg.header.stamp = this->now();
//     local_msg.header.frame_id = "standard_vtol_0";
//     local_msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_LOCAL_NED;
//     local_msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_BODY_NED;
//     local_msg.type_mask = 
//                           // mavros_msgs::msg::PositionTarget::IGNORE_PX       |
//                           // mavros_msgs::msg::PositionTarget::IGNORE_PY       |
//                           // mavros_msgs::msg::PositionTarget::IGNORE_PZ       |
//                           // mavros_msgs::msg::PositionTarget::IGNORE_VX    |
//                           // mavros_msgs::msg::PositionTarget::IGNORE_VY    |
//                           // mavros_msgs::msg::PositionTarget::IGNORE_VZ       |
//                           mavros_msgs::msg::PositionTarget::IGNORE_AFX      |
//                           mavros_msgs::msg::PositionTarget::IGNORE_AFY      |
//                           mavros_msgs::msg::PositionTarget::IGNORE_AFZ      ;
//                           // mavros_msgs::msg::PositionTarget::IGNORE_YAW_RATE |
//                           // mavros_msgs::msg::PositionTarget::IGNORE_YAW;
//     // mavros_msgs::msg::PositionTarget::FORCE; >> WARN  [mavlink] SET_POSITION_TARGET_LOCAL_NED force not supported
//     local_msg.velocity.x = local_velocity_[0];
//     local_msg.velocity.y = local_velocity_[1];
//     local_msg.velocity.z = local_velocity_[2];
//     // local_msg.acceleration_or_force.x = local_velocity_[0]; // e
//     // local_msg.acceleration_or_force.y = local_velocity_[1]; // n
//     // local_msg.acceleration_or_force.z = local_velocity_[2]; // u
//     local_msg.yaw = local_velocity_[4];
//     local_msg.yaw_rate = local_velocity_[5];
//     local_pub->publish(local_msg);
//     // target_local_pub->publish(local_msg);
// }
//
void OffboardMavros::publish_local(void) {
    std::cout << "Publishing local..." << std::endl;
    mavros_msgs::msg::PositionTarget local_msg;
    local_msg.header.stamp = this->now();
    local_msg.header.frame_id = "standard_vtol_0";
    // local_msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_LOCAL_NED;
    local_msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_BODY_NED;
    local_msg.type_mask = mavros_msgs::msg::PositionTarget::IGNORE_PX	|
                          mavros_msgs::msg::PositionTarget::IGNORE_PY	|
                          mavros_msgs::msg::PositionTarget::IGNORE_PZ	|
                          mavros_msgs::msg::PositionTarget::IGNORE_AFX	|
                          mavros_msgs::msg::PositionTarget::IGNORE_AFY	|
                          mavros_msgs::msg::PositionTarget::IGNORE_AFZ  ;
    local_msg.velocity.x    = local_velocity_[0]; // East
    local_msg.velocity.y    = local_velocity_[1]; // North
    local_msg.velocity.z    = local_velocity_[2]; // Up
    local_msg.yaw           = local_velocity_[4];
    local_msg.yaw_rate      = local_velocity_[5];
    local_pub->publish(local_msg);
}


/**
 * @brief Publish local velocity
 * fixed wing은 velocity로 제어가 불가능해 보임
 * 왜인지 모르겟찌만, 아래처럼 하면 가능하긴 함. 다만 늦음
 */
void OffboardMavros::publish_local_fixed(void) {
    std::cout << "Publishing local fixed..." << std::endl;
    mavros_msgs::msg::PositionTarget local_msg;
    local_msg.header.stamp = this->now();
    local_msg.header.frame_id = "standard_vtol_0";
    // local_msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_LOCAL_NED;
    local_msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_BODY_NED;
    local_msg.type_mask = \
                          // mavros_msgs::msg::PositionTarget::IGNORE_PX |
                          // mavros_msgs::msg::PositionTarget::IGNORE_PY |
                          // mavros_msgs::msg::PositionTarget::IGNORE_PZ |
                          mavros_msgs::msg::PositionTarget::IGNORE_AFX |
                          mavros_msgs::msg::PositionTarget::IGNORE_AFY |
                          mavros_msgs::msg::PositionTarget::IGNORE_AFZ ;
                          // mavros_msgs::msg::PositionTarget::IGNORE_YAW |
                          // mavros_msgs::msg::PositionTarget::IGNORE_YAW_RATE;
    local_msg.velocity.x = local_velocity_[0]; // East
    local_msg.velocity.y = local_velocity_[1]; // North
    local_msg.velocity.z = local_velocity_[2]; // Up
    local_msg.yaw = local_velocity_[4];
    local_msg.yaw_rate = local_velocity_[5];
    local_pub->publish(local_msg);
}

