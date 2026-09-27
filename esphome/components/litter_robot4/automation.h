#pragma once

#include "esphome/core/automation.h"
#include "litter_robot4.h"
#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

namespace esphome::litter_robot4 {

template<typename... Ts>
class LitterRobot4SyncTimeAction final : public Action<Ts...>, public Parented<LitterRobot4Component> {
 public:
  void play(const Ts &...) override {
#ifdef USE_TIME
    if (this->time_ == nullptr)
      return;
    auto now = this->time_->now();
    if (!now.is_valid())
      return;
    // ESPHome Time uses Sunday=1, PIC uses Sunday=0.
    this->parent_->queue_register_write(REG_TIME_DOW, now.day_of_week - 1);
    this->parent_->queue_register_write(REG_TIME_HOUR, now.hour);
    this->parent_->queue_register_write(REG_TIME_MINUTE, now.minute);
    this->parent_->queue_register_write(REG_TIME_SECOND, now.second);
    this->parent_->queue_register_write(REG_TIME_DAY, now.day_of_month);
    this->parent_->queue_register_write(REG_TIME_MONTH, now.month);
    // Original firmware writes only last 2 digits of year.
    this->parent_->queue_register_write(REG_TIME_YEAR, now.year % 100);
#endif
  }

#ifdef USE_TIME
  void set_time(time::RealTimeClock *time) { this->time_ = time; }

 protected:
  time::RealTimeClock *time_{nullptr};
#endif
};

}  // namespace esphome::litter_robot4
