// Copyright 2021 RoboMaster-OSS
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "rmoss_cam/usb_cam_node.hpp"

#include <memory>
#include <string>
#include <vector>
#include <chrono>

using namespace std::chrono_literals;


namespace rmoss_cam
{
UsbCamNode::UsbCamNode(const rclcpp::NodeOptions & options)
{
  node_ = std::make_shared<rclcpp::Node>("usb_cam", options);
  // declare parameters
  node_->declare_parameter("usb_cam_path", "/dev/video0");
  // get parameters
  auto dev_name = node_->get_parameter("usb_cam_path").as_string();
  // create camera device
  cam_dev_ = std::make_shared<UsbCam>(dev_name);
  cam_server_ = std::make_shared<CamServer>(node_, cam_dev_);
}

}  // namespace rmoss_cam

#include "rclcpp_components/register_node_macro.hpp"

// Register the component with class_loader.
// This acts as a sort of entry point, allowing the component to be discoverable when its library
// is being loaded into a running process.
RCLCPP_COMPONENTS_REGISTER_NODE(rmoss_cam::UsbCamNode)
