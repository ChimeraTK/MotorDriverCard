// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ReferenceStateMachine.h"

namespace ChimeraTK::MotorDriver {
  class LinearStepperMotor;

  class LinearStepperMotorStateMachine : public ReferenceStateMachine {
   public:
    explicit LinearStepperMotorStateMachine(LinearStepperMotor& motor);
    ~LinearStepperMotorStateMachine() override = default;

   protected:
    void performCalibration() override;
    void calculateToleranceValues() override;
    int getPositionEndSwitch(Sign) override;
    void findEndSwitch(Sign sign) override;
    /// Offset in steps used when searching for the end switch from the current position.
    static constexpr int END_SWITCH_SEARCH_OFFSET = 50000;
    int getOffset() const override { return END_SWITCH_SEARCH_OFFSET; }
  };
} // namespace ChimeraTK::MotorDriver
