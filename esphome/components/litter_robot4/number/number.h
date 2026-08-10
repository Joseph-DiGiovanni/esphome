#pragma once

#include "esphome/core/component.h"
#include "esphome/components/litter_robot4/litter_robot4.h"
#include "esphome/components/number/number.h"

namespace esphome::litter_robot4 {

class LitterRobot4CycleDelayNumber : public number::Number, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;

 protected:
  void control(float value) override;
};

#if LITTER_ROBOT4_MAX_TRACKED_CATS > 0
class LitterRobot4CatWeightNumber : public number::Number, public Component, public Parented<LitterRobot4Component> {
 public:
  void setup() override;
  void dump_config() override;
  void publish_weight(float weight);
  void clear_persisted_state();
  uint32_t get_visit_count() const { return this->visit_count_; }
  template<typename F> void add_on_visit_callback(F &&callback) {
    this->on_visit_callback_.add(std::forward<F>(callback));
  }

 protected:
  void control(float value) override;
  void save_state_();
  uint32_t visit_count_{0};
  CallbackManager<void(uint32_t)> on_visit_callback_;
  ESPPreferenceObject pref_{};
};
#endif

}  // namespace esphome::litter_robot4
