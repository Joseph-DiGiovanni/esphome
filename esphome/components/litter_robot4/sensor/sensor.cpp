#include "esphome/core/log.h"
#include "sensor.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.sensor";

void LitterRobot4WasteDrawerSensor::setup() {
  this->parent_->add_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_WASTE_DRAWER_PCT) {
      this->publish_state(value);
    }
  });
}

void LitterRobot4WasteDrawerSensor::dump_config() { LOG_SENSOR("", "Litter Robot 4 Waste Drawer Level", this); }

void LitterRobot4LitterLevelSensor::setup() {
  this->parent_->add_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_LITTER_LEVEL_RAW) {
      float pct =
          ((LITTER_EMPTY_RAW_DIST - static_cast<float>(value)) / (LITTER_EMPTY_RAW_DIST - LITTER_FULL_RAW_DIST)) *
          100.0f;
      if (pct > 100.0f)
        pct = 100.0f;
      if (pct < 0.0f)
        pct = 0.0f;
      this->publish_state(pct);
    }
  });
}

void LitterRobot4LitterLevelSensor::dump_config() { LOG_SENSOR("", "Litter Robot 4 Litter Level", this); }

void LitterRobot4CatWeightSensor::setup() {
  this->parent_->add_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_CAT_WEIGHT) {
      auto weight = static_cast<int16_t>(value) / 100.0f;
      this->publish_state(weight);
    }
  });
}

void LitterRobot4CatWeightSensor::dump_config() { LOG_SENSOR("", "Litter Robot 4 Cat Weight", this); }

void LitterRobot4CleanCycleCountSensor::setup() {
  this->parent_->add_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_CLEAN_CYCLE_COUNT) {
      this->publish_state(value);
    }
  });
}

void LitterRobot4CleanCycleCountSensor::dump_config() { LOG_SENSOR("", "Litter Robot 4 Clean Cycle Count", this); }

void LitterRobot4OdometerSensor::setup() {
  this->parent_->add_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == this->register_) {
      this->publish_state(value);
    }
  });
}

void LitterRobot4OdometerSensor::dump_config() {
  auto *name = register_name(this->register_);
  if (name) {
    LOG_SENSOR("", name, this);
  }
}

}  // namespace esphome::litter_robot4
