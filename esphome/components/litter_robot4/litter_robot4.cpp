#include "litter_robot4.h"
#include "esphome/core/log.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4";

void LitterRobot4Component::setup() {}

void LitterRobot4Component::loop() {}

void LitterRobot4Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Litter Robot 4:");
  ESP_LOGCONFIG(TAG, "  Baud rate: 256000");
#ifdef USE_TIME
  ESP_LOGCONFIG(TAG, "  Time sync: %s", this->time_id_ != nullptr ? "Enabled" : "Disabled");
#endif
}

}  // namespace esphome::litter_robot4
