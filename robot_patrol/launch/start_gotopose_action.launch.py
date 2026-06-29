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
            executable='action_service',
            output='screen')
    
    ])