#include "esphome/core/log.h"
#include "text_sensor.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.text_sensor";

void LitterRobot4StatusTextSensor::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    switch (reg) {
      case REG_FAULT_CODE:
        this->fault_code_ = value;
        break;
      case REG_POWER_TYPE:
        this->on_battery_ = value != 0;
        break;
#ifdef USE_WIFI
      case REG_WIFI_STATUS:
        this->wifi_disabled_ = value == WIFI_OFF;
        break;
#endif
      case REG_ROBOT_STATUS:
      case REG_DETECTION_EVENT:
        break;
      default:
        return;
    }
    this->update_display_();
  });
}

void LitterRobot4StatusTextSensor::update_display_() {
  if (this->wifi_disabled_) {
    this->publish_state("WiFi disabled");
    return;
  }
  if (this->on_battery_) {
    this->publish_state("On battery");
    return;
  }
  if (this->fault_code_ != 0) {
    this->publish_state("Fault detected");
    return;
  }
  uint16_t effective_status = this->parent_->get_robot_status();
  if (effective_status == STATUS_CAT_DETECTED && this->parent_->is_false_detection_suspect()) {
    effective_status = STATUS_READY;
  }
  auto *str = status_name(effective_status);
  if (str == nullptr) {
    return;
  }
  if (effective_status == STATUS_CAT_DETECTED && !this->parent_->is_laser_detected() &&
      !this->parent_->is_weight_detected()) {
    str = "Waiting to cycle";
  }
  if (effective_status == STATUS_CLEAN_CYCLE &&
      (this->parent_->is_laser_detected() || this->parent_->is_weight_detected())) {
    str = "Cycle interrupted";
  }
  this->publish_state(str);
}

void LitterRobot4StatusTextSensor::dump_config() { LOG_TEXT_SENSOR("", "Litter Robot 4 Status", this); }

void LitterRobot4PowerTypeTextSensor::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_POWER_TYPE) {
      this->publish_state(value == 0 ? "AC" : "Battery");
    }
  });
}

void LitterRobot4PowerTypeTextSensor::dump_config() { LOG_TEXT_SENSOR("", "Litter Robot 4 Power Type", this); }

}  // namespace esphome::litter_robot4
