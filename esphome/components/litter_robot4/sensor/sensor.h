#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#if LITTER_ROBOT4_MAX_TRACKED_CATS > 0
#include "esphome/components/litter_robot4/number/number.h"
#endif
#include "esphome/components/sensor/sensor.h"

namespace esphome::litter_robot4 {

class LitterRobot4WasteDrawerSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4LitterLevelSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4CatWeightSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
  void clear_saved_weight();

 protected:
  ESPPreferenceObject pref_{};
};

class LitterRobot4CleanCycleCountSensor : public sensor::Sensor,
                                          public Component,
                                          public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
};

class LitterRobot4OdometerSensor : public sensor::Sensor, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
  void set_register(Register reg) { this->register_ = reg; }

 protected:
  Register register_{};
};

#if LITTER_ROBOT4_MAX_TRACKED_CATS > 0
class LitterRobot4CatVisitCountSensor : public sensor::Sensor,
                                        public Component,
                                        public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
  void set_cat_number(LitterRobot4CatWeightNumber *cat) { this->cat_number_ = cat; }

 protected:
  LitterRobot4CatWeightNumber *cat_number_{nullptr};
};
#endif

}  // namespace esphome::litter_robot4
