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
