// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ReferenceStepperMotor.h"

namespace ChimeraTK::MotorDriver {
  class RotaryStepperMotorStateMachine; // forward declare

  /** Rotary stepper motor (continuous 360° rotation, no negative end switch).
   *
   *  The rotary stage has no physical negative end switch. The software handles
   *  this by overriding hasNegativeReferenceSwitch() to return false, which makes
   *  isEndSwitchActive() ignore the negative switch, and by overriding limitsOK()
   *  to skip the end-switch range check.
   *
   *  At the hardware level, the TMC429 chip can also be configured to ignore
   *  end switches via the referenceConfigAndRampModeData register.
   *  For a rotary motor, set this in the XML config:
   *  <Register name="referenceConfigAndRampModeData" value="0x300"/>
   *  Value 0x300 sets both DISABLE_STOP_L and DISABLE_STOP_R bits,
   *  telling the TMC429 to ignore both left and right end switches.
   */
  class RotaryStepperMotor : public ReferenceStepperMotor {
   public:
    explicit RotaryStepperMotor(const StepperMotorParameters&);
    ~RotaryStepperMotor() override = default;

    bool limitsOK(int newPositionInSteps) override { return BasicStepperMotor::limitsOK(newPositionInSteps); }

   protected:
    bool hasNegativeReferenceSwitch() const override { return false; }

    /// Accepts the move when the positive switch is active or the position matches; the negative switch is ignored.
    bool verifyMoveAction() override;
  };

} // namespace ChimeraTK::MotorDriver
