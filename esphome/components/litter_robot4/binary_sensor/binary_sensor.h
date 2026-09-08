#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome::litter_robot4 {

class LitterRobot4BoolBinarySensor : public binary_sensor::BinarySensor,
                                     public Component,
                                     public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
  void set_register(Register reg) { this->register_ = reg; }

 protected:
  Register register_{};
};

class LitterRobot4LaserDetectBinarySensor : public binary_sensor::BinarySensor,
                                            public Component,
                                            public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4WeightDetectBinarySensor : public binary_sensor::BinarySensor,
                                             public Component,
                                             public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4HopperMotorBinarySensor : public binary_sensor::BinarySensor,
                                            public Component,
                                            public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

}  // namespace esphome::litter_robot4
