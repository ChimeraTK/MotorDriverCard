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
    /// Searches for the wanted watch switch state in chunks of getOffset() towards direction. Aborts on stop, on a
    /// blocked move, on a move while neither reference is active, and on timeout; sets _moveInterrupted for the latter
    /// two, and for the move check, but not for a stop.
    bool waitForSwitchState(Sign watchSign, Sign direction, bool wanted);
    /// Moves the motor negatively in chunks of getOffset() until an active positive end switch releases.
    void moveOffEndSwitch();
    void calculateToleranceValues() override;
    int getPositionEndSwitch(Sign) override;
    void findEndSwitch(Sign sign) override;
    /// Offset in steps used when searching for the end switch from the current position.
    static constexpr int END_SWITCH_SEARCH_OFFSET = 5000;
    int getOffset() const override { return END_SWITCH_SEARCH_OFFSET; }
  };
} // namespace ChimeraTK::MotorDriver
