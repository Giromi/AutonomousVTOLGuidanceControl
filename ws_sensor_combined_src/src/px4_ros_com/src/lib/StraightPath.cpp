#include "px4_ros_com/StraightPath.hpp"

// Constructor
StraightPath::StraightPath(const Eigen::Vector3d& start, const Eigen::Vector3d& goal, const Eigen::Matrix3d& k1, const Eigen::Matrix3d& k2)
    : Path(start, goal, k1, k2) {}

// Guidance method
Eigen::Vector3d StraightPath::guidance_control(const Eigen::Vector3d& UAV_position, float UAV_speed) {
    // Combine the results from path_following and path_traveling
    Eigen::Vector3d follow_result = path_following(UAV_position);
    Eigen::Vector3d travel_result = path_traveling(UAV_position);
    Eigen::Vector3d u_prime = - K1 * follow_result + K2 * travel_result;

    // Normalize and scale by UAV speed
    return UAV_speed * u_prime / u_prime.norm();
}

bool StraightPath::is_arrived(const Eigen::Vector3d& UAV_position, float threshold) {
    // Calculate the distance to the goal
    double distance_to_goal = (goal_point - UAV_position).norm();

    // Check if the distance is within the threshold
    return distance_to_goal <= threshold;
}

// Calculation methods
Eigen::Vector3d StraightPath::path_traveling(const Eigen::Vector3d& UAV_position) {
    // Calculate the longitudinal and lateral manifolds
    Eigen::Vector3d partial_lon = calculate_lon_manifold();
    Eigen::Vector3d partial_lat = calculate_lat_manifold();

    // Calculate the traveling path
    Eigen::Vector3d follow_result = UAV_position - start_point;
    return follow_result.cwiseProduct(partial_lon) + follow_result.cwiseProduct(partial_lat);
}

Eigen::Vector3d StraightPath::path_following(const Eigen::Vector3d& UAV_position) {
    // Normalize tilde for the following path
    return tilde / tilde.norm();
}

Eigen::Vector3d StraightPath::calculate_lon_manifold() {
    // Calculate the longitudinal manifold
    Eigen::Vector3d direction_z(0, 0, 1);
    Eigen::Vector3d partial_lon = direction_z.cross(tilde);
    return partial_lon.normalized();
}

Eigen::Vector3d StraightPath::calculate_lat_manifold() {
    // Calculate the lateral manifold
    Eigen::Vector3d partial_lon = calculate_lon_manifold();
    Eigen::Vector3d partial_lat = tilde.cross(partial_lon);
    return partial_lat.normalized();
}
