#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

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

enum RobotStatus : uint16_t {
  STATUS_POWERED_OFF = 0x01,
  STATUS_POWERING_ON = 0x02,
  STATUS_POWERING_OFF = 0x03,
  STATUS_READY = 0x04,
  STATUS_BONNET_REMOVED = 0x05,
  STATUS_CAT_DETECTED = 0x07,
  STATUS_CLEAN_CYCLE = 0x0A,
  STATUS_EMPTYING = 0x0B,
  STATUS_FILTER_REPLACEMENT = 0x0E,
  STATUS_START_CALIBRATION = 0x17,
  STATUS_CALIBRATING = 0x18,
};

static const uint8_t FRAME_LENGTH = 7;
static const uint8_t FRAME_TERMINATOR = 0xFF;
static const uint8_t MAX_PENDING = 64;
static const uint32_t PENDING_TIMEOUT = 200;

static const uint32_t SYNC_TIME_INTERVAL = 86400000;

// Register addresses
enum Register : uint8_t {
  REG_KEYPAD = 0x01,
  REG_POWER_TYPE = 0x07,
  REG_CAT_WEIGHT = 0x09,
  REG_TIME_DOW = 0x0F,
  REG_TIME_HOUR = 0x10,
  REG_TIME_MINUTE = 0x11,
  REG_TIME_SECOND = 0x12,
  REG_TIME_DAY = 0x13,
  REG_TIME_MONTH = 0x14,
  REG_TIME_YEAR = 0x15,
  REG_PANEL_LED = 0x0E,
  REG_CLEAN_CYCLE_DELAY = 0x16,
  REG_PANEL_LOCKOUT = 0x17,
  REG_NIGHT_LIGHT_MODE = 0x18,
  REG_NIGHT_LIGHT_BRIGHTNESS = 0x19,
  REG_SLEEP_DAY_MASK = 0x1D,
  REG_SLEEP_SUN = 0x1E,
  REG_WAKE_SUN = 0x1F,
  REG_SLEEP_MON = 0x20,
  REG_WAKE_MON = 0x21,
  REG_SLEEP_TUE = 0x22,
  REG_WAKE_TUE = 0x23,
  REG_SLEEP_WED = 0x24,
  REG_WAKE_WED = 0x25,
  REG_SLEEP_THU = 0x26,
  REG_WAKE_THU = 0x27,
  REG_SLEEP_FRI = 0x28,
  REG_WAKE_FRI = 0x29,
  REG_SLEEP_SAT = 0x2A,
  REG_WAKE_SAT = 0x2B,
  REG_FACTORY_RESET = 0x2D,
  REG_ROBOT_STATUS = 0x34,
  REG_FAULT_CODE = 0x35,
  REG_SLEEP_STATUS = 0x32,
  REG_BONNET_REMOVED = 0x3A,
  REG_NIGHT_LIGHT = 0x3B,
  REG_POWER_CYCLE_COUNT = 0x3D,
  REG_CLEAN_CYCLE_COUNT = 0x3E,
  REG_EMPTY_CYCLE_COUNT = 0x3F,
  REG_FILTER_CYCLE_COUNT = 0x40,
  REG_WASTE_DRAWER_PCT = 0x43,
  REG_WASTE_DRAWER_FULL = 0x44,
  REG_LITTER_LEVEL_RAW = 0x47,
};

// Command values
enum Command : uint16_t {
  CMD_EMPTY_LITTER = 0x000B,
  CMD_REPLACE_FILTER = 0x000C,
  CMD_FACTORY_RESET = 0x0001,
  CMD_START_CLEAN = 0x000A,
};

// Simulated keypad input values
enum KeypadCommand : uint16_t {
  CMD_KEYPAD_POWER = 0x0101,
  CMD_KEYPAD_RESET = 0x0401,
  CMD_KEYPAD_CYCLE = 0x0201,
  CMD_KEYPAD_EMPTY = 0x0801,
  CMD_KEYPAD_WIFI = 0x1001,
};

enum DayOfWeek : uint8_t {
  DAY_SUN,
  DAY_MON,
  DAY_TUE,
  DAY_WED,
  DAY_THU,
  DAY_FRI,
  DAY_SAT,
};

// Litter level distance mapping
static const float LITTER_EMPTY_RAW_DIST = 490.0f;
static const float LITTER_FULL_RAW_DIST = 440.0f;

struct RegisterInfo {
  Register reg;
  const char *name;
};

struct StatusInfo {
  RobotStatus status;
  const char *name;
};

const char *register_name(uint8_t reg);
const char *status_name(uint16_t status);

struct PendingOperation {
  bool is_write{false};
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
  void write_sleep_day_enabled(DayOfWeek day, bool enabled);
  void poll_registers();

#ifdef USE_TIME
  void set_time_id(time::RealTimeClock *time_id) { this->time_id_ = time_id; }
#endif

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
  void pop_queue_();
  void check_timeouts_();
  void sync_time_();

  uint8_t rx_buf_[FRAME_LENGTH];
  uint8_t rx_count_{0};

  PendingOperation op_queue_[MAX_PENDING];
  uint8_t op_head_{0}, op_tail_{0}, op_count_{0};

  LazyCallbackManager<void(uint8_t, uint16_t)> on_register_update_callback_;

  uint16_t sleep_mask_{0};
  bool api_was_connected_{false};
#ifdef USE_TIME
  time::RealTimeClock *time_id_{nullptr};
#endif
  uint32_t last_time_sync_{0};
};

}  // namespace esphome::litter_robot4
