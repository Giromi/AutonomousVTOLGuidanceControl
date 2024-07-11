#include "px4_ros_com/Traj.hpp"

// Constructor
Traj::Traj(const Eigen::Vector3d &start, const Eigen::Vector3d &goal, const Eigen::Matrix3d &k1, const Eigen::Matrix3d &k2)
    : start_point(start), goal_point(goal), tilde(goal - start), K1(k1), K2(k2) {}

// Setter methods
void Traj::set_path(const Eigen::Vector3d &start, const Eigen::Vector3d &goal)
{
    this->start_point = start;
    this->goal_point = goal;
    this->tilde = goal_point - start_point;
}

void Traj::set_K1(const Eigen::Matrix3d &k1)
{
    this->K1 = k1;
}

void Traj::set_K2(const Eigen::Matrix3d &k2)
{
    this->K2 = k2;
}
