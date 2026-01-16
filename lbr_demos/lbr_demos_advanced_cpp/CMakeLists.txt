cmake_minimum_required(VERSION 3.22)
project(lbr_demos_advanced_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(FRIClient REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(kdl_parser REQUIRED)
find_package(lbr_fri_idl REQUIRED)
find_package(lbr_fri_ros2 REQUIRED)
find_package(orocos_kdl_vendor REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)

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

# pose planning node
add_library(pose_planning_component
  SHARED
    src/pose_planning_node.cpp
)

target_include_directories(pose_planning_component
  PRIVATE src
)

ament_target_dependencies(pose_planning_component
  geometry_msgs
  rclcpp
  rclcpp_components
)

rclcpp_components_register_node(pose_planning_component
  PLUGIN lbr_demos::PosePlanningNode
  EXECUTABLE pose_planning
)

# pose contol node
add_library(pose_control_component
  SHARED
    src/pose_control_node.cpp
)

target_include_directories(pose_control_component
  PRIVATE src
)

ament_target_dependencies(pose_control_component
  geometry_msgs
  kdl_parser
  lbr_fri_idl
  orocos_kdl_vendor
  rclcpp
  rclcpp_components
)

target_link_libraries(pose_control_component
  ${LBR_FRI_DEFAULT_TARGET}
)

rclcpp_components_register_node(pose_control_component
  PLUGIN lbr_demos::PoseControlNode
  EXECUTABLE pose_control
)

set(LBR_DEMOS_ADVANCED_CPP_VARIANT_TARGETS "")
foreach(version IN LISTS LBR_FRI_VERSIONS)
  string(REPLACE "." "_" version_tag "${version}")
  if(TARGET FRIClient::FRIClient_${version_tag})
    set(fri_target "FRIClient::FRIClient_${version_tag}")
  else()
    message(STATUS "Skipping pose_control_component_fri_${version_tag} (missing FRIClient target)")
    continue()
  endif()
  if(NOT TARGET lbr_fri_ros2_fri_${version_tag})
    message(STATUS "Skipping pose_control_component_fri_${version_tag} (missing lbr_fri_ros2_fri target)")
    continue()
  endif()

  add_library(pose_control_component_fri_${version_tag}
    SHARED
      src/pose_control_node.cpp
  )
  target_include_directories(pose_control_component_fri_${version_tag}
    PRIVATE src
  )
  ament_target_dependencies(pose_control_component_fri_${version_tag}
    geometry_msgs
    kdl_parser
    lbr_fri_idl
    orocos_kdl_vendor
    rclcpp
    rclcpp_components
  )
  target_link_libraries(pose_control_component_fri_${version_tag}
    lbr_fri_ros2_fri_${version_tag}
    ${fri_target}
  )

  rclcpp_components_register_node(pose_control_component_fri_${version_tag}
    PLUGIN lbr_demos::PoseControlNode
    EXECUTABLE pose_control_fri_${version_tag}
  )

  list(APPEND LBR_DEMOS_ADVANCED_CPP_VARIANT_TARGETS pose_control_component_fri_${version_tag})
endforeach()

# install
install(
  TARGETS pose_planning_component pose_control_component ${LBR_DEMOS_ADVANCED_CPP_VARIANT_TARGETS}
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
