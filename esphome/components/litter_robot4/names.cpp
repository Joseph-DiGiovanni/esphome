#include "litter_robot4.h"

#include <cstring>

namespace esphome::litter_robot4 {

struct RegisterInfo {
  Register reg;
  const char *name;
};

struct StatusInfo {
  RobotStatus status;
  const char *name;
};

struct FaultInfo {
  GlobeFaultCode code;
  const char *name;
};

struct WifiStatusInfo {
  WifiStatus status;
  const char *name;
};

struct DetectionEventInfo {
  DetectionEvent event;
  const char *name;
};

struct KeypadCommandInfo {
  KeypadCommand command;
  const char *name;
};

struct BrightnessInfo {
  uint16_t value;
  const char *name;
};

struct NightLightModeInfo {
  NightLightMode mode;
  const char *name;
};

struct PanelBrightnessInfo {
  const char *name;
  uint16_t value;
};

static const RegisterInfo REGISTER_NAMES[] = {
    {REG_KEYPAD, "Keypad"},
    {REG_DEBUG, "Debug"},
    {REG_POWER_TYPE, "Power type"},
    {REG_CAT_WEIGHT, "Cat weight"},
    {REG_LITTER_HOPPER, "LitterHopper"},
    {REG_TIME_DOW, "Time day of week"},
    {REG_TIME_HOUR, "Time hour"},
    {REG_TIME_MINUTE, "Time minute"},
    {REG_TIME_SECOND, "Time second"},
    {REG_TIME_DAY, "Time day"},
    {REG_TIME_MONTH, "Time month"},
    {REG_TIME_YEAR, "Time year"},
    {REG_PANEL_LED, "Panel LED"},
    {REG_CLEAN_CYCLE_DELAY, "Clean cycle delay"},
    {REG_PANEL_LOCKOUT, "Control panel lockout"},
    {REG_NIGHT_LIGHT_MODE, "Night light mode"},
    {REG_NIGHT_LIGHT_BRIGHTNESS, "Night light brightness"},
    {REG_SLEEP_DAY_MASK, "Sleep schedule day mask"},
    {REG_SLEEP_SUN, "Sunday sleep time"},
    {REG_WAKE_SUN, "Sunday wake time"},
    {REG_SLEEP_MON, "Monday sleep time"},
    {REG_WAKE_MON, "Monday wake time"},
    {REG_SLEEP_TUE, "Tuesday sleep time"},
    {REG_WAKE_TUE, "Tuesday wake time"},
    {REG_SLEEP_WED, "Wednesday sleep time"},
    {REG_WAKE_WED, "Wednesday wake time"},
    {REG_SLEEP_THU, "Thursday sleep time"},
    {REG_WAKE_THU, "Thursday wake time"},
    {REG_SLEEP_FRI, "Friday sleep time"},
    {REG_WAKE_FRI, "Friday wake time"},
    {REG_SLEEP_SAT, "Saturday sleep time"},
    {REG_WAKE_SAT, "Saturday wake time"},
    {REG_FACTORY_RESET, "Factory reset"},
    {REG_HEARTBEAT, "Heartbeat"},
    {REG_WIFI_STATUS, "WiFi status"},
    {REG_ROBOT_STATUS, "Robot status"},
    {REG_FAULT_CODE, "Fault code"},
    {REG_SLEEPING, "Sleep state"},
    {REG_DETECTION_EVENT, "Detection event"},
    {REG_BONNET_REMOVED, "Bonnet removed"},
    {REG_NIGHT_LIGHT, "Night light"},
    {REG_POWER_CYCLE_COUNT, "Power cycle count"},
    {REG_CLEAN_CYCLE_COUNT, "Clean cycle count"},
    {REG_EMPTY_CYCLE_COUNT, "Empty cycle count"},
    {REG_FILTER_CYCLE_COUNT, "Filter cycle count"},
    {REG_WASTE_DRAWER_PCT, "Waste drawer percent"},
    {REG_WASTE_DRAWER_FULL, "Waste drawer full"},
    {REG_LITTER_LEVEL_RAW, "Litter level raw distance"},
    {REG_LASER_LEFT, "Laser sensor left"},
    {REG_LASER_CENTER, "Laser sensor center"},
    {REG_LASER_RIGHT, "Laser sensor right"},
    {REG_CALIBRATED_LITTER_LEVEL_RAW, "Calibrated litter level raw distance"},
    {REG_DEBUG_LIGHT_LEVEL, "Light level"},
    {REG_DEBUG_WEIGHT, "Weight"},
    {REG_DEBUG_LASER_LEFT, "Laser sensor left"},
    {REG_DEBUG_LASER_CENTER, "Laser sensor center"},
    {REG_DEBUG_LASER_RIGHT, "Laser sensor right"},
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

static const FaultInfo FAULT_NAMES[] = {
    {FAULT_CLEAR, "None"},
    {FAULT_GLOBE_DISCONNECT, "Globe motor disconnected"},
    {FAULT_UNDERVOLTAGE, "Undervoltage"},
    {FAULT_GLOBE_OVERTORQUE_AMP, "Globe motor over-torque"},
    {FAULT_GLOBE_OVERTORQUE_SLOPE, "Globe motor over-torque"},
    {FAULT_PINCH, "Pinch detected"},
};

// clang-format off
static const WifiStatusInfo WIFI_STATUS_NAMES[] = {
    {WIFI_OFF, "Off"},
    {WIFI_PAIRING, "Pairing"},
    {WIFI_CONNECTING, "Connecting"},
    {WIFI_CONNECTED, "Connected"},
    {WIFI_ERROR, "Error"},
};
// clang-format on

static const DetectionEventInfo DETECTION_EVENT_NAMES[] = {
    {DETECTION_EVENT_LASER_CLEAR, "Laser clear"},
    {DETECTION_EVENT_LASER_DETECTED, "Laser triggered"},
    {DETECTION_EVENT_WEIGHT_CLEAR, "Weight clear"},
    {DETECTION_EVENT_WEIGHT_DETECTED, "Weight detected"},
};

// clang-format off
static const KeypadCommandInfo KEYPAD_COMMAND_NAMES[] = {
    {CMD_KEYPAD_POWER, "Power pressed"},
    {CMD_KEYPAD_RESET, "Reset pressed"},
    {CMD_KEYPAD_CYCLE, "Cycle pressed"},
    {CMD_KEYPAD_EMPTY, "Empty pressed"},
    {CMD_KEYPAD_WIFI, "Connect pressed"},
};
// clang-format on

static const BrightnessInfo BRIGHTNESS_NAMES[] = {
    {BRIGHTNESS_LOW, "Low"},
    {BRIGHTNESS_MEDIUM, "Medium"},
    {BRIGHTNESS_HIGH, "High"},
};

static const NightLightModeInfo NIGHT_LIGHT_MODE_NAMES[] = {
    {NIGHT_LIGHT_OFF, "Off"},
    {NIGHT_LIGHT_ON, "On"},
    {NIGHT_LIGHT_AUTO, "Auto"},
};

static const PanelBrightnessInfo PANEL_BRIGHTNESS_NAMES[] = {
    {"Low", (BRIGHTNESS_LOW << 8) | PANEL_DIM_LOW},
    {"Medium", (BRIGHTNESS_MEDIUM << 8) | PANEL_DIM_MEDIUM},
    {"High", (BRIGHTNESS_HIGH << 8) | PANEL_DIM_HIGH},
};

const char *register_name(Register reg) {
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

const char *wifi_status_name(uint16_t status) {
  for (const auto &info : WIFI_STATUS_NAMES) {
    if (info.status == status)
      return info.name;
  }
  return nullptr;
}

uint16_t wifi_status_value(const char *name) {
  for (const auto &info : WIFI_STATUS_NAMES) {
    if (strcmp(info.name, name) == 0)
      return info.status;
  }
  return WIFI_ERROR;
}

const char *fault_name(uint16_t code) {
  for (const auto &info : FAULT_NAMES) {
    if (info.code == code)
      return info.name;
  }
  return nullptr;
}

const char *brightness_name(uint16_t value) {
  for (const auto &info : BRIGHTNESS_NAMES) {
    if (info.value == value)
      return info.name;
  }
  return nullptr;
}

uint16_t brightness_value(const char *name) {
  for (const auto &info : BRIGHTNESS_NAMES) {
    if (strcmp(info.name, name) == 0)
      return info.value;
  }
  return BRIGHTNESS_HIGH;
}

const char *night_light_mode_name(uint16_t value) {
  for (const auto &info : NIGHT_LIGHT_MODE_NAMES) {
    if (info.mode == value)
      return info.name;
  }
  return nullptr;
}

uint16_t night_light_mode_value(const char *name) {
  for (const auto &info : NIGHT_LIGHT_MODE_NAMES) {
    if (strcmp(info.name, name) == 0)
      return info.mode;
  }
  return NIGHT_LIGHT_AUTO;
}

uint16_t panel_brightness_value(const char *name) {
  for (const auto &info : PANEL_BRIGHTNESS_NAMES) {
    if (strcmp(info.name, name) == 0)
      return info.value;
  }
  return (BRIGHTNESS_HIGH << 8) | PANEL_DIM_HIGH;
}

const char *format_register_value(Register reg, uint16_t value) {
  switch (reg) {
    case REG_KEYPAD: {
      for (const auto &info : KEYPAD_COMMAND_NAMES) {
        if (info.command == value)
          return info.name;
      }
      static char keypad_buf[32];
      snprintf(keypad_buf, sizeof(keypad_buf), "%u (0x%04X)", value, value);
      return keypad_buf;
    }

    case REG_FACTORY_RESET:
      if (value == CMD_FACTORY_RESET) {
        return "Reset";
      }
      break;

    case REG_POWER_TYPE:
      return value == 0 ? "AC" : "Battery";

    case REG_CAT_WEIGHT: {
      static char weight_buf[16];
      snprintf(weight_buf, sizeof(weight_buf), "%.1f lbs", static_cast<int16_t>(value) / 100.0f);
      return weight_buf;
    }

    case REG_CLEAN_CYCLE_DELAY: {
      static char delay_buf[16];
      snprintf(delay_buf, sizeof(delay_buf), "%u min", value);
      return delay_buf;
    }

    case REG_PANEL_LOCKOUT:
    case REG_BONNET_REMOVED:
    case REG_NIGHT_LIGHT:
    case REG_SLEEPING:
    case REG_WASTE_DRAWER_FULL:
      return value != 0 ? "On" : "Off";

    case REG_LITTER_HOPPER: {
      switch (value) {
        case LITTER_HOPPER_DISABLED:
          return "Off";
        case LITTER_HOPPER_ENABLE_CMD:
        case LITTER_HOPPER_ENABLED:
          return "On";
        case LITTER_HOPPER_MOTOR_START:
          return "Dispense start";
        case LITTER_HOPPER_NOT_CONNECTED:
          return "Fault";
        default:
          if ((value >> 8) == LITTER_HOPPER_STOPPED_HIGH) {
            return "Dispense complete";
          }
      }
      break;
    }

    case REG_DETECTION_EVENT: {
      for (const auto &info : DETECTION_EVENT_NAMES) {
        if (info.event == value)
          return info.name;
      }
      break;
    }

    case REG_WASTE_DRAWER_PCT:
    case REG_DEBUG_LIGHT_LEVEL: {
      static char pct_buf[16];
      snprintf(pct_buf, sizeof(pct_buf), "%u%%", value);
      return pct_buf;
    }

    case REG_LITTER_LEVEL_RAW:
    case REG_CALIBRATED_LITTER_LEVEL_RAW:
    case REG_LASER_LEFT:
    case REG_LASER_CENTER:
    case REG_LASER_RIGHT:
    case REG_DEBUG_LASER_LEFT:
    case REG_DEBUG_LASER_CENTER:
    case REG_DEBUG_LASER_RIGHT: {
      static char mm_buf[16];
      snprintf(mm_buf, sizeof(mm_buf), "%u mm", value);
      return mm_buf;
    }

    case REG_DEBUG_WEIGHT: {
      static char live_weight_buf[16];
      snprintf(live_weight_buf, sizeof(live_weight_buf), "%.1f lbs", static_cast<int16_t>(value) / 10.0f);
      return live_weight_buf;
    }

    case REG_PANEL_LED: {
      static char panel_buf[32];
      uint8_t brightness = static_cast<uint8_t>(value >> 8);
      const char *level = brightness_name(brightness);
      if (level) {
        snprintf(panel_buf, sizeof(panel_buf), "%s", level);
      } else {
        snprintf(panel_buf, sizeof(panel_buf), "%u (0x%04X)", value, value);
      }
      return panel_buf;
    }

    case REG_NIGHT_LIGHT_MODE: {
      const char *mode = night_light_mode_name(value);
      if (mode) {
        return mode;
      }
      static char mode_buf[20];
      snprintf(mode_buf, sizeof(mode_buf), "%u (0x%04X)", value, value);
      return mode_buf;
    }

    case REG_NIGHT_LIGHT_BRIGHTNESS: {
      static char bright_buf[20];
      const char *level = brightness_name(value);
      if (level) {
        snprintf(bright_buf, sizeof(bright_buf), "%s", level);
      } else {
        snprintf(bright_buf, sizeof(bright_buf), "%u (0x%04X)", value, value);
      }
      return bright_buf;
    }

    case REG_ROBOT_STATUS: {
      auto *name = status_name(value);
      static char status_buf[32];
      if (name) {
        return name;
      } else {
        snprintf(status_buf, sizeof(status_buf), "%u (0x%04X)", value, value);
      }
      return status_buf;
    }

    case REG_WIFI_STATUS: {
      auto *wifi_name = wifi_status_name(value);
      static char wifi_buf[32];
      if (wifi_name) {
        return wifi_name;
      } else {
        snprintf(wifi_buf, sizeof(wifi_buf), "%u (0x%04X)", value, value);
      }
      return wifi_buf;
    }

    case REG_FAULT_CODE: {
      if (auto *name = fault_name(value)) {
        return name;
      }
      static char fault_buf[16];
      snprintf(fault_buf, sizeof(fault_buf), "%s (%u)", value != 0 ? "Fault" : "None", value);
      return fault_buf;
    }

    case REG_SLEEP_SUN:
    case REG_WAKE_SUN:
    case REG_SLEEP_MON:
    case REG_WAKE_MON:
    case REG_SLEEP_TUE:
    case REG_WAKE_TUE:
    case REG_SLEEP_WED:
    case REG_WAKE_WED:
    case REG_SLEEP_THU:
    case REG_WAKE_THU:
    case REG_SLEEP_FRI:
    case REG_WAKE_FRI:
    case REG_SLEEP_SAT:
    case REG_WAKE_SAT: {
      static char time_buf[8];
      snprintf(time_buf, sizeof(time_buf), "%02u:%02u", value / 60, value % 60);
      return time_buf;
    }

    case REG_HEARTBEAT:
      if (value == 99) {
        return "Normal";
      }
      break;

    default:
      break;
  }

  static char fallback_buf[20];
  snprintf(fallback_buf, sizeof(fallback_buf), "%u (0x%04X)", value, value);
  return fallback_buf;
}

}  // namespace esphome::litter_robot4
