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

void LitterRobot4FaultTextSensor::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_FAULT_CODE) {
      this->fault_code_ = value;
      this->update_display_();
    } else if (reg == REG_LITTER_HOPPER) {
      this->hopper_fault_ = value == 0x0030;
      this->update_display_();
    }
  });
}

void LitterRobot4FaultTextSensor::update_display_() {
  const char *state = "None";

  if (this->hopper_fault_) {
    state = "Hopper not detected";
  }

  if (this->fault_code_ != 0) {
    switch (this->fault_code_) {
      case 0x02:
        state = "Globe motor disconnected";
        break;
      case 0x03:
        state = "Undervoltage";
        break;
      case 0x04:
      case 0x05:
        state = "Globe motor over-torque";
        break;
      case 0x06:
        state = "Pinch detected";
        break;
      default: {
        static char unknown_buf[16];
        snprintf(unknown_buf, sizeof(unknown_buf), "Fault (0x%02X)", this->fault_code_);
        state = unknown_buf;
        break;
      }
    }
  }
  this->publish_state(state);
}

void LitterRobot4FaultTextSensor::dump_config() { LOG_TEXT_SENSOR("", "Litter Robot 4 Fault", this); }

}  // namespace esphome::litter_robot4
