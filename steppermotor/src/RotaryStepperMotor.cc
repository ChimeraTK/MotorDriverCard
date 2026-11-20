// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RotaryStepperMotor.h"

#include "RotaryStepperMotorStateMachine.h"

#include <stdexcept>

namespace ChimeraTK::MotorDriver {

  RotaryStepperMotor::RotaryStepperMotor(const StepperMotorParameters& parameters)
  : ReferenceStepperMotor(parameters, std::make_shared<RotaryStepperMotorStateMachine>(*this)) {
    if(_positiveEndSwitchEnabled.load() || _negativeEndSwitchEnabled.load()) {
      throw std::logic_error("RotaryStepperMotor requires both hardware stops disabled (configuration 0x300)");
    }
  }

} // namespace ChimeraTK::MotorDriver
