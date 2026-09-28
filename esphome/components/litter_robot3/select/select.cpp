#include "select.h"

#include "esphome/core/log.h"

namespace esphome::litter_robot3 {

static const char *const TAG = "litter_robot3.select";

// Must stay in sync with the option strings generated in select/__init__.py.
static const uint8_t CYCLE_DELAY_MINUTES[] = {3, 7, 15};
static const size_t CYCLE_DELAY_OPTIONS = sizeof(CYCLE_DELAY_MINUTES) / sizeof(CYCLE_DELAY_MINUTES[0]);

void LitterRobot3CycleDelaySelect::setup() {
  this->parent_->add_status_callback([this](LitterRobot3Field field, uint32_t value) {
    if (field != LR3_FIELD_CYCLE_DELAY)
      return;
    for (size_t i = 0; i < CYCLE_DELAY_OPTIONS; i++) {
      if (CYCLE_DELAY_MINUTES[i] == value) {
        this->publish_state(i);
        return;
      }
    }
  });
}

void LitterRobot3CycleDelaySelect::control(size_t index) {
  if (index >= CYCLE_DELAY_OPTIONS)
    return;
  this->parent_->set_cycle_delay(CYCLE_DELAY_MINUTES[index]);
}

void LitterRobot3CycleDelaySelect::dump_config() { LOG_SELECT("", "Litter Robot 3 Cycle Delay", this); }

}  // namespace esphome::litter_robot3
