// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "ReferenceStateMachine.h"

#include <chrono>

namespace ChimeraTK::MotorDriver {
  class RotaryStepperMotor;
  class RotaryStepperMotorStateMachine : public ReferenceStateMachine {
   public:
    explicit RotaryStepperMotorStateMachine(RotaryStepperMotor& motor);
    ~RotaryStepperMotorStateMachine() override = default;

   protected:
    void performCalibration() override;
    int getPositionEndSwitch(Sign) override;
    void findEndSwitch(Sign sign) override;
    /// Offset in steps used when searching for the end switch from the current position.
    static constexpr int END_SWITCH_SEARCH_OFFSET = 5000;
    int getOffset() const override { return END_SWITCH_SEARCH_OFFSET; }

   private:
    using HomingDeadline = std::chrono::steady_clock::time_point;
    void refineHomePosition(HomingDeadline deadline, double savedSpeed, bool& speedRestored);
    void checkHomingDeadline(HomingDeadline deadline);
    void moveForHoming(int offset, HomingDeadline deadline);
    void findEndSwitch(Sign sign, HomingDeadline deadline);
  };
} // namespace ChimeraTK::MotorDriver
