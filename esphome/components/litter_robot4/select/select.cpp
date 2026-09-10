#include "esphome/core/log.h"
#include "select.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.select";

void LitterRobot4NightLightModeSelect::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_NIGHT_LIGHT_MODE) {
      auto *opt = night_light_mode_name(value);
      if (opt != nullptr && this->has_option(opt)) {
        auto idx = this->index_of(opt);
        if (idx.has_value())
          this->publish_state(*idx);
      }
    }
  });
}

void LitterRobot4NightLightModeSelect::control(size_t index) {
  uint16_t value = night_light_mode_value(this->option_at(index));
  this->parent_->queue_register_write(REG_NIGHT_LIGHT_MODE, value);
}

void LitterRobot4NightLightModeSelect::dump_config() { LOG_SELECT("", "Litter Robot 4 Night Light Mode", this); }

void LitterRobot4NightLightBrightnessSelect::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_NIGHT_LIGHT_BRIGHTNESS) {
      auto *opt = brightness_name(value);
      if (opt != nullptr && this->has_option(opt)) {
        auto idx = this->index_of(opt);
        if (idx.has_value())
          this->publish_state(*idx);
      }
    }
  });
}

void LitterRobot4NightLightBrightnessSelect::control(size_t index) {
  uint16_t value = brightness_value(this->option_at(index));
  this->parent_->queue_register_write(REG_NIGHT_LIGHT_BRIGHTNESS, value);
}

void LitterRobot4NightLightBrightnessSelect::dump_config() {
  LOG_SELECT("", "Litter Robot 4 Night Light Brightness", this);
}

void LitterRobot4PanelBrightnessSelect::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_PANEL_LED) {
      auto *opt = brightness_name(value >> 8);
      if (opt != nullptr && this->has_option(opt)) {
        auto idx = this->index_of(opt);
        if (idx.has_value())
          this->publish_state(*idx);
      }
    }
  });
}

void LitterRobot4PanelBrightnessSelect::control(size_t index) {
  uint16_t value = panel_brightness_value(this->option_at(index));
  this->parent_->queue_register_write(REG_PANEL_LED, value);
}

void LitterRobot4PanelBrightnessSelect::dump_config() { LOG_SELECT("", "Litter Robot 4 Panel Brightness", this); }

void LitterRobot4WifiStatusSelect::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_WIFI_STATUS) {
      auto *opt = wifi_status_name(value);
      if (opt != nullptr && this->has_option(opt)) {
        auto idx = this->index_of(opt);
        if (idx.has_value())
          this->publish_state(*idx);
      }
    }
  });
}

void LitterRobot4WifiStatusSelect::control(size_t index) {
  uint16_t value = wifi_status_value(this->option_at(index));
  this->parent_->queue_register_write(REG_WIFI_STATUS, value);
}

void LitterRobot4WifiStatusSelect::dump_config() { LOG_SELECT("", "Litter Robot 4 WiFi Status", this); }

}  // namespace esphome::litter_robot4
