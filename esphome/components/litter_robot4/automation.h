#pragma once

#include "esphome/core/automation.h"
#include "litter_robot4.h"

namespace esphome::litter_robot4 {

template<typename... Ts>
class LitterRobot4SyncTimeAction final : public Action<Ts...>, public Parented<LitterRobot4Component> {
 public:
  void play(const Ts &...) override { this->parent_->sync_time(); }
};

}  // namespace esphome::litter_robot4
