#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

namespace esphome::litter_robot4 {

enum Direction : uint8_t {
  DIR_PIC_TO_ESP = 0x01,
  DIR_ESP_TO_PIC = 0x02,
};

enum Operation : uint8_t {
  OP_READ = 0x01,
  OP_WRITE = 0x02,
  OP_READ_REPLY = 0x03,
  OP_WRITE_ACK = 0x04,
};

static const uint8_t FRAME_LENGTH = 7;
static const uint8_t FRAME_TERMINATOR = 0xFF;
static const uint8_t MAX_PENDING = 64;
static const uint32_t PENDING_TIMEOUT = 200;

struct PendingRead {
  uint8_t reg{0};
  uint32_t timestamp{0};
};

struct PendingWrite {
  uint8_t reg{0};
  uint16_t value{0};
  uint32_t timestamp{0};
};

class LitterRobot4Component final : public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void read_register(uint8_t reg);
  void write_register(uint8_t reg, uint16_t value);

  template<typename F> void add_on_register_update_callback(F &&callback) {
    this->on_register_update_callback_.add(std::forward<F>(callback));
  }

 protected:
  void send_frame_(uint8_t operation, uint8_t reg, uint16_t value);
  void parse_byte_(uint8_t byte);
  void handle_frame_(uint8_t direction, uint8_t operation, uint8_t reg, uint16_t value);
  void handle_event_(uint8_t reg, uint16_t value);
  void handle_read_reply_(uint8_t reg, uint16_t value);
  void handle_write_ack_(uint8_t reg, uint16_t value);
  void pop_read_queue_();
  void pop_write_queue_();
  void check_timeouts_();

  uint8_t rx_buf_[FRAME_LENGTH];
  uint8_t rx_count_{0};

  PendingRead read_queue_[MAX_PENDING];
  PendingWrite write_queue_[MAX_PENDING];
  uint8_t read_head_{0}, read_tail_{0}, read_count_{0};
  uint8_t write_head_{0}, write_tail_{0}, write_count_{0};

  LazyCallbackManager<void(uint8_t, uint16_t)> on_register_update_callback_;
};

}  // namespace esphome::litter_robot4
