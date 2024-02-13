cmake_minimum_required(VERSION 3.8)
project(key_event)

# Default to C99
if(NOT CMAKE_C_STANDARD)
	set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()


# find dependencies
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)


add_executable(key_event src/key_event.cpp)
add_executable(key_sub src/key_sub.cpp)
add_executable(key_pub src/key_pub.cpp)
ament_target_dependencies(key_event rclcpp geometry_msgs)
ament_target_dependencies(key_sub rclcpp std_msgs)
ament_target_dependencies(key_pub rclcpp std_msgs)
install(TARGETS key_event DESTINATION lib/${PROJECT_NAME})
install(TARGETS key_sub DESTINATION lib/${PROJECT_NAME})
install(TARGETS key_pub DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
