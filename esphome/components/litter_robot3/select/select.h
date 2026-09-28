#pragma once

#include "esphome/components/litter_robot3/litter_robot3.h"
#include "esphome/components/select/select.h"
#include "esphome/core/component.h"

namespace esphome::litter_robot3 {

class LitterRobot3CycleDelaySelect : public select::Select, public Component, public Parented<LitterRobot3Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(size_t index) override;

  Deduplicator<uint32_t> publish_dedup_;
};

}  // namespace esphome::litter_robot3
