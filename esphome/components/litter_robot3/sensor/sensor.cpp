#include "sensor.h"

#include "esphome/core/log.h"

namespace esphome::litter_robot3 {

static const char *const TAG = "litter_robot3.sensor";

void LitterRobot3CycleCountSensor::setup() {
  this->parent_->add_status_callback([this](LitterRobot3Field field, uint32_t value) {
    if (field == LR3_FIELD_CYCLE_COUNT)
      this->publish_state(value);
  });
}

void LitterRobot3CycleCountSensor::dump_config() { LOG_SENSOR("", "Litter Robot 3 Waste Drawer Cycle Count", this); }

}  // namespace esphome::litter_robot3
