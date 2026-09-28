#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

namespace esphome::litter_robot3 {

static const uint8_t LINE_BUFFER_SIZE = 32;

enum LitterRobot3Field : uint8_t {
  LR3_FIELD_POWER_STATE,
  LR3_FIELD_SYSTEM_STATE,
  LR3_FIELD_NIGHT_LIGHT,
  LR3_FIELD_CYCLE_COUNT,
  LR3_FIELD_POWER_TYPE,
  LR3_FIELD_CYCLE_DELAY,
  LR3_FIELD_PANEL_LOCKOUT,
};

const char *system_state_name(uint8_t state);

class LitterRobot3Component final : public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void send_command(char command, const char *value = nullptr);
  void set_cycle_delay(uint8_t minutes);

  // Parsed fields are broadcast as (field, value); consumers own their own state.
  template<typename F> void add_status_callback(F &&callback) { this->status_callback_.add(std::forward<F>(callback)); }

 protected:
  void handle_line_(char *line);
  void handle_response_(const char *response);
  void handle_status_(char *line);
  void handle_cycle_count_(const char *value);

  char rx_line_[LINE_BUFFER_SIZE];
  size_t rx_len_{0};

  LazyCallbackManager<void(LitterRobot3Field, uint32_t)> status_callback_;
};

}  // namespace esphome::litter_robot3
