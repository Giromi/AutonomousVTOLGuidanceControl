#ifndef TRAJ_H
#define TRAJ_H

#include <Eigen/Dense>
#include "DEBUG.hpp"


class Traj
{
public:
    // Constructor
    Traj(const Eigen::Vector3d &start, const Eigen::Vector3d &goal);

    // Setter methods
    virtual void setPath(const Eigen::Vector3d &start, const Eigen::Vector3d &goal);
    void setK1(const Eigen::Matrix3d &k1);
    void setK2(const Eigen::Matrix3d &k2);

    // Guidance method
    virtual Eigen::Vector3d guidanceControl(const Eigen::Vector3d &UAV_position, float UAV_speed);
    virtual double headingControl(const Eigen::Vector3d &u_prime);
    virtual bool isArrived(const Eigen::Vector3d &UAV_position, float threshold);

protected:
    // Calculation methods
    virtual Eigen::Vector3d pathTraveling(const Eigen::Vector3d &UAV_position);
    virtual Eigen::Vector3d pathFollowing(const Eigen::Vector3d &UAV_position);
    virtual Eigen::Vector3d calculateLonManifold(void);
    virtual Eigen::Vector3d calculateLatManifold(void);

    // Member variables
    Eigen::Vector3d start_point;
    Eigen::Vector3d goal_point;
    Eigen::Vector3d tilde;
    Eigen::Matrix3d K1;
    Eigen::Matrix3d K2;
};

#endif
