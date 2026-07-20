#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome::litter_robot4 {

class LitterRobot4WasteDrawerFullBinarySensor : public binary_sensor::BinarySensor,
                                                public Component,
                                                public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

}  // namespace esphome::litter_robot4
