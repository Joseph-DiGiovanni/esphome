#include "litter_robot3.h"

#include <cstdio>
#include <cstdlib>

#include "esphome/core/log.h"

namespace esphome::litter_robot3 {

static const char *const TAG = "litter_robot3";

static size_t split_tokens(char *line, char *tokens[], size_t max_tokens) {
  size_t count = 0;
  char *p = line;
  while (count < max_tokens) {
    while (*p == ' ')
      p++;
    if (*p == '\0')
      break;
    tokens[count++] = p;
    while (*p != '\0' && *p != ' ')
      p++;
    if (*p == ' ')
      *p++ = '\0';
  }
  return count;
}

const char *system_state_name(uint8_t state) {
  switch (state) {
    case 1:
      return "Ready";
    case 2:
      return "Cat detected";
    case 3:
      return "Clean cycle in progress";
    case 5:
      return "Cycle interrupted";
    case 6:
      return "Cycle paused";
    case 7:
      return "Bonnet removed";
    case 8:
      return "Pinch detected";
    default:
      return nullptr;
  }
}

void LitterRobot3Component::setup() {
  // Mirror the stock firmware, which announces itself to the PIC about a second
  // after every reset.  Delayed so the PIC is ready to receive it.
  this->set_timeout("handshake", 1000, [this]() { this->send_command('F', "0"); });
}

void LitterRobot3Component::dump_config() { ESP_LOGCONFIG(TAG, "Litter Robot 3:"); }

void LitterRobot3Component::send_command(char command, const char *value) {
  char buffer[16];
  int length;
  if (value != nullptr)
    length = snprintf(buffer, sizeof(buffer), ")%c%s\r\n", command, value);
  else
    length = snprintf(buffer, sizeof(buffer), ")%c\r\n", command);
  if (length <= 0 || length >= static_cast<int>(sizeof(buffer)))
    return;
  this->write_array(reinterpret_cast<const uint8_t *>(buffer), length);
  if (value != nullptr)
    ESP_LOGD(TAG, "Sent )%c%s", command, value);
  else
    ESP_LOGD(TAG, "Sent )%c", command);
}

void LitterRobot3Component::set_cycle_delay(uint8_t minutes) {
  if (minutes > 15)
    return;
  char value[2] = {format_hex_pretty_char(minutes), '\0'};
  this->send_command('W', value);
}

void LitterRobot3Component::loop() {
  while (this->available()) {
    char c = static_cast<char>(this->read());
    if (c == '\r')
      continue;
    if (c == '\n') {
      if (this->rx_len_ > 0) {
        this->rx_line_[this->rx_len_] = '\0';
        this->handle_line_(this->rx_line_);
      }
      this->rx_len_ = 0;
    } else if (this->rx_len_ < sizeof(this->rx_line_) - 1) {
      this->rx_line_[this->rx_len_++] = c;
    } else {
      // Line longer than expected, drop it.
      this->rx_len_ = 0;
    }
  }
}

void LitterRobot3Component::handle_line_(char *line) {
  if (line[0] == ')') {
    if (line[1] == 'Y') {
      this->handle_cycle_count_(&line[2]);
    } else if (line[1] == '0' || line[1] == '1') {
      this->handle_status_(line);
    } else {
      ESP_LOGD(TAG, "Unhandled message: %s", line);
    }
  } else if (line[0] == '(') {
    ESP_LOGD(TAG, "Response: %s", line);
  } else {
    ESP_LOGV(TAG, "Ignoring: %s", line);
  }
}

void LitterRobot3Component::handle_status_(char *line) {
  char *tokens[7];
  if (split_tokens(line, tokens, 7) < 7) {
    ESP_LOGW(TAG, "Malformed status packet: %s", line);
    return;
  }

  uint8_t power_type = static_cast<uint8_t>(tokens[0][1] - '0');
  uint8_t power_state = static_cast<uint8_t>(tokens[1][0] - '0');
  uint8_t system_state = parse_hex_char(tokens[1][1]);
  uint8_t cycle_delay = parse_hex_char(tokens[2][0]);
  bool night_light = tokens[3][0] != '0';
  bool panel_lockout = tokens[5][0] != '0';

  if (!this->has_power_type_ || power_type != this->power_type_) {
    this->power_type_ = power_type;
    this->has_power_type_ = true;
    this->publish_field_(LR3_FIELD_POWER_TYPE, power_type);
  }
  if (!this->has_power_state_ || power_state != this->power_state_) {
    this->power_state_ = power_state;
    this->has_power_state_ = true;
    this->publish_field_(LR3_FIELD_POWER_STATE, power_state);
  }
  if (!this->has_system_state_ || system_state != this->system_state_) {
    this->system_state_ = system_state;
    this->has_system_state_ = true;
    this->publish_field_(LR3_FIELD_SYSTEM_STATE, system_state);
  }
  if (!this->has_cycle_delay_ || cycle_delay != this->cycle_delay_) {
    this->cycle_delay_ = cycle_delay;
    this->has_cycle_delay_ = true;
    this->publish_field_(LR3_FIELD_CYCLE_DELAY, cycle_delay);
  }
  if (!this->has_night_light_ || night_light != this->night_light_) {
    this->night_light_ = night_light;
    this->has_night_light_ = true;
    this->publish_field_(LR3_FIELD_NIGHT_LIGHT, night_light ? 1 : 0);
  }
  if (!this->has_panel_lockout_ || panel_lockout != this->panel_lockout_) {
    this->panel_lockout_ = panel_lockout;
    this->has_panel_lockout_ = true;
    this->publish_field_(LR3_FIELD_PANEL_LOCKOUT, panel_lockout ? 1 : 0);
  }
}

void LitterRobot3Component::handle_cycle_count_(const char *value) {
  char *end = nullptr;
  unsigned long count = strtoul(value, &end, 16);
  if (end == value)
    return;
  if (this->has_cycle_count_ && count == this->cycle_count_)
    return;
  this->cycle_count_ = static_cast<uint32_t>(count);
  this->has_cycle_count_ = true;
  this->publish_field_(LR3_FIELD_CYCLE_COUNT, this->cycle_count_);
}

void LitterRobot3Component::publish_field_(LitterRobot3Field field, uint32_t value) {
  this->status_callback_.call(field, value);
}

}  // namespace esphome::litter_robot3
