#include "litter_robot4.h"
#include "esphome/core/log.h"

#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4";

void LitterRobot4Component::setup() {}

void LitterRobot4Component::loop() {
  while (this->available()) {
    this->parse_byte_(this->read());
  }
  this->check_timeouts_();
#ifdef USE_API
  // Poll registers and sync time when API connects so the frontend immediately gets fresh state after a reconnect.
  bool connected = api::global_api_server->is_connected();
  if (connected && !this->api_was_connected_) {
    this->poll_registers();
  }
  this->api_was_connected_ = connected;
#endif
}

void LitterRobot4Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Litter Robot 4:");
#ifdef USE_TIME
  ESP_LOGCONFIG(TAG, "  Time sync: %s", this->time_id_ != nullptr ? "Enabled" : "Disabled");
#endif
}

void LitterRobot4Component::read_register(uint8_t reg) {
  if (this->read_count_ >= MAX_PENDING) {
    auto *name = register_name(reg);
    if (name) {
      ESP_LOGW(TAG, "Read queue full, dropping read for %s (0x%02X)", name, reg);
    } else {
      ESP_LOGW(TAG, "Read queue full, dropping read for register 0x%02X", reg);
    }
    return;
  }

  auto &pending = this->read_queue_[this->read_tail_];
  pending.reg = reg;
  pending.timestamp = millis();
  this->read_tail_ = (this->read_tail_ + 1) % MAX_PENDING;
  this->read_count_++;

  if (this->read_count_ == 1) {
    this->send_frame_(OP_READ, reg, 0);
  }
}

void LitterRobot4Component::write_register(uint8_t reg, uint16_t value) {
  if (this->write_count_ >= MAX_PENDING) {
    auto *name = register_name(reg);
    if (name) {
      ESP_LOGW(TAG, "Write queue full, dropping write for %s (0x%02X)", name, reg);
    } else {
      ESP_LOGW(TAG, "Write queue full, dropping write for register 0x%02X", reg);
    }
    return;
  }

  auto &pending = this->write_queue_[this->write_tail_];
  pending.reg = reg;
  pending.value = value;
  pending.timestamp = millis();
  this->write_tail_ = (this->write_tail_ + 1) % MAX_PENDING;
  this->write_count_++;

  if (this->write_count_ == 1) {
    this->send_frame_(OP_WRITE, reg, value);
  }
}

void LitterRobot4Component::send_frame_(uint8_t operation, uint8_t reg, uint16_t value) {
  auto *name = register_name(reg);
  if (operation == OP_READ) {
    if (name) {
      ESP_LOGD(TAG, "ESP read: %s (0x%02X)", name, reg);
    } else {
      ESP_LOGD(TAG, "ESP read: reg=0x%02X", reg);
    }
  } else if (operation == OP_WRITE) {
    if (name) {
      ESP_LOGD(TAG, "ESP write: %s = %u (0x%04X)", name, value, value);
    } else {
      ESP_LOGD(TAG, "ESP write: reg=0x%02X value=%u (0x%04X)", reg, value, value);
    }
  }
  uint8_t value_high = static_cast<uint8_t>(value >> 8);
  uint8_t value_low = static_cast<uint8_t>(value & 0xFF);
  uint8_t checksum = (DIR_ESP_TO_PIC + operation + reg + value_high + value_low) & 0xFF;

  uint8_t frame[FRAME_LENGTH] = {
      DIR_ESP_TO_PIC, operation, reg, value_high, value_low, checksum, FRAME_TERMINATOR,
  };

  this->write_array(frame, FRAME_LENGTH);
}

void LitterRobot4Component::parse_byte_(uint8_t byte) {
  this->rx_buf_[this->rx_count_++] = byte;

  if (this->rx_count_ < FRAME_LENGTH) {
    return;
  }

  if (this->rx_buf_[6] == FRAME_TERMINATOR) {
    uint8_t dir = this->rx_buf_[0];
    uint8_t op = this->rx_buf_[1];
    if ((dir == DIR_PIC_TO_ESP || dir == DIR_ESP_TO_PIC) && op >= OP_READ && op <= OP_WRITE_ACK) {
      uint8_t sum = dir + op + this->rx_buf_[2] + this->rx_buf_[3] + this->rx_buf_[4];
      if (sum == this->rx_buf_[5]) {
        uint16_t value = (static_cast<uint16_t>(this->rx_buf_[3]) << 8) | this->rx_buf_[4];
        this->handle_frame_(dir, op, this->rx_buf_[2], value);
        this->rx_count_ = 0;
        return;
      }
    }
  }

  for (uint8_t i = 0; i < FRAME_LENGTH - 1; i++) {
    this->rx_buf_[i] = this->rx_buf_[i + 1];
  }
  this->rx_count_ = FRAME_LENGTH - 1;
}

void LitterRobot4Component::handle_frame_(uint8_t direction, uint8_t operation, uint8_t reg, uint16_t value) {
  if (direction != DIR_PIC_TO_ESP) {
    ESP_LOGD(TAG, "Unexpected direction 0x%02X", direction);
    return;
  }

  switch (operation) {
    case OP_WRITE:
      this->handle_event_(reg, value);
      break;
    case OP_READ_REPLY:
      this->handle_read_reply_(reg, value);
      break;
    case OP_WRITE_ACK:
      this->handle_write_ack_(reg, value);
      break;
    default:
      ESP_LOGD(TAG, "Unknown operation 0x%02X", operation);
      break;
  }
}

void LitterRobot4Component::handle_event_(uint8_t reg, uint16_t value) {
  if (reg == REG_FACTORY_RESET && value == CMD_FACTORY_RESET) {
    this->set_timeout(3000, [this]() { this->poll_registers(); });
  }

  auto *name = register_name(reg);
  if (name) {
    ESP_LOGD(TAG, "PIC write: %s = %u (0x%04X)", name, value, value);
  } else {
    ESP_LOGD(TAG, "PIC write: reg=0x%02X value=%u (0x%04X)", reg, value, value);
  }
  this->send_frame_(OP_WRITE_ACK, reg, value);
  this->on_register_update_callback_.call(reg, value);
}

void LitterRobot4Component::handle_read_reply_(uint8_t reg, uint16_t value) {
  if (this->read_count_ == 0) {
    auto *name = register_name(reg);
    if (name) {
      ESP_LOGD(TAG, "Unexpected read reply for %s (0x%02X)", name, reg);
    } else {
      ESP_LOGD(TAG, "Unexpected read reply for register 0x%02X", reg);
    }
    return;
  }

  auto &pending = this->read_queue_[this->read_head_];
  if (pending.reg != reg) {
    auto *exp = register_name(pending.reg);
    auto *got = register_name(reg);
    ESP_LOGW(TAG, "Read reply mismatch: expected %s (0x%02X) got %s (0x%02X)", exp ? exp : "?", pending.reg,
             got ? got : "?", reg);
    return;
  }

  auto *n = register_name(reg);
  if (n) {
    ESP_LOGD(TAG, "PIC read reply: %s = %u (0x%04X)", n, value, value);
  } else {
    ESP_LOGD(TAG, "PIC read reply: reg=0x%02X value=%u (0x%04X)", reg, value, value);
  }

  this->on_register_update_callback_.call(reg, value);
  this->pop_read_queue_();
}

void LitterRobot4Component::handle_write_ack_(uint8_t reg, uint16_t value) {
  if (this->write_count_ == 0) {
    auto *name = register_name(reg);
    if (name) {
      ESP_LOGD(TAG, "Unexpected write ack for %s (0x%02X)", name, reg);
    } else {
      ESP_LOGD(TAG, "Unexpected write ack for register 0x%02X", reg);
    }
    return;
  }

  auto &pending = this->write_queue_[this->write_head_];
  if (pending.reg != reg || pending.value != value) {
    auto *exp = register_name(pending.reg);
    auto *got = register_name(reg);
    ESP_LOGW(TAG, "Write ack mismatch: expected %s (0x%02X)=0x%04X got %s (0x%02X)=0x%04X", exp ? exp : "?",
             pending.reg, pending.value, got ? got : "?", reg, value);
    return;
  }

  auto *n = register_name(reg);
  if (n) {
    ESP_LOGD(TAG, "PIC write ack: %s = %u (0x%04X)", n, value, value);
  } else {
    ESP_LOGD(TAG, "PIC write ack: reg=0x%02X value=%u (0x%04X)", reg, value, value);
  }

  this->pop_write_queue_();
  this->on_register_update_callback_.call(reg, value);
}

void LitterRobot4Component::pop_read_queue_() {
  this->read_head_ = (this->read_head_ + 1) % MAX_PENDING;
  this->read_count_--;

  if (this->read_count_ > 0) {
    auto &next = this->read_queue_[this->read_head_];
    this->send_frame_(OP_READ, next.reg, 0);
    next.timestamp = millis();
  }
}

void LitterRobot4Component::pop_write_queue_() {
  this->write_head_ = (this->write_head_ + 1) % MAX_PENDING;
  this->write_count_--;

  if (this->write_count_ > 0) {
    auto &next = this->write_queue_[this->write_head_];
    this->send_frame_(OP_WRITE, next.reg, next.value);
    next.timestamp = millis();
  }
}

void LitterRobot4Component::check_timeouts_() {
  while (this->read_count_ > 0) {
    auto &pending = this->read_queue_[this->read_head_];
    if (millis() - pending.timestamp <= PENDING_TIMEOUT) {
      break;
    }
    auto *name = register_name(pending.reg);
    if (name) {
      ESP_LOGW(TAG, "Read timeout for %s (0x%02X)", name, pending.reg);
    } else {
      ESP_LOGW(TAG, "Read timeout for register 0x%02X", pending.reg);
    }
    this->pop_read_queue_();
  }

  while (this->write_count_ > 0) {
    auto &pending = this->write_queue_[this->write_head_];
    if (millis() - pending.timestamp <= PENDING_TIMEOUT) {
      break;
    }
    auto *name = register_name(pending.reg);
    if (name) {
      ESP_LOGW(TAG, "Write ack timeout for %s (0x%02X)", name, pending.reg);
    } else {
      ESP_LOGW(TAG, "Write ack timeout for register 0x%02X", pending.reg);
    }
    this->pop_write_queue_();
  }
}
void LitterRobot4Component::poll_registers() {
  for (auto reg : POLL_REGISTERS) {
    this->read_register(reg);
  }
}
#ifdef USE_TEXT_SENSOR
void LitterRobot4StatusTextSensor::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    switch (reg) {
      case REG_ROBOT_STATUS:
        this->robot_status_ = value;
        break;
      case REG_FAULT_CODE:
        this->fault_code_ = value;
        break;
      case REG_WASTE_DRAWER_FULL:
        this->waste_drawer_full_ = value != 0;
        break;
      default:
        return;
    }
    this->update_display_();
  });
}

void LitterRobot4StatusTextSensor::update_display_() {
  if (this->waste_drawer_full_) {
    this->publish_state("Waste drawer full");
    return;
  }
  if (this->fault_code_ != 0) {
    this->publish_state("Fault detected");
    return;
  }
  auto *str = status_name(this->robot_status_);
  if (str != nullptr) {
    this->publish_state(str);
  }
}

void LitterRobot4StatusTextSensor::dump_config() { LOG_TEXT_SENSOR("", "Litter Robot 4 Status", this); }
#endif

#ifdef USE_SENSOR
void LitterRobot4WasteDrawerSensor::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    if (reg == REG_WASTE_DRAWER_PCT) {
      this->publish_state(value);
    }
  });
}

void LitterRobot4WasteDrawerSensor::dump_config() { LOG_SENSOR("", "Litter Robot 4 Waste Drawer Level", this); }

void LitterRobot4LitterLevelSensor::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    if (reg == REG_LITTER_LEVEL_RAW) {
      float pct =
          ((LITTER_EMPTY_RAW_DIST - static_cast<float>(value)) / (LITTER_EMPTY_RAW_DIST - LITTER_FULL_RAW_DIST)) *
          100.0f;
      if (pct > 100.0f)
        pct = 100.0f;
      if (pct < 0.0f)
        pct = 0.0f;
      this->publish_state(pct);
    }
  });
}

void LitterRobot4LitterLevelSensor::dump_config() { LOG_SENSOR("", "Litter Robot 4 Litter Level", this); }

void LitterRobot4CatWeightSensor::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    if (reg == REG_CAT_WEIGHT) {
      auto weight = static_cast<int16_t>(value) / 100.0f;
      this->publish_state(weight);
    }
  });
}

void LitterRobot4CatWeightSensor::dump_config() { LOG_SENSOR("", "Litter Robot 4 Cat Weight", this); }

void LitterRobot4CleanCycleCountSensor::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    if (reg == REG_CLEAN_CYCLE_COUNT) {
      this->publish_state(value);
    }
  });
}

void LitterRobot4CleanCycleCountSensor::dump_config() { LOG_SENSOR("", "Litter Robot 4 Clean Cycle Count", this); }
#endif

#ifdef USE_NUMBER
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
#endif

#ifdef USE_SELECT
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

static uint16_t option_to_panel_brightness(const char *option) {
  if (strcmp(option, "Low") == 0)
    return (25 << 8) | 15;
  if (strcmp(option, "Medium") == 0)
    return (50 << 8) | 40;
  return (100 << 8) | 90;
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
#endif

#ifdef USE_SWITCH
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
#endif

#ifdef USE_BINARY_SENSOR
void LitterRobot4WasteDrawerFullBinarySensor::setup() {
  this->parent_->add_on_register_update_callback([this](uint8_t reg, uint16_t value) {
    if (reg == REG_WASTE_DRAWER_FULL) {
      this->publish_state(value != 0);
    }
  });
}

void LitterRobot4WasteDrawerFullBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "Litter Robot 4 Waste Drawer Full", this);
}
#endif

}  // namespace esphome::litter_robot4
