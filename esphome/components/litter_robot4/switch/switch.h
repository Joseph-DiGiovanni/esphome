#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/switch/switch.h"

namespace esphome::litter_robot4 {

class LitterRobot4ControlPanelLockoutSwitch : public switch_::Switch,
                                              public Component,
                                              public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void write_state(bool state) override;
};

}  // namespace esphome::litter_robot4
