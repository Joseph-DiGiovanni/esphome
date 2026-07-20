#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/datetime/time_entity.h"

namespace esphome::litter_robot4 {

class LitterRobot4Time : public datetime::TimeEntity, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
  void set_register(Register reg) { this->register_ = reg; }

 protected:
  void control(const datetime::TimeCall &call) override;
  Register register_{};
};

}  // namespace esphome::litter_robot4
