#include "esphome/core/log.h"
#include "text_sensor.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.text_sensor";

void LitterRobot4StatusTextSensor::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    switch (reg) {
      case REG_ROBOT_STATUS:
        this->robot_status_ = value;
        break;
      case REG_FAULT_CODE:
        this->fault_code_ = value;
        break;
      case REG_POWER_TYPE:
        this->on_battery_ = value != 0;
        break;
      case REG_DETECTION_EVENT:
        switch (value) {
          case DETECTION_EVENT_LASER_CLEAR:
            this->laser_detected_ = false;
            break;
          case DETECTION_EVENT_LASER_DETECTED:
            this->laser_detected_ = true;
            break;
          case DETECTION_EVENT_WEIGHT_CLEAR:
            this->weight_detected_ = false;
            break;
          case DETECTION_EVENT_WEIGHT_DETECTED:
            this->weight_detected_ = true;
            break;
          default:
            return;
        }
        break;
#ifdef USE_WIFI
      case REG_WIFI_STATUS:
        this->wifi_disabled_ = value == WIFI_OFF;
        break;
#endif
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
  auto *str = status_name(this->robot_status_);
  if (str == nullptr) {
    return;
  }
  if (this->robot_status_ == STATUS_CAT_DETECTED && !this->laser_detected_ && !this->weight_detected_) {
    str = "Waiting to cycle";
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
