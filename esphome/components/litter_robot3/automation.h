#pragma once

#include "esphome/core/automation.h"
#include "litter_robot3.h"

namespace esphome::litter_robot3 {

template<typename... Ts>
class LitterRobot3SetSleepAction final : public Action<Ts...>, public Parented<LitterRobot3Component> {
 public:
  TEMPLATABLE_VALUE(uint32_t, offset)

  void play(const Ts &...) override { this->parent_->set_sleep(this->offset_.value()); }
};

}  // namespace esphome::litter_robot3
