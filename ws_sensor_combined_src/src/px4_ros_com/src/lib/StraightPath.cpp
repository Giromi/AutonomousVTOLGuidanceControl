#include "px4_ros_com/StraightPath.hpp"

// Constructor
StraightPath::StraightPath(const Eigen::Vector3d &start, const Eigen::Vector3d &goal)
    : Traj(start, goal)
{
    this->K1 << 0.5, 0.0, 0.0,
        0.0, 0.5, 0.0,
        0.0, 0.0, 0.5;

    this->K2 << 1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0;
}

// Guidance method
Eigen::Vector3d StraightPath::guidanceControl(const Eigen::Vector3d &UAV_position, float UAV_speed)
{
    // Combine the results from pathFollowing and pathTraveling
    Eigen::Vector3d follow_result = pathFollowing(UAV_position);
    Eigen::Vector3d travel_result = pathTraveling(UAV_position);
    Eigen::Vector3d u_prime = -K1 * follow_result + K2 * travel_result;

    // Normalize and scale by UAV speed
    return UAV_speed * u_prime.normalized();
}

double StraightPath::headingControl(const Eigen::Vector3d &u_prime){
    static_cast<void>(u_prime);
    return 0.0;
}

bool StraightPath::isArrived(const Eigen::Vector3d &UAV_position, float threshold)
{
    // set the manifold for arrival classification
    Eigen::Vector3d norm_vec = (goal_point - start_point).normalized();

    // Check if the distance is within the threshold
    return norm_vec.dot(UAV_position - goal_point) >= 0 || (UAV_position- goal_point).norm() < threshold;
}

// Calculation methods
Eigen::Vector3d StraightPath::pathFollowing(const Eigen::Vector3d &UAV_position)
{
    // Calculate the longitudinal and lateral manifolds
    Eigen::Vector3d partial_lon = calculateLonManifold();
    Eigen::Vector3d partial_lat = calculateLatManifold();

    // Calculate the traveling path
    Eigen::Vector3d follow_result = UAV_position - start_point;
    double lon_alpha = follow_result.dot(partial_lon);
    double lat_alpha = follow_result.dot(partial_lat);
    return lon_alpha * partial_lon + lat_alpha * partial_lat;
}

Eigen::Vector3d StraightPath::pathTraveling(const Eigen::Vector3d &UAV_position)
{
    static_cast<void>(UAV_position);
    // Normalize tilde for the following path
    return tilde.normalized();
}

Eigen::Vector3d StraightPath::calculateLonManifold()
{
    // Calculate the longitudinal manifold
    Eigen::Vector3d direction_z(0.0, 0.0, 1.0);
    Eigen::Vector3d partial_lon = direction_z.cross(tilde);
    DEBUG::print("Partial longitudinal : ", partial_lon.normalized(), BLUE);
    return partial_lon.normalized();
}

Eigen::Vector3d StraightPath::calculateLatManifold()
{
    // Calculate the lateral manifold
    Eigen::Vector3d partial_lon = calculateLonManifold();
    Eigen::Vector3d partial_lat = tilde.cross(partial_lon);
    return partial_lat.normalized();
}
