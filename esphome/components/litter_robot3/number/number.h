#pragma once

#include "esphome/components/litter_robot3/litter_robot3.h"
#include "esphome/components/number/number.h"
#include "esphome/core/component.h"

namespace esphome::litter_robot3 {

class LitterRobot3CycleDelayNumber : public number::Number, public Component, public Parented<LitterRobot3Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(float value) override;
};

}  // namespace esphome::litter_robot3
