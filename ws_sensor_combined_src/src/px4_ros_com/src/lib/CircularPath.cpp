#include "px4_ros_com/CircularPath.hpp"

// Constructor
CircularPath::CircularPath(const Eigen::Vector3d &start, const Eigen::Vector3d &goal, const Eigen::Matrix3d &k1, const Eigen::Matrix3d &k2, const Eigen::Vector3d &center, double radius, bool rotation_dir)
    : Path(start, goal, k1, k2), center_point(center), radius(radius), rotation_dir(rotation_dir) {}

void CircularPath::set_path(const Eigen::Vector3d& start, const Eigen::Vector3d& goal, Eigen::Vector3d center, double radius, bool rotation_dir) {
    this->start_point = start;
    this->goal_point = goal;
    this->tilde = goal - start;
    this->center_point = center;
    this->radius = radius;
    this->rotation_dir = rotation_dir;
}

Eigen::Vector3d CircularPath::guidance_control(const Eigen::Vector3d &UAV_position, float UAV_speed){
    // Combine the results from path_following and path_traveling
    Eigen::Vector3d follow_result = path_following(UAV_position);
    Eigen::Vector3d travel_result = path_traveling(UAV_position);
    Eigen::Vector3d u_prime = - K1 * follow_result + K2 * travel_result;

    // Normalize and scale by UAV speed
    return UAV_speed * u_prime / u_prime.norm();
}

Eigen::Vector3d CircularPath::path_following(const Eigen::Vector3d &UAV_position)
{
    Eigen::Vector3d partial_lat = calculate_lat_manifold();
    Eigen::Vector3d partial_lon = calculate_lon_manifold(UAV_position);
    Eigen::Vector3d follow_result = UAV_position - center_point;
    double scale_lon = pow(follow_result.x(), 2) + pow(follow_result.y(), 2) - pow(radius, 2);
    double scale_lat = follow_result.z();
    return scale_lon * partial_lon + scale_lat * partial_lat;
}

Eigen::Vector3d CircularPath::path_traveling(const Eigen::Vector3d &UAV_position)
{
    Eigen::Vector3d partial_lat = calculate_lat_manifold();
    Eigen::Vector3d partial_lon = calculate_lon_manifold(UAV_position);
    if (rotation_dir)
    {
        // CCW rotation guidance
        return partial_lat.cross(partial_lon);
    }
    else
    {
        // CW rotation guidance
        return partial_lon.cross(partial_lat);
    }
}

Eigen::Vector3d CircularPath::calculate_lat_manifold()
{
    Eigen::Vector3d partial_lat(0.0, 0.0, 1.0);
    return partial_lat;
}

Eigen::Vector3d CircularPath::calculate_lon_manifold(const Eigen::Vector3d &UAV_position)
{
    Eigen::Vector3d partial_lon = UAV_position - center_point;
    partial_lon.z() = 0.0;
    return partial_lon * 2;
}
