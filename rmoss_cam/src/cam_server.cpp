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

#include "rmoss_cam/cam_server.hpp"

#include <memory>
#include <string>
#include <vector>
#include <chrono>

#include "cv_bridge/cv_bridge.h"
#include "sensor_msgs/msg/image.hpp"

using namespace std::chrono_literals;

namespace rmoss_cam
{

constexpr const CamParamType kCamParamTypes[] = {
  CamParamType::Width,
  CamParamType::Height,
  CamParamType::AutoExposure,
  CamParamType::Exposure,
  CamParamType::Brightness,
  CamParamType::AutoWhiteBalance,
  CamParamType::WhiteBalance,
  CamParamType::Gain,
  CamParamType::Gamma,
  CamParamType::Contrast,
  CamParamType::Saturation,
  CamParamType::Hue,
  CamParamType::Fps
};

constexpr const char * kCamParamTypeNames[] = {
  "width",
  "height",
  "auto_exposure",
  "exposure",
  "brightness",
  "auto_white_balance",
  "white_balance",
  "gain",
  "gamma",
  "contrast",
  "saturation",
  "hue",
  "fps"
};

CamServer::CamServer(
  rclcpp::Node::SharedPtr node,
  std::shared_ptr<CamInterface> cam_intercace)
: node_(node), cam_intercace_(cam_intercace)
{
  // 相机参数获取并设置，配置文件中的值会覆盖默认值
  int data;
  constexpr int param_num = sizeof(kCamParamTypes) / sizeof(CamParamType);
  for (int i = 0; i < param_num; i++) {
    if (cam_intercace_->get_parameter(kCamParamTypes[i], data)) {
      node->declare_parameter(kCamParamTypeNames[i], data);
      node->get_parameter(kCamParamTypeNames[i], data);
      cam_intercace_->set_parameter(kCamParamTypes[i], data);
    }
  }
  // 获取fps
  cam_intercace_->get_parameter(rmoss_cam::CamParamType::Fps, fps_);
  // 如果fps值非法，则设置为默认值30
  if (fps_ <= 0) {
    fps_ = 30;
    cam_intercace_->set_parameter(rmoss_cam::CamParamType::Fps, 30);
  }
  // 打开摄像头
  if (!cam_intercace_->open()) {
    RCLCPP_FATAL(
      node_->get_logger(), "fail to open camera: %s",
      cam_intercace_->error_message().c_str());
  }
  std::string camera_name = "camera";
  // declare parameters
  node->declare_parameter("camera_name", "camera");
  node->declare_parameter("autostart", run_flag_);
  if (!node->has_parameter("camera_k")) {
    node->declare_parameter("camera_k", camera_k_);
  }
  if (!node->has_parameter("camera_p")) {
    node->declare_parameter("camera_p", camera_p_);
  }
  if (!node->has_parameter("camera_d")) {
    node->declare_parameter("camera_d", camera_d_);
  }
  node->get_parameter("camera_name", camera_name);
  node->get_parameter("autostart", run_flag_);
  node->get_parameter("camera_k", camera_k_);
  node->get_parameter("camera_p", camera_p_);
  node->get_parameter("camera_d", camera_d_);
  // check parameters
  if (camera_k_.size() != 9) {
    RCLCPP_ERROR(
      node_->get_logger(),
      "the size of camera intrinsic parameter(%ld) != 9", camera_k_.size());
  }
  if (camera_p_.size() != 12) {
    RCLCPP_ERROR(
      node_->get_logger(),
      "the size of camera extrinsic parameter(%ld) != 12", camera_p_.size());
  }
  // create image publisher
  img_pub_ = node_->create_publisher<sensor_msgs::msg::Image>(camera_name + "/image_raw", 1);
  auto period_ms = std::chrono::milliseconds(static_cast<int64_t>(1000.0 / fps_));
  timer_ = node->create_wall_timer(period_ms, std::bind(&CamServer::timer_callback, this));
  // create GetCameraInfo service
  using namespace std::placeholders;
  get_camera_info_srv_ = node->create_service<rmoss_interfaces::srv::GetCameraInfo>(
    camera_name + "/get_camera_info",
    std::bind(&CamServer::get_camera_info_cb, this, _1, _2));
  // task manager
  auto get_task_status_cb = [&]() {
      if (run_flag_) {
        if (!cam_status_ok_) {
          return rmoss_util::TaskStatus::Error;
        }
        return rmoss_util::TaskStatus::Running;
      } else {
        if (!cam_intercace_->is_open()) {
          return rmoss_util::TaskStatus::Error;
        }
        return rmoss_util::TaskStatus::Idle;
      }
    };
  auto control_task_cb = [&](rmoss_util::TaskCmd cmd) {
      if (cmd == rmoss_util::TaskCmd::Start) {
        run_flag_ = true;
      } else if (cmd == rmoss_util::TaskCmd::Stop) {
        run_flag_ = false;
      } else {
        return false;
      }
      return true;
    };
  task_manager_ = std::make_shared<rmoss_util::TaskManager>(
    node_, get_task_status_cb, control_task_cb);
  RCLCPP_INFO(node_->get_logger(), "init successfully!");
}

void CamServer::timer_callback()
{
  if (!run_flag_) {
    return;
  }
  if (cam_intercace_->grab_image(img_)) {
    cam_status_ok_ = true;
    auto header = std_msgs::msg::Header();
    header.stamp = node_->now();
    // publish image msg
    sensor_msgs::msg::Image::SharedPtr img_msg = cv_bridge::CvImage(
      header, "bgr8", img_).toImageMsg();
    img_pub_->publish(*img_msg);
  } else {
    // try to reopen camera
    cam_status_ok_ = false;
    if (reopen_cnt % fps_ == 0) {
      cam_intercace_->close();
      std::this_thread::sleep_for(100ms);
      if (cam_intercace_->open()) {
        RCLCPP_WARN(node_->get_logger(), "reopen camera successed!");
      } else {
        RCLCPP_WARN(node_->get_logger(), "reopen camera failed!");
      }
    }
    reopen_cnt++;
  }
}

void CamServer::get_camera_info_cb(
  const rmoss_interfaces::srv::GetCameraInfo::Request::SharedPtr request,
  rmoss_interfaces::srv::GetCameraInfo::Response::SharedPtr response)
{
  (void)request;
  auto & camera_info = response->camera_info;
  int data;
  cam_intercace_->get_parameter(rmoss_cam::CamParamType::Height, data);
  camera_info.height = data;
  cam_intercace_->get_parameter(rmoss_cam::CamParamType::Width, data);
  camera_info.width = data;
  if (camera_k_.size() == 9 && camera_p_.size() == 12) {
    std::copy_n(camera_k_.begin(), 9, camera_info.k.begin());
    std::copy_n(camera_p_.begin(), 12, camera_info.p.begin());
    camera_info.d = camera_d_;
    response->success = true;
  } else {
    response->success = false;
  }
}

}  // namespace rmoss_cam
