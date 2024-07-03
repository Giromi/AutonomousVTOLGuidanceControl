#include "px4_ros_com/OffboardMavros.hpp"


/* -- Publish Functions -- */
void    OffboardMavros::publish(void) {
    if (_cmd_flag != vtol::START) {
        return ;
    }
    // publishPose();
    publishLocal();
    publish_local_fixed();
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
    geometry_msgs::msg::TwistStamped vel;
    vel.twist.linear.x = _local_velocity[0];
    vel.twist.linear.y = _local_velocity[1];
    vel.twist.linear.z = _local_velocity[2];
    vel.twist.angular.x = _local_velocity[3];
    vel.twist.angular.y = _local_velocity[4];
    vel.twist.angular.z = _local_velocity[5];
    
    local_vel_pub->publish(vel);
}

void OffboardMavros::publishAttitude(void) {
    geometry_msgs::msg::TwistStamped att;
    att.twist.linear.x = _local_velocity[0];
    att.twist.linear.y = _local_velocity[1];
    att.twist.linear.z = _local_velocity[2];
    att.twist.angular.x = _local_velocity[3];
    att.twist.angular.y = _local_velocity[4];
    att.twist.angular.z = _local_velocity[5];
    att_pub->publish(att);
}


void OffboardMavros::publishLocal(void) {
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
    local_msg.yaw = _local_velocity[4];
    local_msg.yaw_rate = _local_velocity[5];

    local_pub->publish(local_msg);
}

