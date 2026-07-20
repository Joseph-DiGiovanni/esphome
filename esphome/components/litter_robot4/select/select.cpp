#include "esphome/core/log.h"
#include "select.h"
#include <cstring>

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.select";

static const char *brightness_to_option(uint16_t value) {
  if (value == 25)
    return "Low";
  if (value == 50)
    return "Medium";
  if (value == 100)
    return "High";
  return nullptr;
}

static uint16_t option_to_brightness(const char *option) {
  if (strcmp(option, "Low") == 0)
    return 25;
  if (strcmp(option, "Medium") == 0)
    return 50;
  return 100;
}

static const char *night_light_mode_to_option(uint16_t value) {
  switch (value) {
    case 0:
      return "Off";
    case 1:
      return "On";
    case 2:
      return "Auto";
    default:
      return nullptr;
  }
}

static uint16_t option_to_mode(const char *option) {
  if (strcmp(option, "Off") == 0)
    return 0;
  if (strcmp(option, "On") == 0)
    return 1;
  return 2;
}

static uint16_t option_to_panel_brightness(const char *option) {
  if (strcmp(option, "Low") == 0)
    return (25 << 8) | 15;
  if (strcmp(option, "Medium") == 0)
    return (50 << 8) | 40;
  return (100 << 8) | 90;
}

void LitterRobot4NightLightModeSelect::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    if (reg == REG_NIGHT_LIGHT_MODE) {
      auto *opt = night_light_mode_to_option(value);
      if (opt != nullptr && this->has_option(opt)) {
        auto idx = this->index_of(opt);
        if (idx.has_value())
          this->publish_state(*idx);
      }
    }
  });
}

void LitterRobot4NightLightModeSelect::control(size_t index) {
  uint16_t value = option_to_mode(this->option_at(index));
  this->parent_->write_register(REG_NIGHT_LIGHT_MODE, value);
}

void LitterRobot4NightLightModeSelect::dump_config() { LOG_SELECT("", "Litter Robot 4 Night Light Mode", this); }

void LitterRobot4NightLightBrightnessSelect::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    if (reg == REG_NIGHT_LIGHT_BRIGHTNESS) {
      auto *opt = brightness_to_option(value);
      if (opt != nullptr && this->has_option(opt)) {
        auto idx = this->index_of(opt);
        if (idx.has_value())
          this->publish_state(*idx);
      }
    }
  });
}

void LitterRobot4NightLightBrightnessSelect::control(size_t index) {
  uint16_t value = option_to_brightness(this->option_at(index));
  this->parent_->write_register(REG_NIGHT_LIGHT_BRIGHTNESS, value);
}

void LitterRobot4NightLightBrightnessSelect::dump_config() {
  LOG_SELECT("", "Litter Robot 4 Night Light Brightness", this);
}

void LitterRobot4PanelBrightnessSelect::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    if (reg == REG_PANEL_LED) {
      auto *opt = brightness_to_option(value >> 8);
      if (opt != nullptr && this->has_option(opt)) {
        auto idx = this->index_of(opt);
        if (idx.has_value())
          this->publish_state(*idx);
      }
    }
  });
}

void LitterRobot4PanelBrightnessSelect::control(size_t index) {
  uint16_t value = option_to_panel_brightness(this->option_at(index));
  this->parent_->write_register(REG_PANEL_LED, value);
}

void LitterRobot4PanelBrightnessSelect::dump_config() { LOG_SELECT("", "Litter Robot 4 Panel Brightness", this); }

}  // namespace esphome::litter_robot4
