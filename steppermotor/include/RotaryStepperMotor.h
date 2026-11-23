// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ReferenceStepperMotor.h"

namespace ChimeraTK::MotorDriver {
  class RotaryStepperMotorStateMachine; // forward declare

  /** Rotary stepper motor, rotating continuously over 360 degrees.
   *
   *  The stage has a physical negative reference switch. It does not stop the
   *  motion when it is passed, so the class reports it as inactive:
   *  hasNegativeReferenceSwitch() and negativeSwitchActive() return false, and
   *  limitsOK() skips the end-switch range check.
   *
   *  Both hardware stops stay disabled for normal movement; the constructor
   *  requires them disabled. The calibration search drives the positive stop,
   *  which is enabled for that search only.
   *
   *  verifyMoveAction() accepts a move only when it reached its target, because
   *  normal movement does not stop at a switch.
   *
   *  At the hardware level, the TMC429 end-switch stop inputs are configured
   *  through the referenceConfigAndRampModeData register. A rotary stage needs
   *  the value 0x300 there, which sets the DISABLE_STOP_L and DISABLE_STOP_R
   *  bits. Set it in the XML config:
   *
   *  @code{.xml}
   *  <Register name="referenceConfigAndRampModeData" value="0x300"/>
   *  @endcode
   */
  class RotaryStepperMotor : public ReferenceStepperMotor {
   public:
    /// Throws std::logic_error if either hardware reference stop is enabled.
    explicit RotaryStepperMotor(const StepperMotorParameters&);
    ~RotaryStepperMotor() override = default;

    bool limitsOK(int newPositionInSteps) override { return BasicStepperMotor::limitsOK(newPositionInSteps); }

    /// Accepts a finished move only when it reached its target position. The
    /// inherited verification, which also accepts an active reference switch,
    /// does not apply: the rotary stage does not stop at a switch.
    bool verifyMoveAction() override { return BasicStepperMotor::verifyMoveAction(); }

   protected:
    bool hasNegativeReferenceSwitch() const override { return false; }
    bool negativeSwitchActive() const override { return false; }
  };

} // namespace ChimeraTK::MotorDriver
