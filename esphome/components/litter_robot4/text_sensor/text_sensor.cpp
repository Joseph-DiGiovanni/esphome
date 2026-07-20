#include "esphome/core/log.h"
#include "text_sensor.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.text_sensor";

void LitterRobot4StatusTextSensor::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    switch (reg) {
      case REG_ROBOT_STATUS:
        this->robot_status_ = value;
        break;
      case REG_FAULT_CODE:
        this->fault_code_ = value;
        break;
      case REG_WASTE_DRAWER_FULL:
        this->waste_drawer_full_ = value != 0;
        break;
      default:
        return;
    }
    this->update_display_();
  });
}

void LitterRobot4StatusTextSensor::update_display_() {
  if (this->fault_code_ != 0) {
    this->publish_state("Fault detected");
    return;
  }
  if (this->waste_drawer_full_) {
    this->publish_state("Waste drawer full");
    return;
  }
  auto *str = status_name(this->robot_status_);
  if (str != nullptr) {
    this->publish_state(str);
  }
}

void LitterRobot4StatusTextSensor::dump_config() { LOG_TEXT_SENSOR("", "Litter Robot 4 Status", this); }

}  // namespace esphome::litter_robot4
