#include "lbr_ros2_control/system_interface_selector.hpp"

#include <algorithm>
#include <cstdlib>

namespace lbr_ros2_control {

controller_interface::CallbackReturn
SystemInterfaceSelector::on_init(const hardware_interface::HardwareInfo &info) {
  if (!ensure_backend_loaded_(info)) {
    return controller_interface::CallbackReturn::ERROR;
  }
  return backend_->on_init(info);
}

std::vector<hardware_interface::StateInterface>
SystemInterfaceSelector::export_state_interfaces() {
  return backend_->export_state_interfaces();
}

std::vector<hardware_interface::CommandInterface>
SystemInterfaceSelector::export_command_interfaces() {
  return backend_->export_command_interfaces();
}

hardware_interface::return_type SystemInterfaceSelector::prepare_command_mode_switch(
    const std::vector<std::string> &start_interfaces,
    const std::vector<std::string> &stop_interfaces) {
  return backend_->prepare_command_mode_switch(start_interfaces, stop_interfaces);
}

controller_interface::CallbackReturn
SystemInterfaceSelector::on_activate(const rclcpp_lifecycle::State &previous_state) {
  return backend_->on_activate(previous_state);
}

controller_interface::CallbackReturn
SystemInterfaceSelector::on_deactivate(const rclcpp_lifecycle::State &previous_state) {
  return backend_->on_deactivate(previous_state);
}

hardware_interface::return_type SystemInterfaceSelector::read(const rclcpp::Time &time,
                                                              const rclcpp::Duration &period) {
  return backend_->read(time, period);
}

hardware_interface::return_type SystemInterfaceSelector::write(const rclcpp::Time &time,
                                                               const rclcpp::Duration &period) {
  return backend_->write(time, period);
}

std::string SystemInterfaceSelector::pick_version_string_(
    const hardware_interface::HardwareInfo &info) const {
  const char *env_version = std::getenv("LBR_FRI_VERSION");
  if (env_version && env_version[0] != '\0') {
    return std::string(env_version);
  }

  auto major_it = info.hardware_parameters.find("fri_client_sdk_major_version");
  auto minor_it = info.hardware_parameters.find("fri_client_sdk_minor_version");
  if (major_it != info.hardware_parameters.end() && minor_it != info.hardware_parameters.end()) {
    return major_it->second + "." + minor_it->second;
  }

#ifndef LBR_FRI_DEFAULT_VERSION
#define LBR_FRI_DEFAULT_VERSION "1.15"
#endif
  return std::string(LBR_FRI_DEFAULT_VERSION);
}

std::string SystemInterfaceSelector::make_class_name_(const std::string &version) const {
  std::string tag = version;
  std::replace(tag.begin(), tag.end(), '_', '.');
  std::replace(tag.begin(), tag.end(), '.', '_');
  return "lbr_ros2_control::SystemInterfaceFri_" + tag;
}

bool SystemInterfaceSelector::ensure_backend_loaded_(const hardware_interface::HardwareInfo &info) {
  if (backend_) {
    return true;
  }

  if (!loader_) {
    loader_ = std::make_unique<pluginlib::ClassLoader<hardware_interface::SystemInterface>>(
        "lbr_ros2_control", "hardware_interface::SystemInterface");
  }

  const std::string version = pick_version_string_(info);
  backend_class_name_ = make_class_name_(version);

  if (!loader_->isClassAvailable(backend_class_name_)) {
    RCLCPP_ERROR_STREAM(rclcpp::get_logger(LOGGER_NAME),
                        "Requested FRI backend '" << backend_class_name_
                                                  << "' is not available.");
    return false;
  }

  try {
    backend_ = loader_->createSharedInstance(backend_class_name_);
  } catch (const pluginlib::PluginlibException &ex) {
    RCLCPP_ERROR_STREAM(rclcpp::get_logger(LOGGER_NAME),
                        "Failed to load backend '" << backend_class_name_
                                                   << "': " << ex.what());
    return false;
  }
  return true;
}

} // namespace lbr_ros2_control

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(lbr_ros2_control::SystemInterfaceSelector,
                       hardware_interface::SystemInterface)
