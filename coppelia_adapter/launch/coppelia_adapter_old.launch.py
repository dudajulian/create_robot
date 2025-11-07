from launch import LaunchDescription
from launch.actions import RegisterEventHandler
from launch.event_handlers import OnProcessStart
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    # Get URDF via xacro
    robot_description = Command([ 
        FindExecutable(name="xacro"),
        " ",
        PathJoinSubstitution([
            FindPackageShare("coppelia_adapter"), "description/urdf", "create_2.urdf.xacro"
        ]),
    ])

    # robot_controllers = PathJoinSubstitution(
    #     [
    #         FindPackageShare("coppelia_adapter"),
    #         "config",
    #         "diff_drive_controller.yaml",
    #     ]
    # )

    rsp_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{"robot_description": robot_description}],
        output="screen",
    )

    joint_state_publisher_node = Node(
        package="joint_state_publisher_gui",
        executable="joint_state_publisher_gui",
        output="screen",
    )

    # control_node = Node(
    #     package="controller_manager",
    #     executable="ros2_control_node",
    #     parameters=[robot_controllers],
    #     output="screen",
    # )

    # diff_drive_spawner = Node(
    #     package="controller_manager",
    #     executable="spawner",
    #     arguments=["diff_drive_controller"],
    # )

    # adapter_node = Node(
    #     package="coppelia_adapter",
    #     executable="diff_drive_adapter",
    #     name="diff_drive_adapter"
    # )

    return LaunchDescription([
        rsp_node,
        joint_state_publisher_node
        # control_node,
        # diff_drive_spawner,
        # adapter_node
    ])