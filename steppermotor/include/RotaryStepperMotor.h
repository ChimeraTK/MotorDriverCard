// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ReferenceStepperMotor.h"

namespace ChimeraTK::MotorDriver {
  class RotaryStepperMotorStateMachine; // forward declare

  /**
   * Rotate a stage continuously over 360 degrees on a stage without a physical negative reference switch.
   *
   * Switch semantics:
   *
   * - The stage has no physical negative reference switch.
   * - hasNegativeReferenceSwitch() returns false, so isEndSwitchActive() ignores the negative switch: it reports it as
   *   inactive, raises no error for both end switches being active, and the negative branch of the tolerance check is
   *   skipped.
   * - limitsOK() skips the end-switch range check.
   * - verifyMoveAction() does not consult the negative switch, so its bit cannot mask a blocked move.
   *
   * Hardware configuration:
   *
   * At the hardware level, the TMC429 chip can also be configured to ignore end switches via the
   * referenceConfigAndRampModeData register. The following XML entry sets this:
   *
   * ```
   * <Register name="referenceConfigAndRampModeData" value="0x300"/>
   * ```
   *
   * The value 0x300 sets both the DISABLE_STOP_L and DISABLE_STOP_R bits, which tells the TMC429 to ignore both the
   * left and the right end switch.
   */
  class RotaryStepperMotor : public ReferenceStepperMotor {
   public:
    explicit RotaryStepperMotor(const StepperMotorParameters&);
    ~RotaryStepperMotor() override = default;

    bool limitsOK(int newPositionInSteps) override { return BasicStepperMotor::limitsOK(newPositionInSteps); }

   protected:
    bool hasNegativeReferenceSwitch() const override { return false; }

    /// Accepts the move when the positive switch is active or the reached target equals the actual position. The
    /// negative switch is ignored.
    bool verifyMoveAction() override;
  };

} // namespace ChimeraTK::MotorDriver
