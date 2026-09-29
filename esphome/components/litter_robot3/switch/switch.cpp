#include "switch.h"

#include "esphome/core/log.h"

namespace esphome::litter_robot3 {

static const char *const TAG = "litter_robot3.switch";

void LitterRobot3Switch::setup() {
  this->parent_->add_status_callback([this](LitterRobot3Field field, uint32_t value) {
    if (field == this->field_)
      this->publish_state((value != 0) != this->invert_);
  });
}

void LitterRobot3Switch::write_state(bool state) { this->parent_->send_command(this->command_, state ? "1" : "0"); }

void LitterRobot3Switch::dump_config() { LOG_SWITCH("", "Litter Robot 3", this); }

void LitterRobot3SleepSwitch::setup() {
  this->parent_->add_status_callback([this](LitterRobot3Field field, uint32_t value) {
    if (field == LR3_FIELD_SLEEP_ENABLED)
      this->publish_state(value != 0);
  });
}

void LitterRobot3SleepSwitch::write_state(bool state) {
  // Turning on starts the 8h sleep window from now; use the set_sleep action to schedule it.
  if (state) {
    this->parent_->set_sleep(0);
  } else {
    this->parent_->disable_sleep();
  }
}

void LitterRobot3SleepSwitch::dump_config() { LOG_SWITCH("", "Litter Robot 3 Sleep", this); }

}  // namespace esphome::litter_robot3
