cmake_minimum_required(VERSION 3.22)
project(lbr_fri_ros2)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(eigen3_cmake_module REQUIRED)
find_package(Eigen3)
find_package(FRIClient REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(kdl_parser REQUIRED)
find_package(lbr_fri_idl REQUIRED)
find_package(orocos_kdl_vendor REQUIRED)
find_package(rclcpp REQUIRED)
find_package(realtime_tools REQUIRED)
find_package(urdf REQUIRED)

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

function(lbr_add_fri_variant version)
  string(REPLACE "." "_" version_tag "${version}")
  if(TARGET FRIClient::FRIClient_${version_tag})
    set(fri_target "FRIClient::FRIClient_${version_tag}")
  else()
    message(STATUS "Skipping lbr_fri_ros2_fri_${version_tag} (missing FRIClient target)")
    return()
  endif()

  add_library(lbr_fri_ros2_fri_${version_tag}
    SHARED
      src/interfaces/base_command.cpp
      src/interfaces/position_command.cpp
      src/interfaces/state.cpp
      src/interfaces/torque_command.cpp
      src/interfaces/wrench_command.cpp
      src/app.cpp
      src/async_client.cpp
      src/command_guard.cpp
      src/control.cpp
      src/filters.cpp
      src/ft_estimator.cpp
      src/kinematics.cpp
      src/worker.cpp
  )

  target_include_directories(lbr_fri_ros2_fri_${version_tag}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
  )

  ament_target_dependencies(lbr_fri_ros2_fri_${version_tag}
    Eigen3
    geometry_msgs
    kdl_parser
    lbr_fri_idl
    orocos_kdl_vendor
    rclcpp
    realtime_tools
  )

  target_link_libraries(lbr_fri_ros2_fri_${version_tag}
    ${fri_target}
  )

  set(LBR_FRI_ROS2_VARIANT_TARGETS
    ${LBR_FRI_ROS2_VARIANT_TARGETS}
    lbr_fri_ros2_fri_${version_tag}
    PARENT_SCOPE
  )
endfunction()

# lbr_fri_ros2
set(LBR_FRI_ROS2_VARIANT_TARGETS "")
foreach(version IN LISTS LBR_FRI_VERSIONS)
  lbr_add_fri_variant("${version}")
endforeach()

string(REPLACE "." "_" LBR_FRI_DEFAULT_VERSION_TAG "${LBR_FRI_DEFAULT_VERSION}")
lbr_fri_select_target("${LBR_FRI_DEFAULT_VERSION}" LBR_FRI_DEFAULT_TARGET)
if(NOT LBR_FRI_DEFAULT_TARGET)
  message(FATAL_ERROR "No FRIClient target found for default version ${LBR_FRI_DEFAULT_VERSION}")
endif()

add_library(lbr_fri_ros2
  SHARED
    src/interfaces/base_command.cpp
    src/interfaces/position_command.cpp
    src/interfaces/state.cpp
    src/interfaces/torque_command.cpp
    src/interfaces/wrench_command.cpp
    src/app.cpp
    src/async_client.cpp
    src/command_guard.cpp
    src/control.cpp
    src/filters.cpp
    src/ft_estimator.cpp
    src/kinematics.cpp
    src/worker.cpp
)

target_include_directories(lbr_fri_ros2
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(lbr_fri_ros2
  Eigen3
  geometry_msgs
  kdl_parser
  lbr_fri_idl
  orocos_kdl_vendor
  rclcpp
  realtime_tools
)

target_link_libraries(lbr_fri_ros2
  ${LBR_FRI_DEFAULT_TARGET}
)

ament_export_targets(lbr_fri_ros2_export HAS_LIBRARY_TARGET)
ament_export_dependencies(
  eigen3_cmake_module
  Eigen3
  FRIClient
  geometry_msgs
  kdl_parser
  lbr_fri_idl
  orocos_kdl_vendor
  rclcpp
  realtime_tools
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  TARGETS lbr_fri_ros2
  EXPORT lbr_fri_ros2_export
  LIBRARY DESTINATION lib
)

if(LBR_FRI_ROS2_VARIANT_TARGETS)
  install(
    TARGETS ${LBR_FRI_ROS2_VARIANT_TARGETS}
    EXPORT lbr_fri_ros2_export
    LIBRARY DESTINATION lib
  )
endif()

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  ament_add_gtest(test_command_interfaces test/test_command_interfaces.cpp)
  target_link_libraries(test_command_interfaces lbr_fri_ros2)

  # # some examples of how to use the interfaces
  # add_executable(test_position_command test/test_position_command.cpp)
  # target_link_libraries(test_position_command lbr_fri_ros2)

  # add_executable(test_torque_command test/test_torque_command.cpp)
  # target_link_libraries(test_torque_command lbr_fri_ros2)

  # add_executable(test_wrench_command test/test_wrench_command.cpp)
  # target_link_libraries(test_wrench_command lbr_fri_ros2)
endif()

ament_package()
