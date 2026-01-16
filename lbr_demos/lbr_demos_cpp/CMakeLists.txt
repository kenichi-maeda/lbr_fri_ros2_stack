cmake_minimum_required(VERSION 3.22)
project(lbr_demos_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(control_msgs REQUIRED)
find_package(FRIClient REQUIRED)
find_package(lbr_fri_idl REQUIRED)
find_package(lbr_fri_ros2 REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)

set(LBR_FRI_VERSIONS "1.11;1.14;1.15;1.16;2.5;2.7" CACHE STRING
  "FRI client SDK versions to build against")
set(LBR_FRI_DEFAULT_VERSION "1.15" CACHE STRING
  "Default FRI client SDK version for the unversioned target")

function(lbr_fri_select_target version out_var)
  string(REPLACE "." "_" version_tag "${version}")
  if(TARGET FRIClient::FRIClient_${version_tag})
    set(${out_var} "FRIClient::FRIClient_${version_tag}" PARENT_SCOPE)
  elseif(TARGET FRIClient::FRIClient)
    set(${out_var} "FRIClient::FRIClient" PARENT_SCOPE)
  else()
    set(${out_var} "" PARENT_SCOPE)
  endif()
endfunction()

string(REPLACE "." "_" LBR_FRI_DEFAULT_VERSION_TAG "${LBR_FRI_DEFAULT_VERSION}")
lbr_fri_select_target("${LBR_FRI_DEFAULT_VERSION}" LBR_FRI_DEFAULT_TARGET)
if(NOT LBR_FRI_DEFAULT_TARGET)
  message(FATAL_ERROR "No FRIClient target found for default version ${LBR_FRI_DEFAULT_VERSION}")
endif()

# joint sine overlay
add_executable(joint_sine_overlay
  src/joint_sine_overlay.cpp
)

ament_target_dependencies(joint_sine_overlay
  lbr_fri_idl
  lbr_fri_ros2
  rclcpp
)

target_link_libraries(joint_sine_overlay
  ${LBR_FRI_DEFAULT_TARGET}
)

# joint trajectory client
add_executable(joint_trajectory_client
  src/joint_trajectory_client.cpp
)

ament_target_dependencies(joint_trajectory_client
  control_msgs
  rclcpp
  rclcpp_action
  sensor_msgs
  trajectory_msgs
)

target_link_libraries(joint_trajectory_client
  ${LBR_FRI_DEFAULT_TARGET}
)

# torque sine overlay
add_executable(torque_sine_overlay
  src/torque_sine_overlay.cpp
)

ament_target_dependencies(torque_sine_overlay
  lbr_fri_idl
  lbr_fri_ros2
  rclcpp
)

target_link_libraries(torque_sine_overlay
  ${LBR_FRI_DEFAULT_TARGET}
)

# wrench sine overlay
add_executable(wrench_sine_overlay
  src/wrench_sine_overlay.cpp
)

ament_target_dependencies(wrench_sine_overlay
  lbr_fri_idl
  lbr_fri_ros2
  rclcpp
)

target_link_libraries(wrench_sine_overlay
  ${LBR_FRI_DEFAULT_TARGET}
)

set(LBR_DEMOS_CPP_VARIANT_TARGETS "")
foreach(version IN LISTS LBR_FRI_VERSIONS)
  string(REPLACE "." "_" version_tag "${version}")
  if(TARGET FRIClient::FRIClient_${version_tag})
    set(fri_target "FRIClient::FRIClient_${version_tag}")
  else()
    message(STATUS "Skipping lbr_demos_cpp variants for FRI ${version} (missing FRIClient target)")
    continue()
  endif()
  if(NOT TARGET lbr_fri_ros2_fri_${version_tag})
    message(STATUS "Skipping lbr_demos_cpp variants for FRI ${version} (missing lbr_fri_ros2_fri target)")
    continue()
  endif()

  add_executable(joint_sine_overlay_fri_${version_tag} src/joint_sine_overlay.cpp)
  ament_target_dependencies(joint_sine_overlay_fri_${version_tag}
    lbr_fri_idl
    lbr_fri_ros2
    rclcpp
  )
  target_link_libraries(joint_sine_overlay_fri_${version_tag}
    lbr_fri_ros2_fri_${version_tag}
    ${fri_target}
  )

  add_executable(joint_trajectory_client_fri_${version_tag} src/joint_trajectory_client.cpp)
  ament_target_dependencies(joint_trajectory_client_fri_${version_tag}
    control_msgs
    rclcpp
    rclcpp_action
    sensor_msgs
    trajectory_msgs
  )
  target_link_libraries(joint_trajectory_client_fri_${version_tag}
    ${fri_target}
  )

  add_executable(torque_sine_overlay_fri_${version_tag} src/torque_sine_overlay.cpp)
  ament_target_dependencies(torque_sine_overlay_fri_${version_tag}
    lbr_fri_idl
    lbr_fri_ros2
    rclcpp
  )
  target_link_libraries(torque_sine_overlay_fri_${version_tag}
    lbr_fri_ros2_fri_${version_tag}
    ${fri_target}
  )

  add_executable(wrench_sine_overlay_fri_${version_tag} src/wrench_sine_overlay.cpp)
  ament_target_dependencies(wrench_sine_overlay_fri_${version_tag}
    lbr_fri_idl
    lbr_fri_ros2
    rclcpp
  )
  target_link_libraries(wrench_sine_overlay_fri_${version_tag}
    lbr_fri_ros2_fri_${version_tag}
    ${fri_target}
  )

  list(APPEND LBR_DEMOS_CPP_VARIANT_TARGETS
    joint_sine_overlay_fri_${version_tag}
    joint_trajectory_client_fri_${version_tag}
    torque_sine_overlay_fri_${version_tag}
    wrench_sine_overlay_fri_${version_tag}
  )
endforeach()

install(TARGETS
  joint_sine_overlay
  joint_trajectory_client
  torque_sine_overlay
  wrench_sine_overlay
  ${LBR_DEMOS_CPP_VARIANT_TARGETS}
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
