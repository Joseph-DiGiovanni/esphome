#include "litter_robot4.h"
#include "esphome/core/log.h"

#ifdef USE_API
#include "esphome/components/api/api_server.h"
#endif

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4";

static const Register POLL_REGISTERS[] = {
    REG_ROBOT_STATUS,      REG_FAULT_CODE,        REG_CLEAN_CYCLE_COUNT, REG_WASTE_DRAWER_PCT,
    REG_WASTE_DRAWER_FULL, REG_LITTER_LEVEL_RAW,  REG_NIGHT_LIGHT_MODE,  REG_NIGHT_LIGHT_BRIGHTNESS,
    REG_PANEL_LED,         REG_CLEAN_CYCLE_DELAY, REG_PANEL_LOCKOUT};

static const RegisterInfo REGISTER_NAMES[] = {
    {REG_KEYPAD, "Keypad"},
    {REG_CAT_WEIGHT, "Cat Weight"},
    {REG_PANEL_LED, "Panel LED"},
    {REG_CLEAN_CYCLE_DELAY, "Clean Cycle Delay"},
    {REG_PANEL_LOCKOUT, "Control Panel Lockout"},
    {REG_FACTORY_RESET, "Factory Reset"},
    {REG_NIGHT_LIGHT_MODE, "Night Light Mode"},
    {REG_NIGHT_LIGHT_BRIGHTNESS, "Night Light Brightness"},
    {REG_ROBOT_STATUS, "Robot Status"},
    {REG_FAULT_CODE, "Fault Code"},
    {REG_CLEAN_CYCLE_COUNT, "Clean Cycle Count"},
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

const char *register_name(uint8_t reg) {
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

void LitterRobot4Component::write_register(uint8_t reg, uint16_t value) {
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
  if (this->op_count_ == 0) {
    auto *name = register_name(reg);
    if (name) {
      ESP_LOGD(TAG, "Unexpected read reply for %s (0x%02X)", name, reg);
    } else {
      ESP_LOGD(TAG, "Unexpected read reply for register 0x%02X", reg);
    }
    return;
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

  this->on_register_update_callback_.call(reg, value);
  this->pop_queue_();
}

void LitterRobot4Component::handle_write_ack_(uint8_t reg, uint16_t value) {
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

}  // namespace esphome::litter_robot4
