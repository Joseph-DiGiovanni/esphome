#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/button/button.h"

namespace esphome::litter_robot4 {

class LitterRobot4CommandButton : public button::Button, public Component, public Parented<LitterRobot4Component> {
 public:
  void set_register(Register reg) { this->register_ = reg; }
  void set_command(uint16_t command) { this->command_ = command; }
  void press_action() override { this->parent_->queue_register_write(this->register_, this->command_); }
  void dump_config() override;

 protected:
  Register register_{};
  uint16_t command_{0};
};

}  // namespace esphome::litter_robot4
