#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome::litter_robot4 {

class LitterRobot4StatusTextSensor : public text_sensor::TextSensor,
                                     public Component,
                                     public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  uint16_t robot_status_{0};
  uint16_t fault_code_{0};
  bool waste_drawer_full_{false};
  void update_display_();
};

}  // namespace esphome::litter_robot4
