#include "esphome/core/log.h"
#include "datetime.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.datetime";

void LitterRobot4Time::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    if (reg == this->register_) {
      this->hour_ = value / 60;
      this->minute_ = value % 60;
      this->publish_state();
    }
  });
}

void LitterRobot4Time::control(const datetime::TimeCall &call) {
  this->hour_ = call.get_hour().value_or(this->hour_);
  this->minute_ = call.get_minute().value_or(this->minute_);
  this->publish_state();
  uint16_t minutes = this->hour_ * 60 + this->minute_;
  this->parent_->write_register(this->register_, minutes);
}

void LitterRobot4Time::dump_config() {
  ESP_LOGCONFIG(TAG, "Litter Robot 4 Schedule Time");
  ESP_LOGCONFIG(TAG, "  Register: 0x%02X", this->register_);
}

}  // namespace esphome::litter_robot4
