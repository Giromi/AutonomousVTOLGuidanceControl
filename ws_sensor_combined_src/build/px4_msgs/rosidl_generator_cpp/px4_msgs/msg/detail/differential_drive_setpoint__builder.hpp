// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from px4_msgs:msg/DifferentialDriveSetpoint.idl
// generated code does not contain a copyright notice

#ifndef PX4_MSGS__MSG__DETAIL__DIFFERENTIAL_DRIVE_SETPOINT__BUILDER_HPP_
#define PX4_MSGS__MSG__DETAIL__DIFFERENTIAL_DRIVE_SETPOINT__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "px4_msgs/msg/detail/differential_drive_setpoint__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace px4_msgs
{

namespace msg
{

namespace builder
{

class Init_DifferentialDriveSetpoint_yaw_rate
{
public:
  explicit Init_DifferentialDriveSetpoint_yaw_rate(::px4_msgs::msg::DifferentialDriveSetpoint & msg)
  : msg_(msg)
  {}
  ::px4_msgs::msg::DifferentialDriveSetpoint yaw_rate(::px4_msgs::msg::DifferentialDriveSetpoint::_yaw_rate_type arg)
  {
    msg_.yaw_rate = std::move(arg);
    return std::move(msg_);
  }

private:
  ::px4_msgs::msg::DifferentialDriveSetpoint msg_;
};

class Init_DifferentialDriveSetpoint_speed
{
public:
  explicit Init_DifferentialDriveSetpoint_speed(::px4_msgs::msg::DifferentialDriveSetpoint & msg)
  : msg_(msg)
  {}
  Init_DifferentialDriveSetpoint_yaw_rate speed(::px4_msgs::msg::DifferentialDriveSetpoint::_speed_type arg)
  {
    msg_.speed = std::move(arg);
    return Init_DifferentialDriveSetpoint_yaw_rate(msg_);
  }

private:
  ::px4_msgs::msg::DifferentialDriveSetpoint msg_;
};

class Init_DifferentialDriveSetpoint_timestamp
{
public:
  Init_DifferentialDriveSetpoint_timestamp()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_DifferentialDriveSetpoint_speed timestamp(::px4_msgs::msg::DifferentialDriveSetpoint::_timestamp_type arg)
  {
    msg_.timestamp = std::move(arg);
    return Init_DifferentialDriveSetpoint_speed(msg_);
  }

private:
  ::px4_msgs::msg::DifferentialDriveSetpoint msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::px4_msgs::msg::DifferentialDriveSetpoint>()
{
  return px4_msgs::msg::builder::Init_DifferentialDriveSetpoint_timestamp();
}

}  // namespace px4_msgs

#endif  // PX4_MSGS__MSG__DETAIL__DIFFERENTIAL_DRIVE_SETPOINT__BUILDER_HPP_
