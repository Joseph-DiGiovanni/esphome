#include "text_sensor.h"

#include "esphome/core/log.h"

namespace esphome::litter_robot3 {

static const char *const TAG = "litter_robot3.text_sensor";

void LitterRobot3StatusTextSensor::setup() {
  this->parent_->add_status_callback([this](LitterRobot3Field field, uint32_t value) {
    if (field != LR3_FIELD_SYSTEM_STATE)
      return;
    if (const char *name = system_state_name(static_cast<uint8_t>(value)))
      this->publish_state(name);
  });
}

void LitterRobot3StatusTextSensor::dump_config() { LOG_TEXT_SENSOR("", "Litter Robot 3 Status", this); }

}  // namespace esphome::litter_robot3
