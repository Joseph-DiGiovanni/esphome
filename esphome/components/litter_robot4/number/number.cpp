#include "esphome/core/log.h"
#include "number.h"

namespace esphome::litter_robot4 {

static const char *const TAG = "litter_robot4.number";

void LitterRobot4CycleDelayNumber::setup() {
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_CLEAN_CYCLE_DELAY) {
      this->publish_state(value);
    }
  });
}

void LitterRobot4CycleDelayNumber::control(float value) {
  this->parent_->queue_register_write(REG_CLEAN_CYCLE_DELAY, static_cast<uint16_t>(value));
}

void LitterRobot4CycleDelayNumber::dump_config() { LOG_NUMBER("", "Litter Robot 4 Clean Cycle Delay", this); }

#if LITTER_ROBOT4_MAX_TRACKED_CATS > 0
struct TrackedCatRestoreState {
  float last_weight;
  uint32_t visit_count;
};

void LitterRobot4CatWeightNumber::setup() {
  this->parent_->register_tracked_cat(this);
  this->pref_ = this->make_entity_preference<TrackedCatRestoreState>();
  TrackedCatRestoreState state{};
  if (this->pref_.load(&state)) {
    this->visit_count_ = state.visit_count;
    this->publish_state(state.last_weight);
    this->on_visit_callback_.call(this->visit_count_);
  }
  this->parent_->setup_on_register_update_callback([this](Register reg, uint16_t value) {
    if (reg == REG_FACTORY_RESET && value == CMD_FACTORY_RESET) {
      this->clear_persisted_state();
    }
  });
}

void LitterRobot4CatWeightNumber::control(float value) {
  this->publish_state(value);
  this->save_state_();
}

void LitterRobot4CatWeightNumber::publish_weight(float weight) {
  this->publish_state(weight);
  this->visit_count_++;
  this->on_visit_callback_.call(this->visit_count_);
  this->save_state_();
}

void LitterRobot4CatWeightNumber::save_state_() {
  TrackedCatRestoreState state{};
  state.last_weight = this->state;
  state.visit_count = this->visit_count_;
  this->pref_.save(&state);
}

void LitterRobot4CatWeightNumber::clear_persisted_state() {
  this->visit_count_ = 0;
  this->on_visit_callback_.call(0);
  this->publish_state(NAN);

  TrackedCatRestoreState state{};
  state.last_weight = NAN;
  state.visit_count = 0;
  this->pref_.save(&state);
  global_preferences->sync();
}

void LitterRobot4CatWeightNumber::dump_config() { LOG_NUMBER("", "Litter Robot 4 Cat Weight", this); }
#endif

}  // namespace esphome::litter_robot4
