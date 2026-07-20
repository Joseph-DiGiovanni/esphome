#include "esphome/core/log.h"
#include "number.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.number";

void LitterRobot4CycleDelayNumber::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    if (reg == REG_CLEAN_CYCLE_DELAY) {
      this->publish_state(value);
    }
  });
}

void LitterRobot4CycleDelayNumber::control(float value) {
  this->parent_->write_register(REG_CLEAN_CYCLE_DELAY, static_cast<uint16_t>(value));
}

void LitterRobot4CycleDelayNumber::dump_config() { LOG_NUMBER("", "Litter Robot 4 Clean Cycle Delay", this); }

}  // namespace esphome::litter_robot4
