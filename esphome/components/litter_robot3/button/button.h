#pragma once

#include "esphome/components/button/button.h"
#include "esphome/components/litter_robot3/litter_robot3.h"
#include "esphome/core/component.h"

namespace esphome::litter_robot3 {

class LitterRobot3Button : public button::Button, public Component, public Parented<LitterRobot3Component> {
 public:
  void set_command(char command) { this->command_ = command; }
  void press_action() override { this->parent_->send_command(this->command_); }
  void dump_config() override;

 protected:
  char command_{0};
};

}  // namespace esphome::litter_robot3
