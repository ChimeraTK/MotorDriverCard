// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "ReferenceStateMachine.h"

namespace ChimeraTK::MotorDriver {
  class RotaryStepperMotor;
  class RotaryStepperMotorStateMachine : public ReferenceStateMachine {
   public:
    explicit RotaryStepperMotorStateMachine(RotaryStepperMotor& motor);
    ~RotaryStepperMotorStateMachine() override = default;

   protected:
    void performCalibration() override;
    void moveOffEndSwitch();
    void calculateToleranceValues() override;
    int getPositionEndSwitch(Sign) override;
    void findEndSwitch(Sign sign) override;
    /// Shared with findEndSwitch(): a search which takes longer than this is treated as interrupted.
    static constexpr int HOMING_TIMEOUT_MS = 60000;
    /// Offset in steps used when searching for the end switch from the current position.
    static constexpr int END_SWITCH_SEARCH_OFFSET = 5000;
    int getOffset() const override { return END_SWITCH_SEARCH_OFFSET; }
  };
} // namespace ChimeraTK::MotorDriver
