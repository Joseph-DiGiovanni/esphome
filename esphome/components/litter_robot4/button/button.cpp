#include "esphome/core/log.h"
#include "button.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.button";

void LitterRobot4CommandButton::dump_config() { LOG_BUTTON("", "Litter Robot 4 Command", this); }

}  // namespace esphome::litter_robot4
