

# 실행하는 법

### 빌드 명령어
```shell
cd ws_sensor_combined_src
colcon build
souce install/setup.bash

```
처음 zip파일을 다운 받았을 때 위와같이 입력합니다.


#### 특정 패키지 빌드 명령어

```
colcon build --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=ON --packages-select px4_ros_com
source install/setup.bash

```
모든 패키지를 빌드하기에는 번거로우니 위와 같이 입력합니다.


#### Header 파일 자동 인식
```shell

colcon build --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=ON --packages-select px4_ros_com
source install/setup.bash
```
cmake 인자로  `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`를 추가해줍니다.
