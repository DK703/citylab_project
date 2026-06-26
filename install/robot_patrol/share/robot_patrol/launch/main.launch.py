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
   
    return LaunchDescription([
        Node(
            package='robot_patrol',
            executable='direction_service',
            output='screen'),
        Node(
            package= 'robot_patrol',
            executable='robot_patrol_executable2',
            output='screen'),
        Node(
            package='joint_state_publisher',
            executable='joint_state_publisher',
            name='joint_state_publisher',
            output='screen'
        ),
        Node(
            package='rviz2',
            executable='rviz2',
            arguments=['-d', rviz_config],
            output='screen'
        )
    
    ])