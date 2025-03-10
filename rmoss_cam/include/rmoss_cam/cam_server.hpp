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

#ifndef RMOSS_CAM__CAM_SERVER_HPP_
#define RMOSS_CAM__CAM_SERVER_HPP_

#include <thread>
#include <string>
#include <memory>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "opencv2/opencv.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "rmoss_interfaces/srv/get_camera_info.hpp"
#include "rmoss_cam/cam_interface.hpp"
#include "rmoss_util/task_manager.hpp"

namespace rmoss_cam
{
// camera server wrapper to publish image
class CamServer
{
public:
  CamServer(
    rclcpp::Node::SharedPtr node,
    std::shared_ptr<CamInterface> cam_intercace);

private:
  void timer_callback();
  void get_camera_info_cb(
    const rmoss_interfaces::srv::GetCameraInfo::Request::SharedPtr request,
    rmoss_interfaces::srv::GetCameraInfo::Response::SharedPtr response);

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr img_pub_;
  rclcpp::Service<rmoss_interfaces::srv::GetCameraInfo>::SharedPtr get_camera_info_srv_;
  rmoss_util::TaskManager::SharedPtr task_manager_;
  rclcpp::TimerBase::SharedPtr timer_;
  // camera_device interface
  std::shared_ptr<CamInterface> cam_intercace_;
  bool run_flag_{false};
  bool cam_status_ok_{false};
  // data
  cv::Mat img_;
  std::vector<double> camera_k_;  // 3*3=9
  std::vector<double> camera_p_{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0};  // 3*4=12
  std::vector<double> camera_d_;
  bool has_camera_info_{false};
  int fps_{30};
  int reopen_cnt{0};
};

}  // namespace rmoss_cam

#endif  // RMOSS_CAM__CAM_SERVER_HPP_
