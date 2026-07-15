#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome::litter_robot4 {

class LitterRobot4Component final : public uart::UARTDevice, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
};

}  // namespace esphome::litter_robot4
