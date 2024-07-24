#ifndef STRAIGHTPATH_H
#define STRAIGHTPATH_H

#include "px4_ros_com/Traj.hpp"

class StraightPath : public Traj
{
public:
    // Constructor
    StraightPath(const Eigen::Vector3d &start, const Eigen::Vector3d &goal);
    ~StraightPath(void);
    // Guidance method
    Eigen::Vector3d guidanceControl(const Eigen::Vector3d &UAV_position, float UAV_speed);
    double headingControl(const Eigen::Vector3d &u_prime);
    bool isArrived(const Eigen::Vector3d &UAV_position, float threshold);

protected:
    // Calculation methods
    Eigen::Vector3d pathTraveling(const Eigen::Vector3d &UAV_position);
    Eigen::Vector3d pathFollowing(const Eigen::Vector3d &UAV_position);
    Eigen::Vector3d calculateLonManifold(void);
    Eigen::Vector3d calculateLatManifold(void);
};

#endif // STRAIGHTPATH_H