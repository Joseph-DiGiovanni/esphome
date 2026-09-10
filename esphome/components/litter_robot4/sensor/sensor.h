#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome::litter_robot4 {

class LitterRobot4LitterLevelSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
  void set_min_distance(float min_distance) { this->min_distance_ = min_distance; }
  void set_max_distance(float max_distance) { this->max_distance_ = max_distance; }

 protected:
  float min_distance_{435.0f};
  float max_distance_{480.0f};
};

class LitterRobot4CatWeightSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4RegisterSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
  void set_register(Register reg) { this->register_ = reg; }

 protected:
  Register register_{};
};

}  // namespace esphome::litter_robot4
