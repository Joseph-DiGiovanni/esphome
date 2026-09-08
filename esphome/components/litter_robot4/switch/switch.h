#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/switch/switch.h"

namespace esphome::litter_robot4 {

class LitterRobot4BoolSwitch : public switch_::Switch, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
  void set_register(Register reg) { this->register_ = reg; }

 protected:
  void write_state(bool state) override;
  Register register_{};
};

class LitterRobot4SleepDayEnabledSwitch : public switch_::Switch,
                                          public Component,
                                          public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
  void set_day(DayOfWeek day) { this->day_ = day; }

 protected:
  void write_state(bool state) override;
  DayOfWeek day_{DAY_SUN};
};

class LitterRobot4PowerSwitch : public switch_::Switch, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void write_state(bool state) override;
};

}  // namespace esphome::litter_robot4
