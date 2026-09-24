#include "number.h"

#include "esphome/core/log.h"

namespace esphome::litter_robot3 {

static const char *const TAG = "litter_robot3.number";

void LitterRobot3CycleDelayNumber::setup() {
  this->parent_->add_status_callback([this](LitterRobot3Field field, uint32_t value) {
    if (field == LR3_FIELD_CYCLE_DELAY)
      this->publish_state(value);
  });
}

void LitterRobot3CycleDelayNumber::control(float value) { this->parent_->set_cycle_delay(static_cast<uint8_t>(value)); }

void LitterRobot3CycleDelayNumber::dump_config() { LOG_NUMBER("", "Litter Robot 3 Cycle Delay", this); }

}  // namespace esphome::litter_robot3
