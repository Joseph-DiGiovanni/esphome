#include "litter_robot4.h"
#include "esphome/core/log.h"

#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4";

static const Register POLL_REGISTERS[] = {REG_POWER_TYPE,        REG_PANEL_LED,         REG_CLEAN_CYCLE_DELAY,
                                          REG_PANEL_LOCKOUT,     REG_NIGHT_LIGHT_MODE,  REG_NIGHT_LIGHT_BRIGHTNESS,
                                          REG_SLEEP_DAY_MASK,    REG_SLEEP_SUN,         REG_WAKE_SUN,
                                          REG_SLEEP_MON,         REG_WAKE_MON,          REG_SLEEP_TUE,
                                          REG_WAKE_TUE,          REG_SLEEP_WED,         REG_WAKE_WED,
                                          REG_SLEEP_THU,         REG_WAKE_THU,          REG_SLEEP_FRI,
                                          REG_WAKE_FRI,          REG_SLEEP_SAT,         REG_WAKE_SAT,
                                          REG_ROBOT_STATUS,      REG_FAULT_CODE,        REG_SLEEP_STATUS,
                                          REG_BONNET_REMOVED,    REG_NIGHT_LIGHT,       REG_POWER_CYCLE_COUNT,
                                          REG_CLEAN_CYCLE_COUNT, REG_EMPTY_CYCLE_COUNT, REG_FILTER_CYCLE_COUNT,
                                          REG_WASTE_DRAWER_PCT,  REG_WASTE_DRAWER_FULL, REG_LITTER_LEVEL_RAW};

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
    {REG_FACTORY_RESET, "Factory Reset"},
    {REG_WIFI_STATUS, "WiFi Status"},
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
  this->add_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_WIFI_STATUS) {
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
    }
  });
#endif

#ifdef USE_WIFI_CONNECT_STATE_LISTENERS
  if (wifi::global_wifi_component != nullptr) {
    wifi::global_wifi_component->add_connect_state_listener(this);
  }
#endif
}

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
    this->sync_time_();
#ifdef USE_WIFI
    this->sync_wifi_status_();
#endif
  }
  this->api_was_connected_ = connected;
#endif

#ifdef USE_TIME
  if (this->time_id_ != nullptr && millis() - this->last_time_sync_ > SYNC_TIME_INTERVAL) {
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
  ESP_LOGCONFIG(TAG, "  Baud rate: 256000");
#ifdef USE_TIME
  ESP_LOGCONFIG(TAG, "  Time sync: %s", this->time_id_ != nullptr ? "Enabled" : "Disabled");
#endif
}

void LitterRobot4Component::read_register(Register reg) {
  if (this->op_count_ >= MAX_PENDING) {
    auto *name = register_name(reg);
    if (name) {
      ESP_LOGW(TAG, "Operation queue full, dropping read for %s (0x%02X)", name, reg);
    } else {
      ESP_LOGW(TAG, "Operation queue full, dropping read for register 0x%02X", reg);
    }
    return;
  }

  auto &op = this->op_queue_[this->op_tail_];
  op.is_write = false;
  op.reg = reg;
  op.timestamp = millis();
  this->op_tail_ = (this->op_tail_ + 1) % MAX_PENDING;
  this->op_count_++;

  if (this->op_count_ == 1) {
    this->send_frame_(OP_READ, reg, 0);
  }
}

void LitterRobot4Component::write_register(Register reg, uint16_t value) {
  if (this->op_count_ >= MAX_PENDING) {
    auto *name = register_name(reg);
    if (name) {
      ESP_LOGW(TAG, "Operation queue full, dropping write for %s (0x%02X)", name, reg);
    } else {
      ESP_LOGW(TAG, "Operation queue full, dropping write for register 0x%02X", reg);
    }
    return;
  }

  auto &op = this->op_queue_[this->op_tail_];
  op.is_write = true;
  op.reg = reg;
  op.value = value;
  op.timestamp = millis();
  this->op_tail_ = (this->op_tail_ + 1) % MAX_PENDING;
  this->op_count_++;

  if (this->op_count_ == 1) {
    this->send_frame_(OP_WRITE, reg, value);
  }
}

// Sleep schedule bits are written optimistically to avoid a race condition if multiple switches are toggled quickly.
void LitterRobot4Component::write_sleep_day_enabled(DayOfWeek day, bool enabled) {
  if (enabled) {
    this->sleep_mask_ |= (1 << static_cast<uint8_t>(day));
  } else {
    this->sleep_mask_ &= ~(1 << static_cast<uint8_t>(day));
  }
  this->write_register(REG_SLEEP_DAY_MASK, this->sleep_mask_);
}

void LitterRobot4Component::send_frame_(Operation op, Register reg, uint16_t value) {
  auto *name = register_name(reg);
  if (op == OP_READ) {
    if (name) {
      ESP_LOGD(TAG, "ESP read: %s (0x%02X)", name, reg);
    } else {
      ESP_LOGD(TAG, "ESP read: reg=0x%02X", reg);
    }
  } else if (op == OP_WRITE) {
    if (name) {
      ESP_LOGD(TAG, "ESP write: %s = %u (0x%04X)", name, value, value);
    } else {
      ESP_LOGD(TAG, "ESP write: reg=0x%02X value=%u (0x%04X)", reg, value, value);
    }
  }
  uint8_t value_high = static_cast<uint8_t>(value >> 8);
  uint8_t value_low = static_cast<uint8_t>(value & 0xFF);
  uint8_t checksum = (static_cast<uint8_t>(DIR_ESP_TO_PIC) + static_cast<uint8_t>(op) + static_cast<uint8_t>(reg) +
                      value_high + value_low) &
                     0xFF;

  uint8_t frame[FRAME_LENGTH] = {static_cast<uint8_t>(DIR_ESP_TO_PIC),
                                 static_cast<uint8_t>(op),
                                 static_cast<uint8_t>(reg),
                                 value_high,
                                 value_low,
                                 checksum,
                                 FRAME_TERMINATOR};

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
        this->handle_frame_(static_cast<Direction>(dir), static_cast<Operation>(op),
                            static_cast<Register>(this->rx_buf_[2]), value);
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

void LitterRobot4Component::handle_frame_(Direction dir, Operation op, Register reg, uint16_t value) {
  if (dir != DIR_PIC_TO_ESP) {
    ESP_LOGD(TAG, "Unexpected direction 0x%02X", dir);
    return;
  }

  switch (op) {
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
      ESP_LOGD(TAG, "Unknown operation 0x%02X", op);
      break;
  }
}

void LitterRobot4Component::handle_event_(Register reg, uint16_t value) {
  if (reg == REG_FACTORY_RESET && value == CMD_FACTORY_RESET) {
    this->set_timeout(3000, [this]() {
      this->poll_registers();
      this->sync_time_();
    });
  }

  if (reg == REG_SLEEP_DAY_MASK) {
    this->sleep_mask_ = value;
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

void LitterRobot4Component::handle_read_reply_(Register reg, uint16_t value) {
  if (this->op_count_ == 0) {
    auto *name = register_name(reg);
    if (name) {
      ESP_LOGD(TAG, "Unexpected read reply for %s (0x%02X)", name, reg);
    } else {
      ESP_LOGD(TAG, "Unexpected read reply for register 0x%02X", reg);
    }
    return;
  }

  if (reg == REG_SLEEP_DAY_MASK) {
    this->sleep_mask_ = value;
  }

  auto &op = this->op_queue_[this->op_head_];
  if (op.is_write) {
    auto *got = register_name(reg);
    ESP_LOGW(TAG, "Unexpected read reply for %s (0x%02X): queue head is a write", got ? got : "?", reg);
    return;
  }

  if (op.reg != reg) {
    auto *exp = register_name(op.reg);
    auto *got = register_name(reg);
    ESP_LOGW(TAG, "Read reply mismatch: expected %s (0x%02X) got %s (0x%02X)", exp ? exp : "?", op.reg, got ? got : "?",
             reg);
    return;
  }

  auto *n = register_name(reg);
  if (n) {
    ESP_LOGD(TAG, "PIC read reply: %s = %u (0x%04X)", n, value, value);
  } else {
    ESP_LOGD(TAG, "PIC read reply: reg=0x%02X value=%u (0x%04X)", reg, value, value);
  }

  if (reg == REG_SLEEP_DAY_MASK) {
    this->sleep_mask_ = value;
  }

  this->on_register_update_callback_.call(reg, value);
  this->pop_queue_();
}

void LitterRobot4Component::handle_write_ack_(Register reg, uint16_t value) {
  if (this->op_count_ == 0) {
    auto *name = register_name(reg);
    if (name) {
      ESP_LOGD(TAG, "Unexpected write ack for %s (0x%02X)", name, reg);
    } else {
      ESP_LOGD(TAG, "Unexpected write ack for register 0x%02X", reg);
    }
    return;
  }

  auto &op = this->op_queue_[this->op_head_];
  if (!op.is_write) {
    auto *got = register_name(reg);
    ESP_LOGW(TAG, "Unexpected write ack for %s (0x%02X): queue head is a read", got ? got : "?", reg);
    return;
  }

  if (op.reg != reg || op.value != value) {
    auto *exp = register_name(op.reg);
    auto *got = register_name(reg);
    ESP_LOGW(TAG, "Write ack mismatch: expected %s (0x%02X)=0x%04X got %s (0x%02X)=0x%04X", exp ? exp : "?", op.reg,
             op.value, got ? got : "?", reg, value);
    return;
  }

  auto *n = register_name(reg);
  if (n) {
    ESP_LOGD(TAG, "PIC write ack: %s = %u (0x%04X)", n, value, value);
  } else {
    ESP_LOGD(TAG, "PIC write ack: reg=0x%02X value=%u (0x%04X)", reg, value, value);
  }

  this->pop_queue_();
  this->on_register_update_callback_.call(reg, value);
}

void LitterRobot4Component::pop_queue_() {
  this->op_head_ = (this->op_head_ + 1) % MAX_PENDING;
  this->op_count_--;

  if (this->op_count_ > 0) {
    auto &next = this->op_queue_[this->op_head_];
    if (next.is_write) {
      this->send_frame_(OP_WRITE, next.reg, next.value);
    } else {
      this->send_frame_(OP_READ, next.reg, 0);
    }
    next.timestamp = millis();
  }
}

void LitterRobot4Component::check_timeouts_() {
  while (this->op_count_ > 0) {
    auto &op = this->op_queue_[this->op_head_];
    if (millis() - op.timestamp <= PENDING_TIMEOUT) {
      break;
    }
    auto *name = register_name(op.reg);
    if (op.is_write) {
      if (name) {
        ESP_LOGW(TAG, "Write ack timeout for %s (0x%02X)", name, op.reg);
      } else {
        ESP_LOGW(TAG, "Write ack timeout for register 0x%02X", op.reg);
      }
    } else {
      if (name) {
        ESP_LOGW(TAG, "Read timeout for %s (0x%02X)", name, op.reg);
      } else {
        ESP_LOGW(TAG, "Read timeout for register 0x%02X", op.reg);
      }
    }
    this->pop_queue_();
  }
}

void LitterRobot4Component::poll_registers() {
  for (auto reg : POLL_REGISTERS) {
    this->read_register(reg);
  }
}

void LitterRobot4Component::sync_time_() {
#ifdef USE_TIME
  if (this->time_id_ == nullptr)
    return;
  auto now = this->time_id_->now();
  if (!now.is_valid())
    return;
  // ESPHome Time uses Sunday=1, PIC uses Sunday=0.
  this->write_register(REG_TIME_DOW, now.day_of_week - 1);
  this->write_register(REG_TIME_HOUR, now.hour);
  this->write_register(REG_TIME_MINUTE, now.minute);
  this->write_register(REG_TIME_SECOND, now.second);
  this->write_register(REG_TIME_DAY, now.day_of_month);
  this->write_register(REG_TIME_MONTH, now.month);
  // Original firmware writes only last 2 digits of year.
  this->write_register(REG_TIME_YEAR, now.year % 100);
  this->last_time_sync_ = millis();
  ESP_LOGD(TAG, "Time synced: %04d-%02d-%02d %02d:%02d:%02d DOW=%d", now.year, now.month, now.day_of_month, now.hour,
           now.minute, now.second, now.day_of_week - 1);
#endif
}

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
  this->write_register(REG_WIFI_STATUS, status);
}
#endif

#ifdef USE_WIFI_CONNECT_STATE_LISTENERS
void LitterRobot4Component::on_wifi_connect_state(StringRef ssid, std::span<const uint8_t, 6> bssid) {
  this->sync_wifi_status_();
}
#endif

}  // namespace esphome::litter_robot4
