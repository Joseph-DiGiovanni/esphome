#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/number/number.h"

namespace esphome::litter_robot4 {

class LitterRobot4CycleDelayNumber : public number::Number, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(float value) override;
};

}  // namespace esphome::litter_robot4
