from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare


def generate_launch_description() -> LaunchDescription:
    ld = LaunchDescription()

    ld.add_action(
        DeclareLaunchArgument(
            name="model",
            default_value="iiwa7",
            description="Robot model. One of [iiwa7, iiwa14, med14].",
        )
    )
    ld.add_action(
        DeclareLaunchArgument(
            name="ctrl",
            default_value="joint_trajectory_controller",
            description="Desired default controller. One of specified in ctrl_cfg.",
        )
    )
    ld.add_action(
        DeclareLaunchArgument(
            name="robot1_name",
            default_value="white",
            description="Robot name for the first arm.",
        )
    )
    ld.add_action(
        DeclareLaunchArgument(
            name="robot2_name",
            default_value="orange",
            description="Robot name for the second arm.",
        )
    )
    ld.add_action(
        DeclareLaunchArgument(
            name="sys_cfg_pkg",
            default_value="lbr_description",
            description="Package containing the lbr_system_config.yaml file for FRI configurations.",
        )
    )
    ld.add_action(
        DeclareLaunchArgument(
            name="ctrl_cfg_pkg",
            default_value="lbr_description",
            description="Controller configuration package. The package containing the ctrl_cfg.",
        )
    )
    ld.add_action(
        DeclareLaunchArgument(
            name="robot1_sys_cfg",
            default_value="ros2_control/white_system_config.yaml",
            description="Relative path from sys_cfg_pkg to the first robot system config.",
        )
    )
    ld.add_action(
        DeclareLaunchArgument(
            name="robot2_sys_cfg",
            default_value="ros2_control/orange_system_config.yaml",
            description="Relative path from sys_cfg_pkg to the second robot system config.",
        )
    )
    ld.add_action(
        DeclareLaunchArgument(
            name="robot1_ctrl_cfg",
            default_value="ros2_control/hardware_controllers_white.yaml",
            description="Relative path from ctrl_cfg_pkg to the first robot controller config.",
        )
    )
    ld.add_action(
        DeclareLaunchArgument(
            name="robot2_ctrl_cfg",
            default_value="ros2_control/hardware_controllers_orange.yaml",
            description="Relative path from ctrl_cfg_pkg to the second robot controller config.",
        )
    )

    hardware_launch = PathJoinSubstitution(
        [FindPackageShare("lbr_bringup"), "launch", "hardware.launch.py"]
    )

    robot1 = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(hardware_launch),
        launch_arguments={
            "model": LaunchConfiguration("model"),
            "robot_name": LaunchConfiguration("robot1_name"),
            "sys_cfg_pkg": LaunchConfiguration("sys_cfg_pkg"),
            "sys_cfg": LaunchConfiguration("robot1_sys_cfg"),
            "ctrl_cfg_pkg": LaunchConfiguration("ctrl_cfg_pkg"),
            "ctrl_cfg": LaunchConfiguration("robot1_ctrl_cfg"),
            "ctrl": LaunchConfiguration("ctrl"),
        }.items(),
    )

    robot2 = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(hardware_launch),
        launch_arguments={
            "model": LaunchConfiguration("model"),
            "robot_name": LaunchConfiguration("robot2_name"),
            "sys_cfg_pkg": LaunchConfiguration("sys_cfg_pkg"),
            "sys_cfg": LaunchConfiguration("robot2_sys_cfg"),
            "ctrl_cfg_pkg": LaunchConfiguration("ctrl_cfg_pkg"),
            "ctrl_cfg": LaunchConfiguration("robot2_ctrl_cfg"),
            "ctrl": LaunchConfiguration("ctrl"),
        }.items(),
    )

    ld.add_action(robot1)
    ld.add_action(robot2)
    return ld
