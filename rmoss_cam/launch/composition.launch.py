# Copyright 2021 RoboMaster-OSS
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import LoadComposableNodes
from launch_ros.actions import Node
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    image_path = os.path.join(get_package_share_directory('rmoss_cam'), 'resource', 'test.jpg')
    # 创建容器
    rmoss_container = Node(
        name='rmoss_container',
        package='rclcpp_components',
        executable='component_container',
        output='screen'
    )
    # 加载虚拟图片相机节点到容器中
    load_cam = LoadComposableNodes(
        target_container='rmoss_container',
        composable_node_descriptions=[
            ComposableNode(
                package='rmoss_cam',
                plugin='rmoss_cam::VirtualCamNode',
                name='virtual_image_cam',
                parameters=[{'image_path': image_path,
                             'camera_name': 'front_camera',
                             'camera_k': [1552.7, 0.0, 640.0, 0.0, 1537.9, 360.0, 0.0, 0.0, 1.0],
                             'camera_d': [0.0, 0.0, 0.0, 0.0, 0.0],
                             'fps': 30,
                             'autostart': True}])
        ],
    )
    ld = LaunchDescription()
    ld.add_action(rmoss_container)
    ld.add_action(load_cam)
    return ld
