#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/select/select.h"

namespace esphome::litter_robot4 {

class LitterRobot4NightLightModeSelect : public select::Select,
                                         public Component,
                                         public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(size_t index) override;
};

class LitterRobot4NightLightBrightnessSelect : public select::Select,
                                               public Component,
                                               public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(size_t index) override;
};

class LitterRobot4PanelBrightnessSelect : public select::Select,
                                          public Component,
                                          public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(size_t index) override;
};

}  // namespace esphome::litter_robot4
