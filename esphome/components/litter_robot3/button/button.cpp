#include "button.h"

#include "esphome/core/log.h"

namespace esphome::litter_robot3 {

static const char *const TAG = "litter_robot3.button";

void LitterRobot3Button::dump_config() { LOG_BUTTON("", "Litter Robot 3 Command", this); }

}  // namespace esphome::litter_robot3
