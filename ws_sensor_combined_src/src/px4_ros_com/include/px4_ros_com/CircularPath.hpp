#ifndef CIRCULARPATH_H
#define CIRCULARPATH_H

#include "px4_ros_com/Traj.hpp"
#include <cmath>

class CircularPath : public Traj {
public:
    // Constructor
    CircularPath(const Eigen::Vector3d& start, const Eigen::Vector3d& goal, const Eigen::Matrix3d& k1, const Eigen::Matrix3d& k2, const Eigen::Vector3d& center, double radius, bool rotation_dir);

    // Setter methods
    void set_path(const Eigen::Vector3d& start, const Eigen::Vector3d& goal, Eigen::Vector3d center, double radius, bool rotation_dir);

    // Guidance method
    Eigen::Vector3d guidance_control(const Eigen::Vector3d &UAV_position, float UAV_speed);

protected:
    // Calculation methods
    Eigen::Vector3d path_traveling(const Eigen::Vector3d& UAV_position);
    Eigen::Vector3d path_following(const Eigen::Vector3d& UAV_position);
    Eigen::Vector3d calculate_lat_manifold();
    Eigen::Vector3d calculate_lon_manifold(const Eigen::Vector3d& UAV_position);

    // Member variables
    Eigen::Vector3d center_point;
    double radius;
    bool rotation_dir;    // false for CW / true for CCW
};

#endif // CIRCULARPATH_H
