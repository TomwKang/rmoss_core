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

#ifndef RMOSS_CAM__CAM_CLIENT_HPP_
#define RMOSS_CAM__CAM_CLIENT_HPP_

#include <string>
#include <thread>
#include <vector>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "opencv2/opencv.hpp"

namespace rmoss_cam
{
typedef std::function<void (cv::Mat &, rclcpp::Time)> Callback;
// camera client wrapper to subscribe image (used by auto aim task, power rune task..)
class CamClient
{
public:
  CamClient() = delete;
  CamClient(
    rclcpp::Node::SharedPtr node, std::string camera_name, Callback process_fn,
    bool spin_thread = true);
  ~CamClient();

  bool get_camera_info(sensor_msgs::msg::CameraInfo & info);
  void start();
  void stop();

private:
  void img_cb(const sensor_msgs::msg::Image::ConstSharedPtr msg);

private:
  rclcpp::Node::SharedPtr node_;
  std::string camera_name_;
  rclcpp::CallbackGroup::SharedPtr callback_group_{nullptr};
  rclcpp::executors::SingleThreadedExecutor::SharedPtr executor_;
  std::unique_ptr<std::thread> executor_thread_;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr img_sub_;  // 订阅图片数据
  Callback process_fn_;
  bool spin_thread_;
  bool run_flag_{false};  // 运行标志位
};
}  // namespace rmoss_cam

#endif  // RMOSS_CAM__CAM_CLIENT_HPP_
