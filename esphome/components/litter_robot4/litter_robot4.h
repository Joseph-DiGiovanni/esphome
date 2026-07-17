#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
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

// Register addresses
enum Register : uint8_t {
  REG_KEYPAD = 0x01,
  REG_CAT_WEIGHT = 0x09,
  REG_PANEL_LED = 0x0E,
  REG_CLEAN_CYCLE_DELAY = 0x16,
  REG_PANEL_LOCKOUT = 0x17,
  REG_NIGHT_LIGHT_MODE = 0x18,
  REG_NIGHT_LIGHT_BRIGHTNESS = 0x19,
  REG_FACTORY_RESET = 0x2D,
  REG_ROBOT_STATUS = 0x34,
  REG_FAULT_CODE = 0x35,
  REG_CLEAN_CYCLE_COUNT = 0x3E,
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

// Litter level distance mapping
static const float LITTER_EMPTY_RAW_DIST = 490.0f;
static const float LITTER_FULL_RAW_DIST = 440.0f;

static const Register POLL_REGISTERS[] = {
    REG_ROBOT_STATUS,      REG_FAULT_CODE,        REG_CLEAN_CYCLE_COUNT, REG_WASTE_DRAWER_PCT,
    REG_WASTE_DRAWER_FULL, REG_LITTER_LEVEL_RAW,  REG_NIGHT_LIGHT_MODE,  REG_NIGHT_LIGHT_BRIGHTNESS,
    REG_PANEL_LED,         REG_CLEAN_CYCLE_DELAY, REG_PANEL_LOCKOUT};

struct RegisterInfo {
  Register reg;
  const char *name;
};

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

inline const char *register_name(uint8_t reg) {
  for (const auto &info : REGISTER_NAMES) {
    if (info.reg == reg)
      return info.name;
  }
  return nullptr;
}

struct StatusInfo {
  RobotStatus status;
  const char *name;
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

inline const char *status_name(uint16_t status) {
  for (const auto &info : STATUS_NAMES) {
    if (info.status == status)
      return info.name;
  }
  return nullptr;
}

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
  void poll_registers();

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

  bool api_was_connected_{false};
};

#ifdef USE_TEXT_SENSOR
class LitterRobot4StatusTextSensor : public text_sensor::TextSensor,
                                     public Component,
                                     public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  uint16_t robot_status_{0};
  uint16_t fault_code_{0};
  bool waste_drawer_full_{false};
  void update_display_();
};
#endif

#ifdef USE_BUTTON
class LitterRobot4CycleButton : public button::Button, public Component, public Parented<LitterRobot4Component> {
 public:
  void press_action() override { this->parent_->write_register(REG_ROBOT_STATUS, CMD_START_CLEAN); }
};

class LitterRobot4EmptyButton : public button::Button, public Component, public Parented<LitterRobot4Component> {
 public:
  void press_action() override { this->parent_->write_register(REG_ROBOT_STATUS, CMD_EMPTY_LITTER); }
};

class LitterRobot4ReplaceFilterButton : public button::Button,
                                        public Component,
                                        public Parented<LitterRobot4Component> {
 public:
  void press_action() override { this->parent_->write_register(REG_ROBOT_STATUS, CMD_REPLACE_FILTER); }
};

class LitterRobot4FactoryResetButton : public button::Button, public Component, public Parented<LitterRobot4Component> {
 public:
  void press_action() override { this->parent_->write_register(REG_FACTORY_RESET, CMD_FACTORY_RESET); }
};

class LitterRobot4PowerButton : public button::Button, public Component, public Parented<LitterRobot4Component> {
 public:
  void press_action() override { this->parent_->write_register(REG_KEYPAD, CMD_KEYPAD_POWER); }
};

class LitterRobot4ResetButton : public button::Button, public Component, public Parented<LitterRobot4Component> {
 public:
  void press_action() override { this->parent_->write_register(REG_KEYPAD, CMD_KEYPAD_RESET); }
};
#endif

#ifdef USE_SENSOR
class LitterRobot4WasteDrawerSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4LitterLevelSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4CatWeightSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4CleanCycleCountSensor : public sensor::Sensor,
                                          public Component,
                                          public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};
#endif

#ifdef USE_NUMBER
class LitterRobot4CycleDelayNumber : public number::Number, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(float value) override;
};
#endif

#ifdef USE_SELECT
class LitterRobot4NightLightModeSelect : public select::Select,
                                         public Component,
                                         public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(size_t index) override;
};

class LitterRobot4NightLightBrightnessSelect : public select::Select,
                                               public Component,
                                               public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(size_t index) override;
};

class LitterRobot4PanelBrightnessSelect : public select::Select,
                                          public Component,
                                          public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(size_t index) override;
};
#endif

#ifdef USE_SWITCH
class LitterRobot4ControlPanelLockoutSwitch : public switch_::Switch,
                                              public Component,
                                              public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void write_state(bool state) override;
};
#endif

#ifdef USE_BINARY_SENSOR
class LitterRobot4WasteDrawerFullBinarySensor : public binary_sensor::BinarySensor,
                                                public Component,
                                                public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};
#endif

}  // namespace esphome::litter_robot4
