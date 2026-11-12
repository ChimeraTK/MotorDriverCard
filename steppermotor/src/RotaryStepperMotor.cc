// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RotaryStepperMotor.h"

#include "MotorControler.h"
#include "RotaryStepperMotorStateMachine.h"

namespace ChimeraTK::MotorDriver {

  RotaryStepperMotor::RotaryStepperMotor(const StepperMotorParameters& parameters)
  : ReferenceStepperMotor(parameters, std::make_shared<RotaryStepperMotorStateMachine>(*this)) {}

  /******************************************************************************************************************/

  bool RotaryStepperMotor::verifyMoveAction() {
    // Same disjunction as the base class, but the negative switch bit is meaningless here and would mask a blocked move.
    bool endSwitchActive = isEndSwitchActive(Sign::POSITIVE);
    bool positionMatch = _motorController->getTargetPosition() == _motorController->getActualPosition();

    return endSwitchActive || positionMatch;
  }

} // namespace ChimeraTK::MotorDriver
