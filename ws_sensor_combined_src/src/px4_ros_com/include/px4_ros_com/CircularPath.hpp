#ifndef CIRCULARPATH_H
#define CIRCULARPATH_H

#include "px4_ros_com/Traj.hpp"
#include <cmath>

class CircularPath : public Traj
{
public:
    // Constructor
    CircularPath(const Eigen::Vector3d &start, const Eigen::Vector3d &goal, const Eigen::Vector3d &center, double radius, bool rotation_dir);
    ~CircularPath(void);

    // Setter methods
    void setPath(const Eigen::Vector3d &start, const Eigen::Vector3d &goal, Eigen::Vector3d center, double radius, bool rotation_dir);

    // Guidance method
    Eigen::Vector3d guidanceControl(const Eigen::Vector3d &UAV_position, float UAV_speed);
    double headingControl(const Eigen::Vector3d &u_prime);

    bool isArrived(const Eigen::Vector3d &UAV_position, float threshold);

protected:
    // Calculation methods
    Eigen::Vector3d pathTraveling(const Eigen::Vector3d &UAV_position);
    Eigen::Vector3d pathFollowing(const Eigen::Vector3d &UAV_position);
    Eigen::Vector3d calculateLatManifold();
    Eigen::Vector3d calculateLonManifold(const Eigen::Vector3d &UAV_position);
    double requiredRadian();

    // Member variables
    Eigen::Vector3d center_point;
    double radius;
    bool rotation_dir; // false for CW / true for CCW
};

#endif // CIRCULARPATH_H
