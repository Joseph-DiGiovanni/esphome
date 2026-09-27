#include "esphome/core/log.h"
#include "switch.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.switch";

void LitterRobot4SleepSchedule::set_day_enabled(DayOfWeek day, bool enabled) {
  if (enabled) {
    this->mask_ |= (1 << static_cast<uint8_t>(day));
  } else {
    this->mask_ &= ~(1 << static_cast<uint8_t>(day));
  }
  this->parent_->queue_register_write(REG_SLEEP_DAY_MASK, this->mask_);
}

void LitterRobot4BoolSwitch::setup() {
  this->parent_->add_poll_register(this->register_);
  this->parent_->add_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == this->register_) {
      this->publish_state(value != 0);
    }
  });
}

void LitterRobot4BoolSwitch::write_state(bool state) {
  this->parent_->queue_register_write(this->register_, state ? 1 : 0);
}

void LitterRobot4BoolSwitch::dump_config() {
  LOG_SWITCH("", "Litter Robot 4", this);
  if (auto *name = register_name(this->register_)) {
    ESP_LOGCONFIG(TAG, "  Register: %s", name);
  }
}

void LitterRobot4SleepDayEnabledSwitch::setup() {
  this->parent_->add_poll_register(REG_SLEEP_DAY_MASK);
  this->parent_->add_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_SLEEP_DAY_MASK) {
      this->schedule_->update_mask(value);
      this->publish_state((value >> static_cast<uint8_t>(this->day_)) & 1);
    }
  });
}

void LitterRobot4SleepDayEnabledSwitch::write_state(bool state) { this->schedule_->set_day_enabled(this->day_, state); }

void LitterRobot4SleepDayEnabledSwitch::dump_config() {
  LOG_SWITCH("", "Litter Robot 4 Sleep Schedule Day Enable", this);
}

void LitterRobot4PowerSwitch::setup() {
  this->parent_->add_poll_register(REG_ROBOT_STATUS);
  this->parent_->add_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg != REG_ROBOT_STATUS)
      return;
    this->publish_state(!(value == STATUS_POWERED_OFF || value == STATUS_POWERING_OFF));
    if (value == STATUS_POWERING_ON)
      this->set_timeout("power_on_poll", 3000, [this] { this->parent_->request_poll(); });
  });
}

void LitterRobot4PowerSwitch::write_state(bool state) {
  if (state == this->state) {
    return;
  }
  this->parent_->queue_register_write(REG_KEYPAD, CMD_KEYPAD_POWER);
}

void LitterRobot4PowerSwitch::dump_config() { LOG_SWITCH("", "Litter Robot 4 Power", this); }

}  // namespace esphome::litter_robot4
