#include "px4_ros_com/CircularPath.hpp"

// Constructor
CircularPath::CircularPath(const Eigen::Vector3d &start, const Eigen::Vector3d &goal, const Eigen::Vector3d &center, double radius, bool rotation_dir)
    : Traj(start, goal), center_point(center), radius(radius), rotation_dir(rotation_dir)
{
    this->K1 << 0.001, 0.0, 0.0,
        0.0, 0.001, 0.0,
        0.0, 0.0, 0.001;

    this->K2 << 1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0;
}

CircularPath::~CircularPath(void) { }

void CircularPath::setPath(const Eigen::Vector3d &start, const Eigen::Vector3d &goal, Eigen::Vector3d center, double radius, bool rotation_dir)
{
    this->start_point = start;
    this->goal_point = goal;
    this->tilde = goal - start;
    this->center_point = center;
    this->radius = radius;
    this->rotation_dir = rotation_dir;
}

Eigen::Vector3d CircularPath::guidanceControl(const Eigen::Vector3d &UAV_position, float UAV_speed)
{
    // Combine the results from pathFollowing and pathTraveling
    Eigen::Vector3d follow_result = pathFollowing(UAV_position);
    Eigen::Vector3d travel_result = pathTraveling(UAV_position);
    Eigen::Vector3d u_prime = -K1 * follow_result + K2 * travel_result;

    // Normalize and scale by UAV speed
    return UAV_speed * u_prime.normalized();
}

double CircularPath::headingControl(const Eigen::Vector3d &u_prime){
    static_cast<void>(u_prime);
    return 0.0;
}

bool CircularPath::isArrived(const Eigen::Vector3d &UAV_position, float threshold)
{
    // static_cast<void>(threshold);
    // Eigen::Vector3d dir_start = (start_point - center_point).normalized();
    // Eigen::Vector3d dir_goal = (goal_point - center_point).normalized();

    // double rad_start = atan2(dir_start(1), dir_start(0));
    // double rad_goal = atan2(dir_goal(1), dir_goal(0));

    // double req_rad = rad_goal - rad_start;

    // Eigen::Vector3d dir_UAV = (UAV_position - center_point).normalized();
    // double rad_UAV = atan2(dir_UAV(1), dir_UAV(0));
    // double tilde_rad = rad_UAV - rad_start;

    // if (rotation_dir)   // CCW Circular Path
    // {
    //     if (rad_start > rad_goal) req_rad += 2 * M_PI;
    //     if (rad_start > rad_UAV) tilde_rad += 2 * M_PI;
    //     return tilde_rad < req_rad;
    // }
    // else                // CW Circular Path
    // {   
    //     if (rad_goal > rad_start) req_rad -= 2 * M_PI;
    //     if (rad_UAV > rad_start) tilde_rad -= 2 * M_PI;
    //     return tilde_rad > req_rad;
    // }
    Eigen::Vector3d norm_vec = (goal_point - start_point).normalized();
    static_cast<void>(threshold);

    // Check if the distance is within the threshold
    // return norm_vec.dot(UAV_position - goal_point) >= 0 || (UAV_position- goal_point).norm() < threshold;
    return norm_vec.dot(UAV_position - goal_point) >= 0;
}

Eigen::Vector3d CircularPath::pathFollowing(const Eigen::Vector3d &UAV_position)
{
    Eigen::Vector3d partial_lat = calculateLatManifold();
    Eigen::Vector3d partial_lon = calculateLonManifold(UAV_position);
    Eigen::Vector3d follow_result = UAV_position - center_point;
    double scale_lon = pow(follow_result.x(), 2) + pow(follow_result.y(), 2) - pow(radius, 2);
    double scale_lat = follow_result.z();
    return scale_lon * partial_lon + scale_lat * partial_lat;
}

Eigen::Vector3d CircularPath::pathTraveling(const Eigen::Vector3d &UAV_position)
{
    Eigen::Vector3d partial_lat = calculateLatManifold();
    Eigen::Vector3d partial_lon = calculateLonManifold(UAV_position);
    if (rotation_dir)
    {
        // CCW rotation guidance
        return (partial_lat.cross(partial_lon)).normalized();
    }
    else
    {
        // CW rotation guidance
        return (partial_lon.cross(partial_lat)).normalized();
    }
}

Eigen::Vector3d CircularPath::calculateLatManifold()
{
    Eigen::Vector3d partial_lat(0.0, 0.0, 1.0);
    return partial_lat;
}

Eigen::Vector3d CircularPath::calculateLonManifold(const Eigen::Vector3d &UAV_position)
{
    Eigen::Vector3d tilde = UAV_position - center_point;
    Eigen::Vector3d partial_lon(2*tilde.x(), 2*tilde.y(), 0.0);
    return partial_lon;
}
