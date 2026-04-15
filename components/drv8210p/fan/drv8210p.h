// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Brian Alvarez

#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/automation.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/fan/fan.h"

namespace esphome {
namespace drv8210p {

static const char *const TAG = "drv8210p";

// DRV8210P H-bridge motor driver fan component.
//
// Truth table (from TI DRV8210P datasheet):
//   IN1   IN2   nSLEEP   Mode
//   0     0     0         Sleep (low power)
//   0     0     1         Coast (outputs Hi-Z)
//   PWM   0     1         Forward
//   0     PWM   1         Reverse
//   1     1     1         Brake (low-side slow decay)
//
// nSLEEP power management:
//   - Active (forward/reverse/brake): nSLEEP HIGH
//   - Coast / off: nSLEEP LOW to save power

class DRV8210PFan : public Component, public fan::Fan {
 public:
  explicit DRV8210PFan(int speed_count) : speed_count_(speed_count) {}

  void set_pin_a(output::FloatOutput *pin_a) { this->pin_a_ = pin_a; }
  void set_pin_b(output::FloatOutput *pin_b) { this->pin_b_ = pin_b; }
  void set_nsleep_pin(GPIOPin *pin) { this->nsleep_pin_ = pin; }

  void setup() override {
    if (this->nsleep_pin_ != nullptr) {
      this->nsleep_pin_->setup();
      this->nsleep_pin_->digital_write(false);  // start in sleep
    }
    this->pin_a_->set_level(0.0f);
    this->pin_b_->set_level(0.0f);

    this->traits_ = fan::FanTraits(false, true, true, this->speed_count_);

    auto restore = this->restore_state_();
    if (restore.has_value()) {
      restore->apply(*this);
      this->write_state_();
    }
  }

  void dump_config() override {
    LOG_FAN("", "DRV8210P Fan", this);
    ESP_LOGCONFIG(TAG, "  Speed count: %d", this->speed_count_);
  }

  fan::FanTraits get_traits() override { return this->traits_; }

  fan::FanCall brake() {
    ESP_LOGV(TAG, "Braking");
    if (this->nsleep_pin_ != nullptr)
      this->nsleep_pin_->digital_write(true);
    this->pin_a_->set_level(1.0f);
    this->pin_b_->set_level(1.0f);
    return this->make_call().set_state(false);
  }

  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  void control(const fan::FanCall &call) override {
    if (call.get_state().has_value())
      this->state = *call.get_state();
    if (call.get_speed().has_value())
      this->speed = *call.get_speed();
    if (call.get_direction().has_value())
      this->direction = *call.get_direction();

    this->write_state_();
    this->publish_state();
  }

  void write_state_() {
    float speed = this->state
                      ? static_cast<float>(this->speed) / static_cast<float>(this->speed_count_)
                      : 0.0f;

    if (speed == 0.0f) {
      // Coast + sleep to save power
      this->pin_a_->set_level(0.0f);
      this->pin_b_->set_level(0.0f);
      if (this->nsleep_pin_ != nullptr)
        this->nsleep_pin_->digital_write(false);
      ESP_LOGV(TAG, "Sleep (coast, power save)");
    } else if (this->direction == fan::FanDirection::FORWARD) {
      if (this->nsleep_pin_ != nullptr)
        this->nsleep_pin_->digital_write(true);
      this->pin_a_->set_level(speed);
      this->pin_b_->set_level(0.0f);
      ESP_LOGV(TAG, "Forward: duty=%.2f", speed);
    } else {
      if (this->nsleep_pin_ != nullptr)
        this->nsleep_pin_->digital_write(true);
      this->pin_a_->set_level(0.0f);
      this->pin_b_->set_level(speed);
      ESP_LOGV(TAG, "Reverse: duty=%.2f", speed);
    }
  }

  output::FloatOutput *pin_a_;
  output::FloatOutput *pin_b_;
  GPIOPin *nsleep_pin_{nullptr};
  int speed_count_{};
  fan::FanTraits traits_;
};

template<typename... Ts> class BrakeAction : public Action<Ts...> {
 public:
  explicit BrakeAction(DRV8210PFan *parent) : parent_(parent) {}

  void play(const Ts &...x) override { this->parent_->brake(); }

 protected:
  DRV8210PFan *parent_;
};

}  // namespace drv8210p
}  // namespace esphome
