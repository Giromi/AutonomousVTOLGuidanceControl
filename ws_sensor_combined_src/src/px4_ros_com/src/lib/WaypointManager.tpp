#ifndef WAYPOINT_MANAGER_TPP
#define WAYPOINT_MANAGER_TPP

#include "px4_ros_com/WaypointManager.hpp"

template <typename T>
WaypointManager<T>::WaypointManager(void) { }

template <typename T>
void WaypointManager<T>::printWaypoints(void) {
    std::queue<T> temp = waypoint_path;
    while (temp.size()) {
        std::cout << "Waypoint: " << temp.front().transpose() << std::endl;
        temp.pop();
    }
}

template <typename T>
bool WaypointManager<T>::isArrived(const T& current_position) const {
    if (waypoint_path.empty()) {
        return false;
    }

    const Eigen::Vector3d cur_position_vector3d = current_position.template head<3>();
    const Eigen::Vector3d position_diff = \
                                  current_position.template head<3>() 
                                    - waypoint_path.front().template head<3>();
    DEBUG::print("Target  position : ", getTarget().transpose(), BOLDGREEN);
    DEBUG::print("Current position : ", cur_position_vector3d.transpose(), BOLDGREEN);
    DEBUG::print("Position diff: ", position_diff.norm(), BOLDGREEN);
    DEBUG::print("Waypoint size    : ", getSize(), BOLDGREEN);
    if (position_diff.norm() < 10.0) {
        return true;
    }
    return false;
}

template <typename T>
template <std::size_t N>
void WaypointManager<T>::setPath(const std::array<T, N>& waypoints) {
    for(const T& waypoint : waypoints) {
        waypoint_path.push(waypoint);
    }
}

template <typename T>
unsigned int WaypointManager<T>::getSize(void) const {
    return waypoint_path.size();
}

template <typename T>
const T& WaypointManager<T>::getTarget(void) const {
    return waypoint_path.front();
}

template <typename T>
void WaypointManager<T>::pop(void) {
    waypoint_path.pop();
}

template <typename T>
bool WaypointManager<T>::empty(void) const {
    return waypoint_path.empty();
}

#endif // WAYPOINT_MANAGER_TPP
