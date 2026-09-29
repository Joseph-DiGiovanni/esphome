#include "litter_robot3.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

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
  if (value == nullptr || command == 'F') {
    ESP_LOGD(TAG, "ESP sent %s", command_name(command));
  } else {
    ESP_LOGD(TAG, "ESP set %s to %s", command_name(command), command_value_name(command, value));
  }
}

void LitterRobot3Component::set_cycle_delay(uint8_t minutes) {
  if (minutes > 15)
    return;
  char value[2] = {format_hex_pretty_char(minutes), '\0'};
  this->send_command('W', value);
}

void LitterRobot3Component::set_sleep(uint32_t seconds) {
  // The value is the current position in the 24h sleep cycle, not a wall-clock time.
  char value[16];
  snprintf(value, sizeof(value), "1%02u:%02u:%02u", seconds / 3600, (seconds / 60) % 60, seconds % 60);
  this->send_command('S', value);
}

void LitterRobot3Component::disable_sleep() { this->send_command('S', "0"); }

void LitterRobot3Component::loop() {
  while (this->available()) {
    char c = static_cast<char>(this->read());

    // Frames end with "\r\n"; only '\n' marks the end of a line.
    if (c == '\r')
      continue;

    if (c != '\n') {
      if (this->rx_len_ < sizeof(this->rx_line_) - 1) {
        this->rx_line_[this->rx_len_++] = c;
      } else {
        // Frame too long for the buffer: drop it and resync on the next newline.
        this->rx_len_ = 0;
      }
      continue;
    }

    // End of line: dispatch if non-empty, then start collecting the next one.
    if (this->rx_len_ > 0) {
      this->rx_line_[this->rx_len_] = '\0';
      this->handle_line_(this->rx_line_);
    }
    this->rx_len_ = 0;
  }
}

void LitterRobot3Component::handle_response_(const char *response) {
  // '(' replies "({cmd}AOK" on success or "({cmd}NOK{reason}" on failure.
  char command = response[0];
  if (command == '\0') {
    ESP_LOGW(TAG, "PIC sent an empty command response");
    return;
  }

  const char *result = &response[1];
  if (strncmp(result, "AOK", 3) == 0) {
    ESP_LOGD(TAG, "PIC acknowledged %s", command_name(command));
    return;
  }
  if (strncmp(result, "NOK", 3) == 0) {
    ESP_LOGW(TAG, "PIC rejected %s (%d)", command_name(command), atoi(&result[3]));
    return;
  }

  ESP_LOGW(TAG, "Unexpected command response: (%s", response);
}

void LitterRobot3Component::handle_line_(char *line) {
  // '(' is a command acknowledgement from the PIC, e.g. "(CAOK" or "(CNOK4".
  if (line[0] == '(') {
    this->handle_response_(&line[1]);
    return;
  }

  // ")Y####" is sent mid clean cycle and carries the cycle count.
  if (line[0] == ')' && line[1] == 'Y') {
    this->handle_cycle_count_(&line[2]);
    return;
  }

  // A status frame starts at the power-supply field, so the character after ')' is
  // '0' (AC) or '1' (battery). Pass the whole line so the field indices stay aligned.
  if (line[0] == ')' && (line[1] == '0' || line[1] == '1')) {
    this->handle_status_(line);
    return;
  }

  ESP_LOGD(TAG, "Unexpected message format: %s", line);
}

void LitterRobot3Component::handle_status_(char *line) {
  // split_tokens() terminates each field in place, so keep the raw line for logging.
  char raw[LINE_BUFFER_SIZE];
  snprintf(raw, sizeof(raw), "%s", line);

  ESP_LOGD(TAG, "PIC reported current status %s", raw);

  char *tokens[7];
  if (split_tokens(line, tokens, 7) < 7) {
    ESP_LOGW(TAG, "Malformed status packet: %s", raw);
    return;
  }

  // Fields: )P XS W N [1hh:mm:ss] L ZCCC
  this->status_callback_.call(LR3_FIELD_POWER_TYPE, static_cast<uint8_t>(tokens[0][1] - '0'));
  this->status_callback_.call(LR3_FIELD_POWER_STATE, static_cast<uint8_t>(tokens[1][0] - '0'));
  this->status_callback_.call(LR3_FIELD_SYSTEM_STATE, parse_hex_char(tokens[1][1]));
  this->status_callback_.call(LR3_FIELD_CYCLE_DELAY, parse_hex_char(tokens[2][0]));
  this->status_callback_.call(LR3_FIELD_NIGHT_LIGHT, tokens[3][0] != '0' ? 1 : 0);
  this->status_callback_.call(LR3_FIELD_SLEEP_ENABLED, tokens[4][0] != '0' ? 1 : 0);
  this->status_callback_.call(LR3_FIELD_PANEL_LOCKOUT, tokens[5][0] != '0' ? 1 : 0);
}

void LitterRobot3Component::handle_cycle_count_(const char *value) {
  char *end = nullptr;
  unsigned long count = strtoul(value, &end, 16);
  if (end == value) {
    ESP_LOGW(TAG, "Malformed cycle count packet: )Y%s", value);
    return;
  }
  ESP_LOGD(TAG, "PIC reported cycle count %lu", count);
  this->status_callback_.call(LR3_FIELD_CYCLE_COUNT, static_cast<uint32_t>(count));
}

}  // namespace esphome::litter_robot3
