
# ros2 pub
ros2 topic pub /mavros/actuator_control mavros_msgs/msg/ActuatorControl "
{header: {stamp: {sec: 0}, frame_id: ''},
group_mix: 2,
controls: [0.5, 0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0]}"



# 한번에 실행하기
colcon build --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=ON --packages-select px4_ros_com && source install/setup.bash && ros2 run px4_ros_com pwm_aux_output


[UdpEndpoint GCS]
Mode = Normal
Address = 127.0.0.1
Port = 14550

