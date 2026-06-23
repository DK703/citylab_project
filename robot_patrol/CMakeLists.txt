cmake_minimum_required(VERSION 3.8)
project(robot_patrol)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(custom_interfaces REQUIRED)

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

add_executable(robot_patrol_executable src/patrol.cpp)
ament_target_dependencies(robot_patrol_executable rclcpp geometry_msgs sensor_msgs custom_interfaces)

install(TARGETS robot_patrol_executable DESTINATION lib/${PROJECT_NAME})

add_executable(robot_patrol_executable2 src/patrol_with_service.cpp)
ament_target_dependencies(robot_patrol_executable2 rclcpp geometry_msgs sensor_msgs custom_interfaces)

install(TARGETS robot_patrol_executable2 DESTINATION lib/${PROJECT_NAME})


add_executable(direction_service src/direction_service.cpp)
ament_target_dependencies(direction_service rclcpp geometry_msgs sensor_msgs custom_interfaces)
install(TARGETS direction_service DESTINATION lib/${PROJECT_NAME})

add_executable(test_service src/test_service.cpp)
ament_target_dependencies(test_service rclcpp geometry_msgs sensor_msgs custom_interfaces)
install(TARGETS test_service DESTINATION lib/${PROJECT_NAME})


#add_executable(rviz_executable src/rviz.cpp)
#ament_target_dependencies(rviz_executable rclcpp geometry_msgs sensor_msgs OpenCV)
#target_include_directories(rviz_executable PRIVATE include)
#install(TARGETS rviz_executable DESTINATION lib/${PROJECT_NAME})

# ADD THESE for custom service generation
find_package(rosidl_default_generators REQUIRED)

#rosidl_generate_interfaces(${PROJECT_NAME}
 # "srv/GetDirection.srv"
 # DEPENDENCIES sensor_msgs  # if GetDirection.srv uses sensor_msgs types
#)

#launch file

install(DIRECTORY launch rivz
  DESTINATION share/${PROJECT_NAME}
)
install(DIRECTORY rivz/
  DESTINATION share/${PROJECT_NAME}/rivz
)
ament_package()
