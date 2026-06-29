'''import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    

    rviz_config = os.path.join(
        get_package_share_directory('robot_patrol'),
        'rivz',
        'realrvizconfig.rviz'  
    )
    return LaunchDescription([
        Node(
            package='robot_patrol',
            executable='robot_patrol_executable',
            output='screen'),
        Node(
            package='rviz2',
            executable='rviz2',
            arguments=['-d', rviz_config],
            output='screen'
        )
    ])
    '''
import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    rviz_config = os.path.join(
        get_package_share_directory('robot_patrol'),
        'rivz',
        'realvizconfig.rviz'
    )

    urdf_file = '/home/simulations/ros2_sims_ws/install/robot_description/share/fastbot_description/models/urdf/fastbot.urdf'

    with open(urdf_file, 'r') as f:
        robot_description = f.read()

    return LaunchDescription([
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{'robot_description': robot_description}],
            output='screen'
        ),
        Node(
            package='robot_patrol',
            executable='action_service',
            output='screen'
        ),
        Node(
            package='rviz2',
            executable='rviz2',
            arguments=['-d', rviz_config],
            output='screen'
        )
    ])