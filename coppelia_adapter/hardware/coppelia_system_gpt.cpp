#include <memory>
#include <string>
#include <vector>
#include <cmath>

#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "std_msgs/msg/float64.hpp"
#include "rclcpp_lifecycle/state.hpp"

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class CoppeliaSimHW : public hardware_interface::SystemInterface
{
public:
  CallbackReturn on_init(const hardware_interface::HardwareInfo & info) override
  {
    if (hardware_interface::SystemInterface::on_init(info) != CallbackReturn::SUCCESS)
      return CallbackReturn::ERROR;

    robot_name_ = info_.hardware_parameters["robot_name"]; // I changed info to info_ so the linter is happy
    for (const auto & joint : info.joints)
    {
      joint_names_.push_back(joint.name);
      hw_positions_.push_back(0.0);
      hw_velocities_.push_back(0.0);
      hw_commands_.push_back(0.0);
    }

    // ROS node handle
    rclcpp::NodeOptions options;
    node_ = std::make_shared<rclcpp::Node>("coppeliasim_hw_" + robot_name_, options);

    // Subscriber to joint_states
    joint_state_sub_ = node_->create_subscription<sensor_msgs::msg::JointState>(
      "/" + robot_name_ + "/joint_states", 10,
      std::bind(&CoppeliaSimHW::jointStateCallback, this, std::placeholders::_1)
    );

    // Publishers for wheel velocity commands (assumes two wheels)
    left_wheel_pub_ = node_->create_publisher<std_msgs::msg::Float64>(
      "/" + robot_name_ + "/left_wheel_velocity", 10
    );
    right_wheel_pub_ = node_->create_publisher<std_msgs::msg::Float64>(
      "/" + robot_name_ + "/right_wheel_velocity", 10
    );

    return CallbackReturn::SUCCESS;
  }

  std::vector<hardware_interface::StateInterface> export_state_interfaces() override
  {
    std::vector<hardware_interface::StateInterface> state_interfaces;
    for (size_t i = 0; i < joint_names_.size(); ++i)
    {
      state_interfaces.emplace_back(hardware_interface::StateInterface(
        joint_names_[i], hardware_interface::HW_IF_POSITION, &hw_positions_[i]));
      state_interfaces.emplace_back(hardware_interface::StateInterface(
        joint_names_[i], hardware_interface::HW_IF_VELOCITY, &hw_velocities_[i]));
    }
    return state_interfaces;
  }

  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override
  {
    std::vector<hardware_interface::CommandInterface> command_interfaces;
    for (size_t i = 0; i < joint_names_.size(); ++i)
    {
      command_interfaces.emplace_back(hardware_interface::CommandInterface(
        joint_names_[i], hardware_interface::HW_IF_VELOCITY, &hw_commands_[i]));
    }
    return command_interfaces;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override
  {
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override
  {
    return CallbackReturn::SUCCESS;
  }

  hardware_interface::return_type read(const rclcpp::Time &, const rclcpp::Duration &)
  {
    // joint_states updated via subscriber callback
    return hardware_interface::return_type::OK;
  }

  hardware_interface::return_type write(const rclcpp::Time &, const rclcpp::Duration &)
  {
    if (joint_names_.size() < 2) return hardware_interface::return_type::ERROR;

    std_msgs::msg::Float64 left_msg;
    std_msgs::msg::Float64 right_msg;

    left_msg.data = hw_commands_[0];   // left wheel velocity
    right_msg.data = hw_commands_[1];  // right wheel velocity

    left_wheel_pub_->publish(left_msg);
    right_wheel_pub_->publish(right_msg);

    rclcpp::spin_some(node_);
    return hardware_interface::return_type::OK;
  }

private:
  void jointStateCallback(const sensor_msgs::msg::JointState::SharedPtr msg)
  {
    for (size_t i = 0; i < joint_names_.size(); ++i)
    {
      for (size_t j = 0; j < msg->name.size(); ++j)
      {
        if (msg->name[j] == joint_names_[i])
        {
          hw_positions_[i] = msg->position[j];
          hw_velocities_[i] = msg->velocity[j];
          break;
        }
      }
    }
  }

  std::string robot_name_;
  std::vector<std::string> joint_names_;
  std::vector<double> hw_positions_;
  std::vector<double> hw_velocities_;
  std::vector<double> hw_commands_;

  rclcpp::Node::SharedPtr node_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr left_wheel_pub_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr right_wheel_pub_;
};

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(CoppeliaSimHW, hardware_interface::SystemInterface)
