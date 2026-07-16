#include "litter_robot4.h"
#include "esphome/core/log.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4";

void LitterRobot4Component::setup() {}

void LitterRobot4Component::loop() {
  while (this->available()) {
    this->parse_byte_(this->read());
  }
  this->check_timeouts_();
}

void LitterRobot4Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Litter Robot 4:");
  ESP_LOGCONFIG(TAG, "  Baud rate: 256000");
#ifdef USE_TIME
  ESP_LOGCONFIG(TAG, "  Time sync: %s", this->time_id_ != nullptr ? "Enabled" : "Disabled");
#endif
}

void LitterRobot4Component::read_register(uint8_t reg) {
  if (this->read_count_ >= MAX_PENDING) {
    ESP_LOGW(TAG, "Read queue full, dropping read for register 0x%02X", reg);
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
    ESP_LOGW(TAG, "Write queue full, dropping write for register 0x%02X", reg);
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
  ESP_LOGD(TAG, "PIC write: reg=0x%02X value=%u (0x%04X)", reg, value, value);
  this->send_frame_(OP_WRITE_ACK, reg, value);
  this->on_register_update_callback_.call(reg, value);
}

void LitterRobot4Component::handle_read_reply_(uint8_t reg, uint16_t value) {
  if (this->read_count_ == 0) {
    ESP_LOGD(TAG, "Unexpected read reply for register 0x%02X", reg);
    return;
  }

  auto &pending = this->read_queue_[this->read_head_];
  if (pending.reg != reg) {
    ESP_LOGW(TAG, "Read reply mismatch: expected 0x%02X got 0x%02X", pending.reg, reg);
    return;
  }

  this->on_register_update_callback_.call(reg, value);
  this->pop_read_queue_();
}

void LitterRobot4Component::handle_write_ack_(uint8_t reg, uint16_t value) {
  if (this->write_count_ == 0) {
    ESP_LOGD(TAG, "Unexpected write ack for register 0x%02X", reg);
    return;
  }

  auto &pending = this->write_queue_[this->write_head_];
  if (pending.reg != reg || pending.value != value) {
    ESP_LOGW(TAG, "Write ack mismatch: expected 0x%02X=0x%04X got 0x%02X=0x%04X", pending.reg, pending.value, reg,
             value);
    return;
  }

  ESP_LOGD(TAG, "PIC write ack: reg=0x%02X value=%u (0x%04X)", reg, value, value);

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
    ESP_LOGW(TAG, "Read timeout for register 0x%02X", pending.reg);
    this->pop_read_queue_();
  }

  while (this->write_count_ > 0) {
    auto &pending = this->write_queue_[this->write_head_];
    if (millis() - pending.timestamp <= PENDING_TIMEOUT) {
      break;
    }
    ESP_LOGW(TAG, "Write ack timeout for register 0x%02X", pending.reg);
    this->pop_write_queue_();
  }
}

}  // namespace esphome::litter_robot4
