// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ReferenceStepperMotor.h"

namespace ChimeraTK::MotorDriver {
  class RotaryStepperMotorStateMachine; // forward declare

  class RotaryStepperMotor : public ReferenceStepperMotor {
   public:
    explicit RotaryStepperMotor(const StepperMotorParameters&);
    ~RotaryStepperMotor() override = default;

    bool limitsOK(int newPositionInSteps) override { return BasicStepperMotor::limitsOK(newPositionInSteps); }

   protected:
    bool hasNegativeReferenceSwitch() const override { return false; }
    bool negativeSwitchActive() const override { return false; }
  };

} // namespace ChimeraTK::MotorDriver
