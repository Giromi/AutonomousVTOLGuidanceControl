
# ros2 pub
ros2 topic pub /mavros/actuator_control mavros_msgs/msg/ActuatorControl "
{header: {stamp: {sec: 0}, frame_id: ''},
group_mix: 2,
controls: [0.5, 0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0]}"



# 한번에 실행하기
colcon build --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=ON --packages-select px4_ros_com && source install/setup.bash && ros2 run px4_ros_com pwm_aux_output


# 잘 안됨
ros2 service call /mavros/cmd/command mavros_msgs/srv/CommandLong "{broadcast: true, command: 183, confirmation: 0, param1: 1.0, param2: 1000.0, param3: 0.0, param4: 0.0, param5: 0.0, param6: 0.0, param7: 0.0}"



ros2 service call /mavros/set_mode mavros_msgs/srv/SetMode "{base_mode: 194, custom_mode: ''}"


ros2 launch mavros px4.launch fcu_url:=/dev/ttyACM0:57600


ros2 param set /mavros/param 'PWM_AUX_DIS1' 1700
