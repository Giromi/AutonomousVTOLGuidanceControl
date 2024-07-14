#include "px4_ros_com/OffboardMavros.hpp"


/* -- Publish Functions -- */
void    OffboardMavros::publish(void) {
    if (_cmd_flag == vtol::INIT) {
        publishGpOrigin();
    }
    if (_cmd_flag == vtol::MC_START || _cmd_flag == vtol::FW_START) {
        return ;
    } 

    // publishPose();
    publishLocalRaw();
    // if (fcu_state.mode == "AUTO.MISSION") {
    // publishWaypoint();
    // } else {
    //     publishLocal(); // publishLocalFixed();
    // }
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
    // DEBUG::print("publishing origin",     mavros_msgs::msg::State::MODE_PX4_MANUAL, BOLDGREEN);
;
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

void OffboardMavros::publishLocal(void) {
    std::cout << "Publishing local..." << std::endl;
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
    // local_msg.velocity.x    = _local_velocity[0]; // East
    // local_msg.velocity.y    = _local_velocity[1]; // North
    // local_msg.velocity.z    = _local_velocity[2]; // Up
    local_msg.yaw           = _local_velocity[4];
    local_msg.yaw_rate      = _local_velocity[5];
    local_pub->publish(local_msg);
}


/**
 * @brief Publish local velocity
 * 
 */
void OffboardMavros::publishLocalRaw(void) {
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
                        ? mavros_msgs::msg::PositionTarget::IGNORE_YAW : 0;

    DEBUG::print("type mask: ", msg->type_mask, BOLDWHITE);
    msg->position.x = wp_manager.getTarget()[vtol::EAST]; // East
    msg->position.y = wp_manager.getTarget()[vtol::NORTH]; // North
    msg->position.z = wp_manager.getTarget()[vtol::UP]; // Up
    msg->yaw        = static_cast<float>(wp_manager.getTarget()[vtol::YAW]);
    local_pub->publish(*msg);
}

