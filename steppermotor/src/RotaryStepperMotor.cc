// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RotaryStepperMotor.h"

#include "RotaryStepperMotorStateMachine.h"

namespace ChimeraTK::MotorDriver {

  RotaryStepperMotor::RotaryStepperMotor(const StepperMotorParameters& parameters)
  : ReferenceStepperMotor(parameters, std::make_shared<RotaryStepperMotorStateMachine>(*this)) {}

} // namespace ChimeraTK::MotorDriver
