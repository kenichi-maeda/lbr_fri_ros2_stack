#ifndef LBR_ROS2_CONTROL__SYSTEM_INTERFACE_SELECTOR_HPP_
#define LBR_ROS2_CONTROL__SYSTEM_INTERFACE_SELECTOR_HPP_

#include <memory>
#include <string>
#include <vector>

#include "hardware_interface/system_interface.hpp"
#include "pluginlib/class_loader.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/state.hpp"

namespace lbr_ros2_control {

class SystemInterfaceSelector : public hardware_interface::SystemInterface {
public:
  SystemInterfaceSelector() = default;

  controller_interface::CallbackReturn
  on_init(const hardware_interface::HardwareInfo &info) override;
  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  hardware_interface::return_type prepare_command_mode_switch(
      const std::vector<std::string> &start_interfaces,
      const std::vector<std::string> &stop_interfaces) override;

  controller_interface::CallbackReturn
  on_activate(const rclcpp_lifecycle::State &previous_state) override;
  controller_interface::CallbackReturn
  on_deactivate(const rclcpp_lifecycle::State &previous_state) override;

  hardware_interface::return_type read(const rclcpp::Time &time,
                                       const rclcpp::Duration &period) override;
  hardware_interface::return_type write(const rclcpp::Time &time,
                                        const rclcpp::Duration &period) override;

private:
  static constexpr char LOGGER_NAME[] = "lbr_ros2_control::SystemInterfaceSelector";

  std::string pick_version_string_(const hardware_interface::HardwareInfo &info) const;
  std::string make_class_name_(const std::string &version) const;
  bool ensure_backend_loaded_(const hardware_interface::HardwareInfo &info);

  std::unique_ptr<pluginlib::ClassLoader<hardware_interface::SystemInterface>> loader_;
  std::shared_ptr<hardware_interface::SystemInterface> backend_;
  std::string backend_class_name_;
};

} // namespace lbr_ros2_control

#endif // LBR_ROS2_CONTROL__SYSTEM_INTERFACE_SELECTOR_HPP_
