// Copyright 2021 Open Source Robotics Foundation, Inc.
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

#ifndef TYPE_ADAPT_EXAMPLE__JULIASET_NODE_HPP_
#define TYPE_ADAPT_EXAMPLE__JULIASET_NODE_HPP_

#include "cuda/julia_set.hpp"

#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "type_adapters/image_container.hpp"

RCLCPP_USING_CUSTOM_TYPE_AS_ROS_MESSAGE_TYPE(
  type_adapt_example::ImageContainer,
  sensor_msgs::msg::Image);

namespace type_adapt_example
{

/**
 * @brief
 *
 */

class JuliasetNode : public rclcpp::Node
{
public:
  explicit JuliasetNode(const rclcpp::NodeOptions options = rclcpp::NodeOptions());
  ~JuliasetNode() {}

private:
/**
* @brief Callback method on each image msg.
*
* @param img_msg Pointer to the image msg
*/
  void JuliasetCallback(std::unique_ptr<type_adapt_example::ImageContainer> image);

  // Flag for pipeline mode or composite mode
  const bool is_composite_;
  // Flag for intialization.
  bool is_initialized;
  // Current node number in case of pipeline mode
  const uint8_t proc_id_;
  // Counter
  size_t counter_{0};
  // Juliaset start x
  float start_x_{0.0};
  // Juliaset start y
  float start_y_{0.0};
  // Juliaset prams
  JuliasetParams juliaset_params_{}; \
  // Image properties to be sent to CUDA kernel
  ImageMsgProperties img_property_{};
  // Juliaset handle
  std::unique_ptr<Juliaset> juliaset_handle_;

  rclcpp::Subscription<type_adapt_example::ImageContainer>::SharedPtr sub_;
  rclcpp::Publisher<type_adapt_example::ImageContainer>::SharedPtr pub_;
};
}  // namespace type_adapt_example
#endif  // TYPE_ADAPT_EXAMPLE__JULIASET_NODE_HPP_