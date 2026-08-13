#include "esphome/core/log.h"
#include "binary_sensor.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.binary_sensor";

void LitterRobot4WasteDrawerFullBinarySensor::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_WASTE_DRAWER_FULL) {
      this->publish_state(value != 0);
    }
  });
}

void LitterRobot4WasteDrawerFullBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "Litter Robot 4 Waste Drawer Full", this);
}

void LitterRobot4BonnetRemovedBinarySensor::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_BONNET_REMOVED) {
      this->publish_state(value != 0);
    }
  });
}

void LitterRobot4BonnetRemovedBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "Litter Robot 4 Bonnet Removed", this);
}

void LitterRobot4NightLightBinarySensor::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_NIGHT_LIGHT) {
      this->publish_state(value != 0);
    }
  });
}

void LitterRobot4NightLightBinarySensor::dump_config() { LOG_BINARY_SENSOR("", "Litter Robot 4 Night Light", this); }

void LitterRobot4SleepStatusBinarySensor::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_SLEEP_STATUS) {
      this->publish_state(value != 0);
    }
  });
}

void LitterRobot4SleepStatusBinarySensor::dump_config() { LOG_BINARY_SENSOR("", "Litter Robot 4 Sleep Status", this); }

void LitterRobot4FaultStatusBinarySensor::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_FAULT_CODE) {
      this->publish_state(value != 0);
    }
  });
}

void LitterRobot4FaultStatusBinarySensor::dump_config() { LOG_BINARY_SENSOR("", "Litter Robot 4 Fault Status", this); }

void LitterRobot4LaserDetectBinarySensor::setup() {
  this->publish_initial_state(false);
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_DETECTION_EVENT) {
      if (value == DETECTION_EVENT_LASER_DETECTED) {
        this->publish_state(true);
      } else if (value == DETECTION_EVENT_LASER_CLEAR) {
        this->publish_state(false);
      }
    }
  });
}

void LitterRobot4LaserDetectBinarySensor::dump_config() { LOG_BINARY_SENSOR("", "Litter Robot 4 Laser Detect", this); }

void LitterRobot4WeightDetectBinarySensor::setup() {
  this->publish_initial_state(false);
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_DETECTION_EVENT) {
      if (value == DETECTION_EVENT_WEIGHT_DETECTED) {
        this->publish_state(true);
      } else if (value == DETECTION_EVENT_WEIGHT_CLEAR) {
        this->publish_state(false);
      }
    }
  });
}

void LitterRobot4WeightDetectBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "Litter Robot 4 Weight Detect", this);
}

}  // namespace esphome::litter_robot4
