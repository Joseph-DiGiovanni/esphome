#pragma once

#include "esphome/components/litter_robot3/litter_robot3.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome::litter_robot3 {

class LitterRobot3StatusTextSensor : public text_sensor::TextSensor,
                                     public Component,
                                     public Parented<LitterRobot3Component> {
 public:
  void setup() override;
  void dump_config() override;
};

}  // namespace esphome::litter_robot3
