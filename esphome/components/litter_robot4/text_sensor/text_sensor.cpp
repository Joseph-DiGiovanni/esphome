#include "esphome/core/log.h"
#include "text_sensor.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.text_sensor";

void LitterRobot4StatusTextSensor::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg != REG_ROBOT_STATUS)
      return;
    if (auto *name = status_name(value)) {
      this->publish_state(name);
    }
  });
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
