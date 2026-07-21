#include "esphome/core/log.h"
#include "switch.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.switch";

void LitterRobot4ControlPanelLockoutSwitch::setup() {
  this->parent_->add_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_PANEL_LOCKOUT) {
      this->publish_state(value != 0);
    }
  });
}

void LitterRobot4ControlPanelLockoutSwitch::write_state(bool state) {
  this->parent_->write_register(REG_PANEL_LOCKOUT, state ? 1 : 0);
}

void LitterRobot4ControlPanelLockoutSwitch::dump_config() {
  LOG_SWITCH("", "Litter Robot 4 Control Panel Lockout", this);
}

void LitterRobot4SleepDayEnabledSwitch::setup() {
  this->parent_->add_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_SLEEP_DAY_MASK) {
      this->publish_state((value >> this->day_) & 1);
    }
  });
}

void LitterRobot4SleepDayEnabledSwitch::write_state(bool state) {
  this->parent_->write_sleep_day_enabled(this->day_, state);
}

void LitterRobot4SleepDayEnabledSwitch::dump_config() {
  LOG_SWITCH("", "Litter Robot 4 Sleep Schedule Day Enable", this);
}

}  // namespace esphome::litter_robot4
