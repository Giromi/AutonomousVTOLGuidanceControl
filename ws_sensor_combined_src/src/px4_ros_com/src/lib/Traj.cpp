#include "px4_ros_com/Traj.hpp"

// Constructor
Traj::Traj(const Eigen::Vector3d &start, const Eigen::Vector3d &goal)
    : start_point(start), goal_point(goal), tilde(goal - start) { 
    }

// Setter methods
void Traj::setPath(const Eigen::Vector3d &start, const Eigen::Vector3d &goal)
{
    this->start_point = start;
    this->goal_point = goal;
    this->tilde = goal_point - start_point;
}

void Traj::setK1(const Eigen::Matrix3d &k1)
{
    this->K1 = k1;
}

void Traj::setK2(const Eigen::Matrix3d &k2)
{
    this->K2 = k2;
}

Eigen::Vector3d Traj::guidanceControl(const Eigen::Vector3d &UAV_position, float UAV_speed) {
    static_cast<void>(UAV_position);
    static_cast<void>(UAV_speed);
    return Eigen::Vector3d::Zero();
}

double Traj::headingControl(const Eigen::Vector3d &u_prime) {
    static_cast<void>(u_prime);
    return 0.0;
}

bool Traj::isArrived(const Eigen::Vector3d &UAV_position, float threshold) {
    static_cast<void>(UAV_position);
    static_cast<void>(threshold);
    return false;
}

Eigen::Vector3d Traj::pathTraveling(const Eigen::Vector3d &UAV_position){
    static_cast<void>(UAV_position);
    return Eigen::Vector3d::Zero();
}

Eigen::Vector3d Traj::pathFollowing(const Eigen::Vector3d &UAV_position){
    static_cast<void>(UAV_position);
    return Eigen::Vector3d::Zero();
}

Eigen::Vector3d Traj::calculateLonManifold(void){
    return Eigen::Vector3d::Zero();
}

Eigen::Vector3d Traj::calculateLatManifold(void){
    return Eigen::Vector3d::Zero();
}
