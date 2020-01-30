/*
 * BasicStepperMotor::StateMachine.cc
 *
 *  Created on: Feb 2, 2017
 *      Author: vitimic
 */

#include "BasicStepperMotor.h"

#include "MotorControler.h"
#include <chrono>
#include <memory>
#include <thread>

namespace ChimeraTK { namespace MotorDriver {

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

  BasicStepperMotor::StateMachine::StateMachine(BasicStepperMotor& stepperMotor)
  : utility::StateMachine(), _moving("moving"), _idle("idle"), _disabled("disabled"), _error("error"),
    _stepperMotor(stepperMotor), _motorControler(stepperMotor._motorControler) {
    _initState.setTransition(initialEvent, &_disabled, [] {});
    _idle.setTransition(moveEvent, &_moving, std::bind(&BasicStepperMotor::StateMachine::actionIdleToMove, this),
        std::bind(&BasicStepperMotor::StateMachine::waitForStandstill, this));
    _idle.setTransition(disableEvent, &_disabled, std::bind(&BasicStepperMotor::StateMachine::actionDisable, this));
    _moving.setTransition(stopEvent, &_idle, std::bind(&BasicStepperMotor::StateMachine::actionMovetoStop, this));
    _moving.setTransition(errorEvent, &_error, [] {});
    _moving.setTransition(
        emergencyStopEvent, &_error, std::bind(&BasicStepperMotor::StateMachine::actionEmergencyStop, this));
    _moving.setTransition(disableEvent, &_disabled, std::bind(&BasicStepperMotor::StateMachine::actionDisable, this));
    _disabled.setTransition(enableEvent, &_idle, std::bind(&BasicStepperMotor::StateMachine::actionEnable, this));
    _error.setTransition(resetToIdleEvent, &_idle, std::bind(&BasicStepperMotor::StateMachine::actionResetError, this));
    _error.setTransition(
        resetToDisableEvent, &_disabled, std::bind(&BasicStepperMotor::StateMachine::actionResetError, this));
  }

  BasicStepperMotor::StateMachine::~StateMachine() {}

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
      return;
    }
  }

  void BasicStepperMotor::StateMachine::actionIdleToMove() {
    _asyncActionActive.exchange(true);
    _motorControler->setTargetPosition(_stepperMotor._targetPositionInSteps);
  }

  void BasicStepperMotor::StateMachine::actionMovetoStop() {
    int currentPos = _motorControler->getActualPosition();
    _motorControler->setTargetPosition(currentPos);
  }

  void BasicStepperMotor::StateMachine::actionMoveToFullStep() {
    bool isFullStepping = _motorControler->isFullStepping();
    _motorControler->enableFullStepping(true);
    actionMovetoStop();
    _motorControler->enableFullStepping(isFullStepping);
  }

  void BasicStepperMotor::StateMachine::actionEmergencyStop() {
    _motorControler->setMotorCurrentEnabled(false);
    _motorControler->setEndSwitchPowerEnabled(false);
    actionMovetoStop();
    _motorControler->setCalibrationTime(0);
    _stepperMotor._calibrationMode.exchange(CalibrationMode::NONE);
    _stepperMotor._errorMode.exchange(Error::EMERGENCY_STOP);
  }

  void BasicStepperMotor::StateMachine::actionEnable() {
    _motorControler->setMotorCurrentEnabled(true);
    _motorControler->setEndSwitchPowerEnabled(true);
  }

  void BasicStepperMotor::StateMachine::actionDisable() {
    actionMoveToFullStep();
    _motorControler->setMotorCurrentEnabled(false);
    _motorControler->setEndSwitchPowerEnabled(false);
  }

  void BasicStepperMotor::StateMachine::actionResetError() {}

}} // namespace ChimeraTK::MotorDriver
