// Copyright (c) 2021, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvToolsExt.h>  // NOLINT

#include "julia_set/colorize_node.hpp"
#include <memory>
#include <utility>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"
#include "sensor_msgs/image_encodings.hpp"
#include "type_adapters/image_container.hpp"

namespace type_adaptation
{
namespace julia_set
{

ColorizeNode::ColorizeNode(rclcpp::NodeOptions options)
: rclcpp::Node("colorize_node", options.use_intra_process_comms(true)),
  type_adaptation_enabled_(declare_parameter<bool>("type_adaptation_enabled", true)),
  is_initialized{false}
{
  RCLCPP_INFO(
    get_logger(), "Setting up Colorize node with adaptation enabled: %s",
    type_adaptation_enabled_ ? "YES" : "NO");

  julia_set_params_.kMaxIterations = declare_parameter<int>("max_iterations", 50);

  if (type_adaptation_enabled_) {
    custom_type_sub_ = create_subscription<type_adaptation::example_type_adapters::ImageContainer>(
      "image_in", 1,
      std::bind(&ColorizeNode::ColorizeCallbackCustomType, this, std::placeholders::_1));
    custom_type_pub_ = create_publisher<type_adaptation::example_type_adapters::ImageContainer>(
      "image_out", 1);
  } else {
    sub_ =
      create_subscription<sensor_msgs::msg::Image>(
      "image_in", 1, std::bind(&ColorizeNode::ColorizeCallback, this, std::placeholders::_1));
    pub_ = create_publisher<sensor_msgs::msg::Image>("image_out", 1);
  }
}

void ColorizeNode::ColorizeCallbackCustomType(
  std::unique_ptr<type_adaptation::example_type_adapters::ImageContainer> image)
{
  nvtxRangePushA("ColorizeNode: ColorizeCallbackCustomType");
  if (!is_initialized) {
    img_property_.row_step = image->step();
    img_property_.height = image->height();
    img_property_.width = image->width();
    img_property_.encoding = image->encoding();

    if (image->encoding() == sensor_msgs::image_encodings::RGB8) {
      img_property_.red_offset = 0;
      img_property_.green_offset = 1;
      img_property_.blue_offset = 2;
      img_property_.color_step = 3;
    } else if (image->encoding() == sensor_msgs::image_encodings::BGR8) {
      img_property_.blue_offset = 0;
      img_property_.green_offset = 1;
      img_property_.red_offset = 2;
      img_property_.color_step = 3;
    } else if (image->encoding() == sensor_msgs::image_encodings::MONO8) {
      img_property_.red_offset = 0;
      img_property_.green_offset = 0;
      img_property_.blue_offset = 0;
      img_property_.color_step = 1;
    }

    julia_set_handle_ = std::make_unique<JuliaSet>(img_property_, julia_set_params_);
    is_initialized = true;
  }

  auto out = std::make_unique<type_adaptation::example_type_adapters::ImageContainer>(
    image->header(), image->height(), image->width(), image->encoding(),
    image->step() / sizeof(float), image->cuda_stream());

  julia_set_handle_->colorize(
    out->cuda_mem(), reinterpret_cast<float *>(image->cuda_mem()), out->cuda_stream()->stream());

  custom_type_pub_->publish(std::move(out));
  nvtxRangePop();
}

void ColorizeNode::ColorizeCallback(std::unique_ptr<sensor_msgs::msg::Image> image_msg)
{
  nvtxRangePushA("ColorizeNode: ColorizeCallback");
  std::unique_ptr<type_adaptation::example_type_adapters::ImageContainer> image =
    std::make_unique<type_adaptation::example_type_adapters::ImageContainer>(std::move(image_msg));
  if (!is_initialized) {
    img_property_.row_step = image->step();
    img_property_.height = image->height();
    img_property_.width = image->width();
    img_property_.encoding = image->encoding();

    if (image->encoding() == sensor_msgs::image_encodings::RGB8) {
      img_property_.red_offset = 0;
      img_property_.green_offset = 1;
      img_property_.blue_offset = 2;
      img_property_.color_step = 3;
    } else if (image->encoding() == sensor_msgs::image_encodings::BGR8) {
      img_property_.blue_offset = 0;
      img_property_.green_offset = 1;
      img_property_.red_offset = 2;
      img_property_.color_step = 3;
    } else if (image->encoding() == sensor_msgs::image_encodings::MONO8) {
      img_property_.red_offset = 0;
      img_property_.green_offset = 0;
      img_property_.blue_offset = 0;
      img_property_.color_step = 1;
    }

    julia_set_handle_ = std::make_unique<JuliaSet>(img_property_, julia_set_params_);
    is_initialized = true;
  }

  auto out = std::make_unique<type_adaptation::example_type_adapters::ImageContainer>(
    image->header(), image->height(), image->width(), image->encoding(),
    image->step() / sizeof(float), image->cuda_stream());

  julia_set_handle_->colorize(
    out->cuda_mem(), reinterpret_cast<float *>(image->cuda_mem()), out->cuda_stream()->stream());

  // Convert in-place before publishing to "disable" type adaptation
  sensor_msgs::msg::Image image_msg_out;
  out->get_sensor_msgs_image(image_msg_out);

  pub_->publish(std::move(image_msg_out));
  nvtxRangePop();
}

}  // namespace julia_set
}  // namespace type_adaptation

RCLCPP_COMPONENTS_REGISTER_NODE(type_adaptation::julia_set::ColorizeNode)
