#ifndef PATH_H
#define PATH_H

#include <Eigen/Core>

class Traj
{
public:
    // Constructor
    Traj(const Eigen::Vector3d &start, const Eigen::Vector3d &goal, const Eigen::Matrix3d &k1, const Eigen::Matrix3d &k2);

    // Setter methods
    virtual void set_path(const Eigen::Vector3d &start, const Eigen::Vector3d &goal);
    void set_K1(const Eigen::Matrix3d &k1);
    void set_K2(const Eigen::Matrix3d &k2);

    // Guidance method
    virtual Eigen::Vector3d guidance_control(const Eigen::Vector3d &UAV_position, float UAV_speed) = 0;
    virtual bool is_arrived(const Eigen::Vector3d &UAV_position, float threshold) = 0;

protected:
    // Calculation methods
    virtual Eigen::Vector3d path_traveling(const Eigen::Vector3d &UAV_position) = 0;
    virtual Eigen::Vector3d path_following(const Eigen::Vector3d &UAV_position) = 0;
    virtual Eigen::Vector3d calculate_lon_manifold() = 0;
    virtual Eigen::Vector3d calculate_lat_manifold() = 0;

    // Member variables
    Eigen::Vector3d start_point;
    Eigen::Vector3d goal_point;
    Eigen::Vector3d tilde;
    Eigen::Matrix3d K1;
    Eigen::Matrix3d K2;
};

#endif // Path_H
