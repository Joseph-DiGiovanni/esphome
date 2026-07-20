#include "esphome/core/log.h"
#include "switch.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.switch";

void LitterRobot4ControlPanelLockoutSwitch::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
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

}  // namespace esphome::litter_robot4
