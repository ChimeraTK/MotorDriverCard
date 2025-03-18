// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "BasicStepperMotor.h"
#include "MotorControler.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

namespace ChimeraTK::MotorDriver {

  const utility::StateMachine::Event BasicStepperMotor::StateMachine::initialEvent("initialEvent");
  const utility::StateMachine::Event BasicStepperMotor::StateMachine::moveEvent("moveEvent");
  const utility::StateMachine::Event BasicStepperMotor::StateMachine::stopEvent("stopEvent");
  const utility::StateMachine::Event BasicStepperMotor::StateMachine::errorEvent("errorEvent");
  const utility::StateMachine::Event BasicStepperMotor::StateMachine::emergencyStopEvent("emergencyStopEvent");
  const utility::StateMachine::Event BasicStepperMotor::StateMachine::actionCompleteEvent("actionCompletedEvent");
  const utility::StateMachine::Event BasicStepperMotor::StateMachine::enableEvent("enableEvent");
  const utility::StateMachine::Event BasicStepperMotor::StateMachine::disableEvent("disableEvent");
  const utility::StateMachine::Event BasicStepperMotor::StateMachine::resetToIdleEvent("resetToIdleEvent");
  const utility::StateMachine::Event BasicStepperMotor::StateMachine::resetToDisableEvent("resetToDisableEvent");

  /********************************************************************************************************************/

  BasicStepperMotor::StateMachine::StateMachine(BasicStepperMotor& stepperMotor)
  : _stepperMotor(stepperMotor), _motorControler(stepperMotor._motorControler) {
    _initState.setTransition(initialEvent, &_disabled, [] {});
    _idle.setTransition(moveEvent, &_moving, [this] { actionIdleToMove(); }, [this] { waitForStandstill(); });
    _idle.setTransition(disableEvent, &_disabled, [this] { actionDisable(); });
    _moving.setTransition(stopEvent, &_idle, [this] { actionMovetoStop(); });
    _moving.setTransition(errorEvent, &_error, [] {});
    _moving.setTransition(emergencyStopEvent, &_error, [this] { actionEmergencyStop(); });
    _moving.setTransition(disableEvent, &_disabled, [this] { actionDisable(); });
    _disabled.setTransition(enableEvent, &_idle, [this] { actionEnable(); });
    _error.setTransition(resetToIdleEvent, &_idle, [this] { actionResetError(); });
    _error.setTransition(resetToDisableEvent, &_disabled, [this] { actionResetError(); });
  }

  /********************************************************************************************************************/

  BasicStepperMotor::StateMachine::~StateMachine() = default;

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::waitForStandstill() {
    if(!_motorControler->isMotorMoving()) {
      _asyncActionActive.exchange(false);

      // Move may be stopped by
      // user requesting state transition
      if(hasRequestedState()) {
        moveToRequestedState();
        return;
      }

      Event stateExitEvnt = stopEvent;

      // Motor stopped by itself
      if(!_stepperMotor.verifyMoveAction()) {
        _stepperMotor._errorMode.exchange(Error::MOVE_INTERRUPTED);

        _motorControler->setTargetPosition(_motorControler->getActualPosition());
        stateExitEvnt = errorEvent;
      }
      performTransition(stateExitEvnt);
    }
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionIdleToMove() {
    _asyncActionActive.exchange(true);
    _motorControler->setTargetPosition(_stepperMotor._targetPositionInSteps);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionMovetoStop() {
    int currentPos = _motorControler->getActualPosition();
    _motorControler->setTargetPosition(currentPos);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionMoveToFullStep() {
    bool isFullStepping = _motorControler->isFullStepping();
    _motorControler->enableFullStepping(true);
    actionMovetoStop();
    _motorControler->enableFullStepping(isFullStepping);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionEmergencyStop() {
    _motorControler->setMotorCurrentEnabled(false);
    _motorControler->setEndSwitchPowerEnabled(false);
    actionMovetoStop();
    _motorControler->setCalibrationTime(0);
    _stepperMotor._calibrationMode.exchange(CalibrationMode::NONE);
    _stepperMotor._errorMode.exchange(Error::EMERGENCY_STOP);
    std::cerr << "Emergency stop" << std::endl;
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionEnable() {
    _motorControler->setMotorCurrentEnabled(true);
    _motorControler->setEndSwitchPowerEnabled(true);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionDisable() {
    actionMoveToFullStep();
    _motorControler->setMotorCurrentEnabled(false);
    _motorControler->setEndSwitchPowerEnabled(false);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionResetError() {}

} // namespace ChimeraTK::MotorDriver
