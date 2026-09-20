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
};

const char *system_state_name(uint8_t state);

class LitterRobot3Component final : public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void send_command(char command, const char *value = nullptr);

  bool is_powered_on() const { return this->power_state_ == 0; }
  uint8_t system_state() const { return this->system_state_; }
  bool night_light() const { return this->night_light_; }
  uint32_t cycle_count() const { return this->cycle_count_; }

  template<typename F> void add_status_callback(F &&callback) { this->status_callback_.add(std::forward<F>(callback)); }

 protected:
  void handle_line_(char *line);
  void handle_status_(char *line);
  void handle_cycle_count_(const char *value);
  void publish_field_(LitterRobot3Field field, uint32_t value);

  char rx_line_[LINE_BUFFER_SIZE];
  size_t rx_len_{0};

  uint8_t power_state_{0};
  uint8_t system_state_{0};
  bool night_light_{false};
  uint32_t cycle_count_{0};

  bool has_power_state_{false};
  bool has_system_state_{false};
  bool has_night_light_{false};
  bool has_cycle_count_{false};

  LazyCallbackManager<void(LitterRobot3Field, uint32_t)> status_callback_;
};

}  // namespace esphome::litter_robot3
