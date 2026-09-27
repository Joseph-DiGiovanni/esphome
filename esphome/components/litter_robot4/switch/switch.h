#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/switch/switch.h"

namespace esphome::litter_robot4 {

// The sleep-schedule day-enable register packs all seven days into one bitmask, so no single
// day switch can own it. This coordinator holds the shared mask and performs the read-modify-write.
class LitterRobot4SleepSchedule : public Parented<LitterRobot4Component> {
 public:
  void set_day_enabled(DayOfWeek day, bool enabled);
  void update_mask(uint16_t mask) { this->mask_ = mask; }

 protected:
  uint16_t mask_{0};
};

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
  void set_schedule(LitterRobot4SleepSchedule *schedule) { this->schedule_ = schedule; }

 protected:
  void write_state(bool state) override;
  DayOfWeek day_{DAY_SUN};
  LitterRobot4SleepSchedule *schedule_{nullptr};
};

class LitterRobot4PowerSwitch : public switch_::Switch, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void write_state(bool state) override;
};

}  // namespace esphome::litter_robot4
