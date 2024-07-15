#ifndef WAYPOINTMANAGER_HPP
# define WAYPOINTMANAGER_HPP

#include <rclcpp/rclcpp.hpp>
#include <iostream>
#include <queue>
#include <array>
#include <Eigen/Dense>
#include "DEBUG.hpp"

struct ReferenceWaypoint {
    float x;
    float y;
    float z;
    float yaw;

    ReferenceWaypoint(float x, float y, float z, float yaw=0.0f) 
        : x(x), y(y), z(z), yaw(yaw) { }
};

template <typename T>
class WaypointManager {
public:
    WaypointManager(void);
    template <std::size_t N>
    void        setPath(const std::array<T, N> & waypoints);
    void        printWaypoints(void);
    bool        isArrived(const T& current_position) const;
    const T&    getTarget(void) const;
    void        pop(void);

    unsigned int size(void) const;

private:
    std::queue<T> waypoint_path;
};

#include "WaypointManager.tpp"


#endif // WAYPOINTMANAGER_H
