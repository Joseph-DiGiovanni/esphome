#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/button/button.h"

namespace esphome::litter_robot4 {

class LitterRobot4CycleButton : public button::Button, public Component, public Parented<LitterRobot4Component> {
 public:
  void press_action() override { this->parent_->queue_register_write(REG_ROBOT_STATUS, CMD_START_CLEAN); }
  void dump_config() override;
};

class LitterRobot4EmptyButton : public button::Button, public Component, public Parented<LitterRobot4Component> {
 public:
  void press_action() override { this->parent_->queue_register_write(REG_ROBOT_STATUS, CMD_EMPTY_LITTER); }
  void dump_config() override;
};

class LitterRobot4ReplaceFilterButton : public button::Button,
                                        public Component,
                                        public Parented<LitterRobot4Component> {
 public:
  void press_action() override { this->parent_->queue_register_write(REG_ROBOT_STATUS, CMD_REPLACE_FILTER); }
  void dump_config() override;
};

class LitterRobot4FactoryResetButton : public button::Button, public Component, public Parented<LitterRobot4Component> {
 public:
  void press_action() override { this->parent_->queue_register_write(REG_FACTORY_RESET, CMD_FACTORY_RESET); }
  void dump_config() override;
};

class LitterRobot4PowerButton : public button::Button, public Component, public Parented<LitterRobot4Component> {
 public:
  void press_action() override { this->parent_->queue_register_write(REG_KEYPAD, CMD_KEYPAD_POWER); }
  void dump_config() override;
};

class LitterRobot4ResetButton : public button::Button, public Component, public Parented<LitterRobot4Component> {
 public:
  void press_action() override { this->parent_->queue_register_write(REG_KEYPAD, CMD_KEYPAD_RESET); }
  void dump_config() override;
};

}  // namespace esphome::litter_robot4
