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

    publishRawLocal();
    // publishVelocity(); MC 모드에서 Yaw rate 제어는 PD 제어나 1차 LPF 적용 필요


    //publishRawLocalPosition();
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
    att_vel_pub->publish(*msg);
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
    auto msg = std::make_shared<geographic_msgs::msg::GeoPoseStamped>();
    msg->header.stamp = this->now();
    msg->header.frame_id = "standard_vtol_0";
    msg->pose.position.altitude = 47.6554;
    msg->pose.position.latitude = 47.3984;
    msg->pose.position.longitude = 8.54616;
    gp_origin_pub->publish(*msg);
    // DEBUG::print("publishing origin",     mavros_msgs::msg::State::MODE_PX4_MANUAL, BOLDGREEN);
}

void OffboardMavros::publishPose(void) {
    auto msg = std::make_shared<geometry_msgs::msg::PoseStamped>();
    msg->pose.position.x = _local_position[vtol::EAST];
    msg->pose.position.y = _local_position[vtol::NORTH];
    msg->pose.position.z = _local_position[vtol::UP];
    msg->pose.orientation.x = _local_velocity[3];
    msg->pose.orientation.y = _local_velocity[4];
    msg->pose.orientation.z = _local_velocity[5];
    local_pos_pub->publish(*msg);
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
                          mavros_msgs::msg::PositionTarget::IGNORE_AFZ |
                          mavros_msgs::msg::PositionTarget::IGNORE_YAW_RATE;

    msg->velocity.x    = _local_velocity[0]; // East
    msg->velocity.y    = _local_velocity[1]; // North
    msg->velocity.z    = _local_velocity[2]; // Up
    msg->yaw           = _local_velocity[4];
    // local_msg.yaw_rate      = _local_velocity[5];
    local_pub->publish(*msg);
}

/* Thrust NaN 넣어도 알아서 작동 안됨 */
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
    msg->orientation.w    =  q.w();      //
    msg->orientation.x    =  q.x();      // roll : 키보드 2, 3  (2 : 반시계, 3 : 시계)
    msg->orientation.y    =  q.y();      //
    msg->orientation.z    =  q.z();      // pitch : 키보드 4, 5 (4 : 하강,   5 : 상승)
    // msg->body_rate.x    =  _local_velocity[3];    
    // msg->body_rate.y    =  _local_velocity[4]; 
    // msg->body_rate.z    =  _local_velocity[5]; 
    msg->thrust = _local_velocity[2];
    raw_attitude_pub->publish(*msg);
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
