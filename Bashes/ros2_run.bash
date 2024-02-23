#!/bin/bash 


# px4_ros_com 빌드하는 법
colcon build --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=ON --packages-select px4_ros_com && source install/setup.bash

# px4_ros_com 실행하는 법
ros2 run px4_ros_com (tab 누르면 실행할 수 있는 명령어들이 나옴)


