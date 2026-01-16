cmake_minimum_required(VERSION 3.22)
project(lbr_ros2_control)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(ament_cmake REQUIRED)
find_package(controller_interface REQUIRED)
find_package(eigen3_cmake_module REQUIRED)
find_package(Eigen3)
find_package(FRIClient REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(kinematics_interface REQUIRED)
find_package(lbr_fri_idl REQUIRED)
find_package(lbr_fri_ros2 REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(realtime_tools REQUIRED)

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

function(lbr_configure_ros2_control_target target_name fri_target)
  target_include_directories(
    ${target_name}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  )

  ament_target_dependencies(
    ${target_name}
    ${AMENT_DEPENDENCIES}
  )

  target_link_libraries(${target_name}
    ${fri_target}
  )
endfunction()

# LBR ROS 2 control library
set(AMENT_DEPENDENCIES
  controller_interface
  Eigen3
  hardware_interface
  kinematics_interface
  lbr_fri_idl
  lbr_fri_ros2
  pluginlib
  rclcpp
  realtime_tools
)

set(LBR_ROS2_CONTROL_VARIANT_TARGETS "")
foreach(version IN LISTS LBR_FRI_VERSIONS)
  string(REPLACE "." "_" version_tag "${version}")
  if(TARGET FRIClient::FRIClient_${version_tag})
    set(fri_target "FRIClient::FRIClient_${version_tag}")
  else()
    message(STATUS "Skipping lbr_ros2_control_fri_${version_tag} (missing FRIClient target)")
    continue()
  endif()
  if(NOT TARGET lbr_fri_ros2_fri_${version_tag})
    message(STATUS "Skipping lbr_ros2_control_fri_${version_tag} (missing lbr_fri_ros2_fri target)")
    continue()
  endif()

  add_library(
    lbr_ros2_control_fri_${version_tag}
    SHARED
    src/controllers/admittance_controller.cpp
    src/controllers/lbr_joint_position_command_controller.cpp
    src/controllers/lbr_torque_command_controller.cpp
    src/controllers/lbr_wrench_command_controller.cpp
    src/controllers/lbr_state_broadcaster.cpp
    src/controllers/twist_controller.cpp
    src/system_interface.cpp
  )

  target_compile_definitions(lbr_ros2_control_fri_${version_tag}
    PRIVATE LBR_ROS2_CONTROL_SYSTEM_INTERFACE_CLASS_NAME=SystemInterfaceFri_${version_tag}
  )

  lbr_configure_ros2_control_target(lbr_ros2_control_fri_${version_tag} ${fri_target})
  target_link_libraries(lbr_ros2_control_fri_${version_tag}
    lbr_fri_ros2_fri_${version_tag}
  )

  set(LBR_ROS2_CONTROL_VARIANT_TARGETS
    ${LBR_ROS2_CONTROL_VARIANT_TARGETS}
    lbr_ros2_control_fri_${version_tag}
  )
endforeach()

string(REPLACE "." "_" LBR_FRI_DEFAULT_VERSION_TAG "${LBR_FRI_DEFAULT_VERSION}")
lbr_fri_select_target("${LBR_FRI_DEFAULT_VERSION}" LBR_FRI_DEFAULT_TARGET)
if(NOT LBR_FRI_DEFAULT_TARGET)
  message(FATAL_ERROR "No FRIClient target found for default version ${LBR_FRI_DEFAULT_VERSION}")
endif()

add_library(
  ${PROJECT_NAME}
  SHARED
  src/controllers/admittance_controller.cpp
  src/controllers/lbr_joint_position_command_controller.cpp
  src/controllers/lbr_torque_command_controller.cpp
  src/controllers/lbr_wrench_command_controller.cpp
  src/controllers/lbr_state_broadcaster.cpp
  src/controllers/twist_controller.cpp
  src/system_interface.cpp
)

lbr_configure_ros2_control_target(${PROJECT_NAME} ${LBR_FRI_DEFAULT_TARGET})

add_library(
  lbr_ros2_control_selector
  SHARED
  src/system_interface_selector.cpp
)

target_include_directories(
  lbr_ros2_control_selector
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(
  lbr_ros2_control_selector
  controller_interface
  hardware_interface
  pluginlib
  rclcpp
)

target_compile_definitions(lbr_ros2_control_selector
  PRIVATE LBR_FRI_DEFAULT_VERSION="${LBR_FRI_DEFAULT_VERSION}"
)

pluginlib_export_plugin_description_file(controller_interface plugin_description_files/controllers.xml)
pluginlib_export_plugin_description_file(hardware_interface plugin_description_files/system_interface.xml)

# Export for downstream usage, see https://docs.ros.org/en/foxy/Guides/Ament-CMake-Documentation.html
ament_export_targets(
  ${PROJECT_NAME}_export HAS_LIBRARY_TARGET
)

ament_export_dependencies(
  FRIClient
  ${AMENT_DEPENDENCIES}
  eigen3_cmake_module
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  TARGETS ${PROJECT_NAME} lbr_ros2_control_selector ${LBR_ROS2_CONTROL_VARIANT_TARGETS}
  EXPORT ${PROJECT_NAME}_export
  LIBRARY DESTINATION lib/${PROJECT_NAME}
  RUNTIME DESTINATION lib/${PROJECT_NAME}
  INCLUDES DESTINATION include
)

ament_package()
