// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "ReferenceStepperMotor.h"

namespace ChimeraTK::MotorDriver {
  class LinearStepperMotorStateMachine;

  class LinearStepperMotor : public ReferenceStepperMotor {
   public:
    explicit LinearStepperMotor(const StepperMotorParameters&);
    ~LinearStepperMotor() override = default;

    // linear has real negative switch → default behavior is fine
  };

} // namespace ChimeraTK::MotorDriver
