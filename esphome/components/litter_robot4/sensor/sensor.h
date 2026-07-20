#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome::litter_robot4 {

class LitterRobot4WasteDrawerSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4LitterLevelSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4CatWeightSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4CleanCycleCountSensor : public sensor::Sensor,
                                          public Component,
                                          public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

}  // namespace esphome::litter_robot4
