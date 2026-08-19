#include "esphome/core/log.h"
#include "button.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.button";

void LitterRobot4CycleButton::dump_config() { LOG_BUTTON("", "Litter Robot 4 Cycle", this); }

void LitterRobot4EmptyButton::dump_config() { LOG_BUTTON("", "Litter Robot 4 Empty", this); }

void LitterRobot4ReplaceFilterButton::dump_config() { LOG_BUTTON("", "Litter Robot 4 Replace Filter", this); }

void LitterRobot4FactoryResetButton::dump_config() { LOG_BUTTON("", "Litter Robot 4 Factory Reset", this); }

void LitterRobot4ResetButton::dump_config() { LOG_BUTTON("", "Litter Robot 4 Reset", this); }

}  // namespace esphome::litter_robot4
