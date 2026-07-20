#include "esphome/core/log.h"
#include "binary_sensor.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.binary_sensor";

void LitterRobot4WasteDrawerFullBinarySensor::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    if (reg == REG_WASTE_DRAWER_FULL) {
      this->publish_state(value != 0);
    }
  });
}

void LitterRobot4WasteDrawerFullBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "Litter Robot 4 Waste Drawer Full", this);
}

}  // namespace esphome::litter_robot4
