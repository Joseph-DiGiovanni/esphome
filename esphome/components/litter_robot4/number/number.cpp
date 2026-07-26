#include "esphome/core/log.h"
#include "number.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.number";

void LitterRobot4CycleDelayNumber::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_CLEAN_CYCLE_DELAY) {
      this->publish_state(value);
    }
  });
}

void LitterRobot4CycleDelayNumber::control(float value) {
  this->parent_->queue_register_write(REG_CLEAN_CYCLE_DELAY, static_cast<uint16_t>(value));
}

void LitterRobot4CycleDelayNumber::dump_config() { LOG_NUMBER("", "Litter Robot 4 Clean Cycle Delay", this); }

}  // namespace esphome::litter_robot4
