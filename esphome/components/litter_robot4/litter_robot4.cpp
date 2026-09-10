#include "litter_robot4.h"
#include "esphome/core/log.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4";

static const Register POLL_REGISTERS[] = {REG_DEBUG,
                                          REG_POWER_TYPE,
                                          REG_PANEL_LED,
                                          REG_CLEAN_CYCLE_DELAY,
                                          REG_LITTER_HOPPER,
                                          REG_PANEL_LOCKOUT,
                                          REG_NIGHT_LIGHT_MODE,
                                          REG_NIGHT_LIGHT_BRIGHTNESS,
                                          REG_SLEEP_DAY_MASK,
                                          REG_SLEEP_SUN,
                                          REG_WAKE_SUN,
                                          REG_SLEEP_MON,
                                          REG_WAKE_MON,
                                          REG_SLEEP_TUE,
                                          REG_WAKE_TUE,
                                          REG_SLEEP_WED,
                                          REG_WAKE_WED,
                                          REG_SLEEP_THU,
                                          REG_WAKE_THU,
                                          REG_SLEEP_FRI,
                                          REG_WAKE_FRI,
                                          REG_SLEEP_SAT,
                                          REG_WAKE_SAT,
                                          REG_WIFI_STATUS,
                                          REG_ROBOT_STATUS,
                                          REG_FAULT_CODE,
                                          REG_SLEEPING,
                                          REG_BONNET_REMOVED,
                                          REG_NIGHT_LIGHT,
                                          REG_POWER_CYCLE_COUNT,
                                          REG_CLEAN_CYCLE_COUNT,
                                          REG_EMPTY_CYCLE_COUNT,
                                          REG_FILTER_CYCLE_COUNT,
                                          REG_WASTE_DRAWER_PCT,
                                          REG_WASTE_DRAWER_FULL,
                                          REG_LITTER_LEVEL_RAW,
                                          REG_CALIBRATED_LITTER_LEVEL_RAW};

static const char *reg_name(Register reg) {
  auto *name = register_name(reg);
  if (name)
    return name;
  static char unknown_buf[16];
  snprintf(unknown_buf, sizeof(unknown_buf), "Unknown (0x%02X)", static_cast<uint8_t>(reg));
  return unknown_buf;
}

void LitterRobot4Component::loop() {
  bool frame_processed = false;
  while (!frame_processed && this->available()) {
    frame_processed = this->parse_byte_(this->read());
  }

  this->check_timeouts_();
}

void LitterRobot4Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Litter Robot 4:");
#ifdef USE_TIME
  ESP_LOGCONFIG(TAG, "  Time sync: %s", this->time_id_ != nullptr ? "Enabled" : "Disabled");
#endif
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
    this->set_timeout("init_poll", 500, [this] { this->poll_registers_(); });
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

  if (reg == REG_ROBOT_STATUS && value == STATUS_POWERING_ON) {
    this->set_timeout("power_on_poll", 3000, [this] { this->poll_registers_(); });
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

void LitterRobot4Component::sync_time() {
#ifdef USE_TIME
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
  ESP_LOGD(TAG, "Time synced: %04d-%02d-%02d %02d:%02d:%02d DOW=%d", now.year, now.month, now.day_of_month, now.hour,
           now.minute, now.second, now.day_of_week - 1);
#endif
}

}  // namespace esphome::litter_robot4
