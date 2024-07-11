#ifndef STRAIGHTPATH_H
#define STRAIGHTPATH_H

#include "px4_ros_com/Traj.hpp"

class StraightPath : public Traj {
public:
    // Constructor
    StraightPath(const Eigen::Vector3d& start, const Eigen::Vector3d& goal, const Eigen::Matrix3d& k1, const Eigen::Matrix3d& k2);

    // Guidance method
    Eigen::Vector3d guidance_control(const Eigen::Vector3d& UAV_position, float UAV_speed);
    bool is_arrived(const Eigen::Vector3d& UAV_position, float threshold);

protected:
    // Calculation methods
    Eigen::Vector3d path_traveling(const Eigen::Vector3d& UAV_position);
    Eigen::Vector3d path_following(const Eigen::Vector3d& UAV_position);
    Eigen::Vector3d calculate_lon_manifold();
    Eigen::Vector3d calculate_lat_manifold();
};

#endif // STRAIGHTPATH_H