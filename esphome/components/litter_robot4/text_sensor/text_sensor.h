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
};

class LitterRobot4PowerTypeTextSensor : public text_sensor::TextSensor,
                                        public Component,
                                        public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4FaultTextSensor : public text_sensor::TextSensor,
                                    public Component,
                                    public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void update_display_();
  uint16_t fault_code_{0};
  bool bonnet_fault_{false};
  uint16_t robot_status_{0};
};

class LitterRobot4HopperStatusTextSensor : public text_sensor::TextSensor,
                                           public Component,
                                           public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

}  // namespace esphome::litter_robot4
