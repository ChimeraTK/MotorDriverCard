// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LinearStepperMotor.h"

#include "LinearStepperMotorStateMachine.h"

namespace ChimeraTK::MotorDriver {

  LinearStepperMotor::LinearStepperMotor(const StepperMotorParameters& parameters)
  : ReferenceStepperMotor(parameters, std::make_shared<LinearStepperMotorStateMachine>(*this)) {}

} // namespace ChimeraTK::MotorDriver
