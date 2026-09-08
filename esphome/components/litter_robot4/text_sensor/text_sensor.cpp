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
    } else if (reg == REG_BONNET_REMOVED) {
      this->bonnet_fault_ = value != 0;
      this->update_display_();
    } else if (reg == REG_ROBOT_STATUS) {
      this->robot_status_ = value;
      this->update_display_();
    }
  });
}

void LitterRobot4FaultTextSensor::update_display_() {
  const char *state = "None";

  if (this->bonnet_fault_) {
    state = "Bonnet removed";
  }

  if (this->robot_status_ == 0x0C) {
    state = "Motion detected in waste drawer";
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

void LitterRobot4HopperStatusTextSensor::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg != REG_LITTER_HOPPER)
      return;
    switch (value) {
      case LITTER_HOPPER_DISABLED:
        this->publish_state("Disabled");
        break;
      case LITTER_HOPPER_ENABLE_CMD:
      case LITTER_HOPPER_ENABLED:
        this->publish_state("Ready");
        break;
      case LITTER_HOPPER_MOTOR_START:
        this->publish_state("Running");
        break;
      case LITTER_HOPPER_NOT_CONNECTED:
        this->publish_state("Not connected");
        break;
      default:
        if ((value >> 8) == LITTER_HOPPER_STOPPED_HIGH) {
          this->publish_state("Ready");
        }
        break;
    }
  });
}

void LitterRobot4HopperStatusTextSensor::dump_config() {
  LOG_TEXT_SENSOR("", "Litter Robot 4 LitterHopper Status", this);
}

}  // namespace esphome::litter_robot4
