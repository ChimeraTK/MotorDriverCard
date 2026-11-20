// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "BasicStepperMotor.h"
#include "MotorController.h"

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
  : _stepperMotor(stepperMotor), _motorController(stepperMotor._motorController) {
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
    if(!_motorController->isMotorMoving()) {
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
        _motorController->setTargetPosition(_motorController->getActualPosition());
        stateExitEvnt = errorEvent;
      }
      performTransition(stateExitEvnt);
    }
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionIdleToMove() {
    _asyncActionActive.exchange(true);
    _motorController->setTargetPosition(_stepperMotor._targetPositionInSteps);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionMovetoStop() {
    int currentPos = _motorController->getActualPosition();

    _motorController->setTargetPosition(currentPos);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionMoveToFullStep() {
    bool isFullStepping = _motorController->isFullStepping();
    _motorController->enableFullStepping(true);
    actionMovetoStop();
    _motorController->enableFullStepping(isFullStepping);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionEmergencyStop() {
    _motorController->setMotorCurrentEnabled(false);
    _motorController->setEndSwitchPowerEnabled(false);
    actionMovetoStop();
    _motorController->setCalibrationTime(0);
    _stepperMotor._calibrationMode.exchange(CalibrationMode::NONE);
    _stepperMotor._errorMode.exchange(Error::EMERGENCY_STOP);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionEnable() {
    _motorController->setMotorCurrentEnabled(true);
    _motorController->setEndSwitchPowerEnabled(true);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionDisable() {
    actionMoveToFullStep();
    _motorController->setMotorCurrentEnabled(false);
    _motorController->setEndSwitchPowerEnabled(false);
  }

  /********************************************************************************************************************/

  void BasicStepperMotor::StateMachine::actionResetError() {}

} // namespace ChimeraTK::MotorDriver
