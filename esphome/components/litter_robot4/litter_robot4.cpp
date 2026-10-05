#include "litter_robot4.h"
#include "esphome/core/log.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4";

static const char *reg_name(Register reg, char *buf, size_t buf_size) {
  auto *name = register_name(reg);
  if (name)
    return name;
  snprintf(buf, buf_size, "Unknown (0x%02X)", static_cast<uint8_t>(reg));
  return buf;
}

void LitterRobot4Component::loop() {
  bool frame_processed = false;
  while (!frame_processed && this->available()) {
    frame_processed = this->parse_byte_(this->read());
  }

  this->check_timeouts_();
}

void LitterRobot4Component::dump_config() { ESP_LOGCONFIG(TAG, "Litter Robot 4:"); }

void LitterRobot4Component::add_poll_register(Register reg) {
  for (auto existing : this->poll_registers_) {
    if (existing == reg)
      return;
  }
  this->poll_registers_.push_back(reg);
}

void LitterRobot4Component::request_poll() {
  for (auto reg : this->poll_registers_)
    this->queue_register_read(reg);
}

void LitterRobot4Component::push_queue_(Operation op, Register reg, uint16_t value) {
  bool is_write = op == OP_WRITE;
  if (this->pending_count_ >= MAX_PENDING) {
    char name[FORMAT_BUFFER_SIZE];
    ESP_LOGW(TAG, "Queue full, dropping %s for %s (0x%02X)", is_write ? "write" : "read",
             reg_name(reg, name, sizeof(name)), reg);
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

void LitterRobot4Component::send_frame_(Operation op, Register reg, uint16_t value) {
  if (op == OP_READ) {
    char name[FORMAT_BUFFER_SIZE];
    ESP_LOGD(TAG, "ESP queried %s", reg_name(reg, name, sizeof(name)));
  } else if (op == OP_WRITE) {
    char name[FORMAT_BUFFER_SIZE];
    char value_str[FORMAT_BUFFER_SIZE];
    ESP_LOGD(TAG, "ESP set %s to %s", reg_name(reg, name, sizeof(name)),
             format_register_value(reg, value, value_str, sizeof(value_str)));
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
    this->set_timeout("init_poll", 500, [this] { this->request_poll(); });
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
  char name[FORMAT_BUFFER_SIZE];
  char value_str[FORMAT_BUFFER_SIZE];
  ESP_LOGD(TAG, "PIC reported %s as %s", reg_name(reg, name, sizeof(name)),
           format_register_value(reg, value, value_str, sizeof(value_str)));
  this->send_frame_(OP_WRITE_ACK, reg, value);
  if (this->pending_count_ > 0 && this->pending_queue_[this->pending_head_].op == OP_WRITE &&
      this->pending_queue_[this->pending_head_].reg == reg) {
    this->pop_queue_();
  }
  this->on_register_update_callback_.call(reg, value);
}

void LitterRobot4Component::handle_read_reply_(Register reg, uint16_t value) {
  if (this->pending_count_ == 0) {
    char name[FORMAT_BUFFER_SIZE];
    char value_str[FORMAT_BUFFER_SIZE];
    ESP_LOGD(TAG, "Unsolicited PIC reply: %s is %s (0x%02X=0x%04X)", reg_name(reg, name, sizeof(name)),
             format_register_value(reg, value, value_str, sizeof(value_str)), reg, value);
    return;
  }

  auto &pending_op = this->pending_queue_[this->pending_head_];
  if (pending_op.op == OP_WRITE) {
    char name[FORMAT_BUFFER_SIZE];
    char value_str[FORMAT_BUFFER_SIZE];
    ESP_LOGW(TAG, "PIC replied %s is %s but a write was expected (0x%02X=0x%04X)", reg_name(reg, name, sizeof(name)),
             format_register_value(reg, value, value_str, sizeof(value_str)), reg, value);
    return;
  }

  if (pending_op.reg != reg) {
    char actual_name[FORMAT_BUFFER_SIZE];
    char expected_name[FORMAT_BUFFER_SIZE];
    char value_str[FORMAT_BUFFER_SIZE];
    ESP_LOGW(TAG, "PIC replied %s is %s but %s was expected (0x%02X=0x%04X vs 0x%02X)",
             reg_name(reg, actual_name, sizeof(actual_name)),
             format_register_value(reg, value, value_str, sizeof(value_str)),
             reg_name(pending_op.reg, expected_name, sizeof(expected_name)), reg, value, pending_op.reg);
  }

  char name[FORMAT_BUFFER_SIZE];
  char value_str[FORMAT_BUFFER_SIZE];
  ESP_LOGD(TAG, "PIC replied %s is %s", reg_name(reg, name, sizeof(name)),
           format_register_value(reg, value, value_str, sizeof(value_str)));

  this->on_register_update_callback_.call(reg, value);
  this->pop_queue_();
}

void LitterRobot4Component::handle_write_ack_(Register reg, uint16_t value) {
  char name[FORMAT_BUFFER_SIZE];
  char value_str[FORMAT_BUFFER_SIZE];
  ESP_LOGD(TAG, "PIC acknowledged %s as %s", reg_name(reg, name, sizeof(name)),
           format_register_value(reg, value, value_str, sizeof(value_str)));

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
    char name[FORMAT_BUFFER_SIZE];
    ESP_LOGW(TAG, "%s for %s timed out", is_write ? "Write" : "Read",
             reg_name(pending_op.reg, name, sizeof(name)));
    this->pop_queue_();
  }
}

}  // namespace esphome::litter_robot4
