#include "litter_robot4.h"
#include "esphome/core/log.h"

#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4";

static const Register POLL_REGISTERS[] = {REG_POWER_TYPE,         REG_PANEL_LED,         REG_CLEAN_CYCLE_DELAY,
                                          REG_PANEL_LOCKOUT,      REG_NIGHT_LIGHT_MODE,  REG_NIGHT_LIGHT_BRIGHTNESS,
                                          REG_SLEEP_DAY_MASK,     REG_SLEEP_SUN,         REG_WAKE_SUN,
                                          REG_SLEEP_MON,          REG_WAKE_MON,          REG_SLEEP_TUE,
                                          REG_WAKE_TUE,           REG_SLEEP_WED,         REG_WAKE_WED,
                                          REG_SLEEP_THU,          REG_WAKE_THU,          REG_SLEEP_FRI,
                                          REG_WAKE_FRI,           REG_SLEEP_SAT,         REG_WAKE_SAT,
                                          REG_WIFI_STATUS,        REG_ROBOT_STATUS,      REG_FAULT_CODE,
                                          REG_SLEEP_STATUS,       REG_BONNET_REMOVED,    REG_NIGHT_LIGHT,
                                          REG_POWER_CYCLE_COUNT,  REG_CLEAN_CYCLE_COUNT, REG_EMPTY_CYCLE_COUNT,
                                          REG_FILTER_CYCLE_COUNT, REG_WASTE_DRAWER_PCT,  REG_WASTE_DRAWER_FULL,
                                          REG_LITTER_LEVEL_RAW};

static const RegisterInfo REGISTER_NAMES[] = {
    {REG_KEYPAD, "Keypad"},
    {REG_POWER_TYPE, "Power Type"},
    {REG_CAT_WEIGHT, "Cat Weight"},
    {REG_TIME_DOW, "Time Day of Week"},
    {REG_TIME_HOUR, "Time Hour"},
    {REG_TIME_MINUTE, "Time Minute"},
    {REG_TIME_SECOND, "Time Second"},
    {REG_TIME_DAY, "Time Day"},
    {REG_TIME_MONTH, "Time Month"},
    {REG_TIME_YEAR, "Time Year"},
    {REG_PANEL_LED, "Panel LED"},
    {REG_CLEAN_CYCLE_DELAY, "Clean Cycle Delay"},
    {REG_PANEL_LOCKOUT, "Control Panel Lockout"},
    {REG_NIGHT_LIGHT_MODE, "Night Light Mode"},
    {REG_NIGHT_LIGHT_BRIGHTNESS, "Night Light Brightness"},
    {REG_SLEEP_DAY_MASK, "Sleep Schedule Day Mask"},
    {REG_SLEEP_SUN, "Sunday Sleep Time"},
    {REG_WAKE_SUN, "Sunday Wake Time"},
    {REG_SLEEP_MON, "Monday Sleep Time"},
    {REG_WAKE_MON, "Monday Wake Time"},
    {REG_SLEEP_TUE, "Tuesday Sleep Time"},
    {REG_WAKE_TUE, "Tuesday Wake Time"},
    {REG_SLEEP_WED, "Wednesday Sleep Time"},
    {REG_WAKE_WED, "Wednesday Wake Time"},
    {REG_SLEEP_THU, "Thursday Sleep Time"},
    {REG_WAKE_THU, "Thursday Wake Time"},
    {REG_SLEEP_FRI, "Friday Sleep Time"},
    {REG_WAKE_FRI, "Friday Wake Time"},
    {REG_SLEEP_SAT, "Saturday Sleep Time"},
    {REG_WAKE_SAT, "Saturday Wake Time"},
    {REG_FACTORY_RESET, "Factory Reset"},
    {REG_HEARTBEAT, "Heartbeat"},
    {REG_WIFI_STATUS, "WiFi Status"},
    {REG_ROBOT_STATUS, "Robot Status"},
    {REG_FAULT_CODE, "Fault Code"},
    {REG_SLEEP_STATUS, "Sleep Status"},
    {REG_BONNET_REMOVED, "Bonnet Removed"},
    {REG_NIGHT_LIGHT, "Night Light"},
    {REG_POWER_CYCLE_COUNT, "Power Cycle Count"},
    {REG_CLEAN_CYCLE_COUNT, "Clean Cycle Count"},
    {REG_EMPTY_CYCLE_COUNT, "Empty Cycle Count"},
    {REG_FILTER_CYCLE_COUNT, "Filter Cycle Count"},
    {REG_WASTE_DRAWER_PCT, "Waste Drawer Percent"},
    {REG_WASTE_DRAWER_FULL, "Waste Drawer Full"},
    {REG_LITTER_LEVEL_RAW, "Litter Level Raw Distance"},
};

static const StatusInfo STATUS_NAMES[] = {
    {STATUS_POWERED_OFF, "Powered off"},
    {STATUS_POWERING_ON, "Powering on"},
    {STATUS_POWERING_OFF, "Powering off"},
    {STATUS_READY, "Ready"},
    {STATUS_BONNET_REMOVED, "Bonnet removed"},
    {STATUS_CAT_DETECTED, "Cat detected"},
    {STATUS_CLEAN_CYCLE, "Clean cycle in progress"},
    {STATUS_EMPTYING, "Emptying litter"},
    {STATUS_FILTER_REPLACEMENT, "Filter replacement in progress"},
    {STATUS_START_CALIBRATION, "Starting calibration"},
    {STATUS_CALIBRATING, "Calibration in progress"},
};

const char *register_name(Register reg) {
  for (const auto &info : REGISTER_NAMES) {
    if (info.reg == reg)
      return info.name;
  }
  return nullptr;
}

const char *status_name(uint16_t status) {
  for (const auto &info : STATUS_NAMES) {
    if (info.status == status)
      return info.name;
  }
  return nullptr;
}

void LitterRobot4Component::setup() {
#ifdef USE_WIFI
  this->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (wifi::global_wifi_component == nullptr) {
      return;
    }
    if (reg == REG_WIFI_STATUS) {
      this->last_wifi_status_ = static_cast<WifiStatus>(value);
      bool wifi_disabled = wifi::global_wifi_component->is_disabled();
      if (value == WIFI_OFF) {
        if (!wifi_disabled) {
          this->set_timeout("wifi_disable", 500, [this] { wifi::global_wifi_component->disable(); });
        }
      } else {
        if (wifi_disabled) {
          wifi::global_wifi_component->enable();
        }
        if (!wifi_disabled) {
          this->sync_wifi_status_();
        }
      }
    } else if (reg == REG_ROBOT_STATUS && value == STATUS_READY) {
      this->set_timeout("wifi_status_check", 500, [this] { this->queue_register_read(REG_WIFI_STATUS); });
    }
  });

#ifdef USE_WIFI_CONNECT_STATE_LISTENERS
  if (wifi::global_wifi_component != nullptr) {
    wifi::global_wifi_component->add_connect_state_listener(this);
  }
#endif
#endif
}

void LitterRobot4Component::loop() {
  bool frame_processed = false;
  while (!frame_processed && this->available()) {
    frame_processed = this->parse_byte_(this->read());
  }

  this->check_timeouts_();

#ifdef USE_TIME
  if (millis() - this->last_time_sync_ > SYNC_TIME_INTERVAL) {
    this->sync_time_();
  }
#endif

#ifdef USE_WIFI
  if (wifi::global_wifi_component != nullptr) {
    this->sync_wifi_status_();
  }
#endif
}

void LitterRobot4Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Litter Robot 4:");
#ifdef USE_TIME
  ESP_LOGCONFIG(TAG, "  Time sync: %s", this->time_id_ != nullptr ? "Enabled" : "Disabled");
#endif
}

static const char *reg_name(Register reg) {
  auto *name = register_name(reg);
  if (name)
    return name;
  static char unknown_buf[16];
  snprintf(unknown_buf, sizeof(unknown_buf), "Unknown (0x%02X)", static_cast<uint8_t>(reg));
  return unknown_buf;
}

const char *format_register_value(Register reg, uint16_t value) {
  static const char *on = "On";
  static const char *off = "Off";
  static const char *ac = "AC";
  static const char *battery = "Battery";
  static const char *none = "None";

  switch (reg) {
    case REG_KEYPAD: {
      static char keypad_buf[32];
      const char *cmd_name = nullptr;
      switch (value) {
        case CMD_KEYPAD_POWER:
          cmd_name = "Power Pressed";
          break;
        case CMD_KEYPAD_RESET:
          cmd_name = "Reset Pressed";
          break;
        case CMD_KEYPAD_CYCLE:
          cmd_name = "Cycle Pressed";
          break;
        case CMD_KEYPAD_EMPTY:
          cmd_name = "Empty Pressed";
          break;
        case CMD_KEYPAD_WIFI:
          cmd_name = "Connect Pressed";
          break;
      }
      if (cmd_name) {
        return cmd_name;
      } else {
        snprintf(keypad_buf, sizeof(keypad_buf), "%u (0x%04X)", value, value);
      }
      return keypad_buf;
    }

    case REG_FACTORY_RESET:
      if (value == CMD_FACTORY_RESET) {
        return "Reset";
      }
      break;

    case REG_POWER_TYPE:
      return value == 0 ? ac : battery;

    case REG_CAT_WEIGHT: {
      static char weight_buf[16];
      snprintf(weight_buf, sizeof(weight_buf), "%.1f lbs", static_cast<int16_t>(value) / 100.0f);
      return weight_buf;
    }

    case REG_CLEAN_CYCLE_DELAY: {
      static char delay_buf[16];
      snprintf(delay_buf, sizeof(delay_buf), "%u min", value);
      return delay_buf;
    }

    case REG_PANEL_LOCKOUT:
    case REG_BONNET_REMOVED:
    case REG_NIGHT_LIGHT:
    case REG_SLEEP_STATUS:
    case REG_WASTE_DRAWER_FULL:
      return value != 0 ? on : off;

    case REG_WASTE_DRAWER_PCT: {
      static char pct_buf[16];
      snprintf(pct_buf, sizeof(pct_buf), "%u%%", value);
      return pct_buf;
    }

    case REG_LITTER_LEVEL_RAW: {
      static char litter_buf[16];
      snprintf(litter_buf, sizeof(litter_buf), "%u mm", value);
      return litter_buf;
    }

    case REG_PANEL_LED: {
      static char panel_buf[32];
      uint8_t brightness = static_cast<uint8_t>(value >> 8);
      const char *level = brightness == 25 ? "Low" : brightness == 50 ? "Medium" : brightness == 100 ? "High" : nullptr;
      if (level) {
        snprintf(panel_buf, sizeof(panel_buf), "%s", level);
      } else {
        snprintf(panel_buf, sizeof(panel_buf), "%u (0x%04X)", value, value);
      }
      return panel_buf;
    }

    case REG_NIGHT_LIGHT_MODE: {
      const char *mode = value == 0 ? off : value == 1 ? on : value == 2 ? "Auto" : nullptr;
      if (mode) {
        return mode;
      }
      static char mode_buf[20];
      snprintf(mode_buf, sizeof(mode_buf), "%u (0x%04X)", value, value);
      return mode_buf;
    }

    case REG_NIGHT_LIGHT_BRIGHTNESS: {
      static char bright_buf[20];
      const char *level = value == 25 ? "Low" : value == 50 ? "Medium" : value == 100 ? "High" : nullptr;
      if (level) {
        snprintf(bright_buf, sizeof(bright_buf), "%s", level);
      } else {
        snprintf(bright_buf, sizeof(bright_buf), "%u (0x%04X)", value, value);
      }
      return bright_buf;
    }

    case REG_ROBOT_STATUS: {
      auto *name = status_name(value);
      static char status_buf[32];
      if (name) {
        return name;
      } else {
        snprintf(status_buf, sizeof(status_buf), "%u (0x%04X)", value, value);
      }
      return status_buf;
    }

#ifdef USE_WIFI
    case REG_WIFI_STATUS: {
      static char wifi_buf[32];
      const char *wifi_name = nullptr;
      switch (value) {
        case WIFI_OFF:
          wifi_name = "Off";
          break;
        case WIFI_PAIRING:
          wifi_name = "Pairing";
          break;
        case WIFI_CONNECTING:
          wifi_name = "Connecting";
          break;
        case WIFI_CONNECTED:
          wifi_name = "Connected";
          break;
        case WIFI_ERROR:
          wifi_name = "Error";
          break;
      }
      if (wifi_name) {
        return wifi_name;
      } else {
        snprintf(wifi_buf, sizeof(wifi_buf), "%u (0x%04X)", value, value);
      }
      return wifi_buf;
    }
#endif

    case REG_FAULT_CODE: {
      static char fault_buf[16];
      snprintf(fault_buf, sizeof(fault_buf), "%s (%u)", value != 0 ? "Fault" : none, value);
      return fault_buf;
    }

    case REG_SLEEP_SUN:
    case REG_WAKE_SUN:
    case REG_SLEEP_MON:
    case REG_WAKE_MON:
    case REG_SLEEP_TUE:
    case REG_WAKE_TUE:
    case REG_SLEEP_WED:
    case REG_WAKE_WED:
    case REG_SLEEP_THU:
    case REG_WAKE_THU:
    case REG_SLEEP_FRI:
    case REG_WAKE_FRI:
    case REG_SLEEP_SAT:
    case REG_WAKE_SAT: {
      static char time_buf[8];
      snprintf(time_buf, sizeof(time_buf), "%02u:%02u", value / 60, value % 60);
      return time_buf;
    }

    case REG_HEARTBEAT:
      return value == 99 ? "Normal" : nullptr;

    default:
      break;
  }

  static char fallback_buf[20];
  snprintf(fallback_buf, sizeof(fallback_buf), "%u (0x%04X)", value, value);
  return fallback_buf;
}

void LitterRobot4Component::push_queue_(Operation op, Register reg, uint16_t value) {
  bool is_write = op == OP_WRITE;
  if (this->pending_count_ >= MAX_PENDING) {
    ESP_LOGW(TAG, "Queue full, dropping %s for %s (0x%02X)", is_write ? "write" : "read", reg_name(reg), reg);
    return;
  }

  auto &pending_op = this->pending_queue_[this->pending_tail_];
  pending_op.op = op;
  pending_op.reg = reg;
  pending_op.value = value;
  this->pending_tail_ = (this->pending_tail_ + 1) % MAX_PENDING;
  this->pending_count_++;

  if (this->pending_count_ == 1) {
    this->pending_timestamp_ = millis();
    this->send_frame_(op, reg, value);
  }
}

// Sleep schedule bits are written optimistically to avoid a race condition if multiple switches are toggled quickly.
void LitterRobot4Component::write_sleep_day_enabled(DayOfWeek day, bool enabled) {
  if (enabled) {
    this->sleep_mask_ |= (1 << static_cast<uint8_t>(day));
  } else {
    this->sleep_mask_ &= ~(1 << static_cast<uint8_t>(day));
  }
  this->queue_register_write(REG_SLEEP_DAY_MASK, this->sleep_mask_);
}

void LitterRobot4Component::send_frame_(Operation op, Register reg, uint16_t value) {
  if (op == OP_READ) {
    ESP_LOGD(TAG, "ESP queried %s", reg_name(reg));
  } else if (op == OP_WRITE) {
    ESP_LOGD(TAG, "ESP set %s to %s", reg_name(reg), format_register_value(reg, value));
  }
  uint8_t value_high = static_cast<uint8_t>(value >> 8);
  uint8_t value_low = static_cast<uint8_t>(value & 0xFF);
  uint8_t checksum = (static_cast<uint8_t>(DIR_FROM_ESP) + static_cast<uint8_t>(op) + static_cast<uint8_t>(reg) +
                      value_high + value_low) &
                     0xFF;

  uint8_t frame[FRAME_LENGTH] = {static_cast<uint8_t>(DIR_FROM_ESP),
                                 static_cast<uint8_t>(op),
                                 static_cast<uint8_t>(reg),
                                 value_high,
                                 value_low,
                                 checksum,
                                 FRAME_TERMINATOR};

  this->write_array(frame, FRAME_LENGTH);
}

bool LitterRobot4Component::parse_byte_(uint8_t byte) {
  this->rx_buf_[this->rx_count_++] = byte;

  if (this->rx_count_ < FRAME_LENGTH) {
    return false;
  }

  if (this->rx_buf_[6] == FRAME_TERMINATOR) {
    uint8_t dir = this->rx_buf_[0];
    uint8_t op = this->rx_buf_[1];
    if ((dir == DIR_FROM_PIC || dir == DIR_FROM_ESP) && op >= OP_READ && op <= OP_WRITE_ACK) {
      uint8_t sum = dir + op + this->rx_buf_[2] + this->rx_buf_[3] + this->rx_buf_[4];
      if (sum == this->rx_buf_[5]) {
        uint16_t value = (static_cast<uint16_t>(this->rx_buf_[3]) << 8) | this->rx_buf_[4];
        this->handle_frame_(static_cast<Direction>(dir), static_cast<Operation>(op),
                            static_cast<Register>(this->rx_buf_[2]), value);
        this->rx_count_ = 0;
        return true;
      }
    }
  }

  for (uint8_t i = 0; i < FRAME_LENGTH - 1; i++) {
    this->rx_buf_[i] = this->rx_buf_[i + 1];
  }
  this->rx_count_ = FRAME_LENGTH - 1;
  return false;
}

void LitterRobot4Component::handle_frame_(Direction dir, Operation op, Register reg, uint16_t value) {
  if (dir != DIR_FROM_PIC) {
    ESP_LOGD(TAG, "Unexpected direction 0x%02X", dir);
    return;
  }

  if (!this->pic_ready_) {
    this->pic_ready_ = true;
    ESP_LOGD(TAG, "PIC Ready");
    this->set_timeout("init_poll", 500, [this] {
      this->poll_registers_();
#ifdef USE_TIME
      this->sync_time_();
#endif
    });
  }

  switch (op) {
    case OP_WRITE:
      this->handle_write_(reg, value);
      break;
    case OP_READ_REPLY:
      this->handle_read_reply_(reg, value);
      break;
    case OP_WRITE_ACK:
      this->handle_write_ack_(reg, value);
      break;
    default:
      ESP_LOGD(TAG, "Unknown operation 0x%02X", op);
      break;
  }
}

void LitterRobot4Component::handle_write_(Register reg, uint16_t value) {
  if (reg == REG_FACTORY_RESET && value == CMD_FACTORY_RESET) {
    this->pic_ready_ = false;
  }

  if (reg == REG_SLEEP_DAY_MASK) {
    this->sleep_mask_ = value;
  }

  ESP_LOGD(TAG, "PIC reported %s as %s", reg_name(reg), format_register_value(reg, value));
  this->send_frame_(OP_WRITE_ACK, reg, value);
  if (this->pending_count_ > 0 && this->pending_queue_[this->pending_head_].op == OP_WRITE &&
      this->pending_queue_[this->pending_head_].reg == reg) {
    this->pop_queue_();
  }
  this->on_register_update_callback_.call(reg, value);
}

void LitterRobot4Component::handle_read_reply_(Register reg, uint16_t value) {
  if (this->pending_count_ == 0) {
    ESP_LOGD(TAG, "Unsolicited PIC reply: %s is %s (0x%02X=0x%04X)", reg_name(reg), format_register_value(reg, value),
             reg, value);
    return;
  }

  if (reg == REG_SLEEP_DAY_MASK) {
    this->sleep_mask_ = value;
  }

  auto &pending_op = this->pending_queue_[this->pending_head_];
  if (pending_op.op == OP_WRITE) {
    ESP_LOGW(TAG, "PIC replied %s is %s but a write was expected (0x%02X=0x%04X)", reg_name(reg),
             format_register_value(reg, value), reg, value);
    return;
  }

  if (pending_op.reg != reg) {
    ESP_LOGW(TAG, "PIC replied %s is %s but %s was expected (0x%02X=0x%04X vs 0x%02X)", reg_name(reg),
             format_register_value(reg, value), reg_name(pending_op.reg), reg, value, pending_op.reg);
  }

  ESP_LOGD(TAG, "PIC replied %s is %s", reg_name(reg), format_register_value(reg, value));

  this->on_register_update_callback_.call(reg, value);
  this->pop_queue_();
}

void LitterRobot4Component::handle_write_ack_(Register reg, uint16_t value) {
  ESP_LOGD(TAG, "PIC acknowledged %s as %s", reg_name(reg), format_register_value(reg, value));

  this->pop_queue_();
  this->on_register_update_callback_.call(reg, value);
}

void LitterRobot4Component::pop_queue_() {
  if (this->pending_count_ == 0) {
    return;
  }
  this->pending_head_ = (this->pending_head_ + 1) % MAX_PENDING;
  this->pending_count_--;

  if (this->pending_count_ > 0) {
    auto &next = this->pending_queue_[this->pending_head_];
    this->pending_timestamp_ = millis();
    this->send_frame_(next.op, next.reg, next.value);
  }
}

void LitterRobot4Component::check_timeouts_() {
  while (this->pending_count_ > 0) {
    auto &pending_op = this->pending_queue_[this->pending_head_];
    if (millis() - this->pending_timestamp_ <= PENDING_TIMEOUT) {
      break;
    }
    bool is_write = pending_op.op == OP_WRITE;
    ESP_LOGW(TAG, "%s for %s timed out", is_write ? "Write" : "Read", reg_name(pending_op.reg));
    this->pop_queue_();
  }
}

void LitterRobot4Component::poll_registers_() {
  for (auto reg : POLL_REGISTERS) {
    this->queue_register_read(reg);
  }
}

#ifdef USE_TIME
void LitterRobot4Component::sync_time_() {
  if (this->time_id_ == nullptr)
    return;
  auto now = this->time_id_->now();
  if (!now.is_valid())
    return;
  // ESPHome Time uses Sunday=1, PIC uses Sunday=0.
  this->queue_register_write(REG_TIME_DOW, now.day_of_week - 1);
  this->queue_register_write(REG_TIME_HOUR, now.hour);
  this->queue_register_write(REG_TIME_MINUTE, now.minute);
  this->queue_register_write(REG_TIME_SECOND, now.second);
  this->queue_register_write(REG_TIME_DAY, now.day_of_month);
  this->queue_register_write(REG_TIME_MONTH, now.month);
  // Original firmware writes only last 2 digits of year.
  this->queue_register_write(REG_TIME_YEAR, now.year % 100);
  this->last_time_sync_ = millis();
  ESP_LOGD(TAG, "Time synced: %04d-%02d-%02d %02d:%02d:%02d DOW=%d", now.year, now.month, now.day_of_month, now.hour,
           now.minute, now.second, now.day_of_week - 1);
}
#endif

#ifdef USE_WIFI
void LitterRobot4Component::sync_wifi_status_() {
  if (wifi::global_wifi_component == nullptr)
    return;

  WifiStatus status = WIFI_CONNECTING;
  if (wifi::global_wifi_component->is_disabled()) {
    status = WIFI_OFF;
  } else if (wifi::global_wifi_component->is_ap_active()) {
    status = WIFI_PAIRING;
  } else if (wifi::global_wifi_component->is_connected()) {
    status = WIFI_CONNECTED;
  }

  if (status == this->last_wifi_status_)
    return;

  this->last_wifi_status_ = status;
  this->queue_register_write(REG_WIFI_STATUS, status);
}

#ifdef USE_WIFI_CONNECT_STATE_LISTENERS
void LitterRobot4Component::on_wifi_connect_state(StringRef ssid, std::span<const uint8_t, 6> bssid) {
  this->sync_wifi_status_();
}
#endif
#endif

}  // namespace esphome::litter_robot4
