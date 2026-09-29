#pragma once

#include "esphome/components/litter_robot3/litter_robot3.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/component.h"

namespace esphome::litter_robot3 {

class LitterRobot3Switch : public switch_::Switch, public Component, public Parented<LitterRobot3Component> {
 public:
  void setup() override;
  void dump_config() override;
  void set_command(char command) { this->command_ = command; }
  void set_field(LitterRobot3Field field) { this->field_ = field; }
  void set_invert(bool invert) { this->invert_ = invert; }

 protected:
  void write_state(bool state) override;

  char command_{0};
  LitterRobot3Field field_{};
  bool invert_{false};
};

class LitterRobot3SleepSwitch : public switch_::Switch, public Component, public Parented<LitterRobot3Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void write_state(bool state) override;
};

}  // namespace esphome::litter_robot3
