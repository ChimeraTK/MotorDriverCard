/*
 * StepperMotorStateMachine.cc
 *
 *  Created on: Feb 2, 2017
 *      Author: vitimic
 */

#include "StepperMotorStateMachine.h"
#include "StepperMotor.h"
#include <memory>
#include <thread>
#include <chrono>

namespace ChimeraTK{

  const Event StepperMotorStateMachine::initialEvent("initialEvent");
  const Event StepperMotorStateMachine::moveEvent("moveEvent");
  const Event StepperMotorStateMachine::stopEvent("stopEvent");
  const Event StepperMotorStateMachine::emergencyStopEvent("emergencyStopEvent");
  const Event StepperMotorStateMachine::actionCompleteEvent("actionCompletedEvent");
  const Event StepperMotorStateMachine::enableEvent("enableEvent");
  const Event StepperMotorStateMachine::disableEvent("disableEvent");
  const Event StepperMotorStateMachine::resetToIdleEvent("resetToIdleEvent");
  const Event StepperMotorStateMachine::resetToDisableEvent("resetToDisableEvent");

  StepperMotorStateMachine::StepperMotorStateMachine(ChimeraTK::StepperMotor &stepperMotor) :
      StateMachine(),
      _moving("moving"),
      _idle("idle"),
      _disabled("disabled"),
      _error("error"),
      _stepperMotor(stepperMotor),
      _motorControler(stepperMotor._motorControler)
  {
    _initState.setTransition(initialEvent,        &_disabled,      []{});
    _idle.setTransition     (moveEvent,           &_moving,        std::bind(&StepperMotorStateMachine::actionIdleToMove, this));
    _idle.setTransition     (disableEvent,        &_disabled,      std::bind(&StepperMotorStateMachine::actionDisable, this));
    _moving.setTransition   (stopEvent,           &_idle,          std::bind(&StepperMotorStateMachine::actionMovetoStop, this));
    _moving.setTransition   (emergencyStopEvent,  &_error,         std::bind(&StepperMotorStateMachine::actionEmergencyStop, this));
    _moving.setTransition   (disableEvent,        &_disabled,      std::bind(&StepperMotorStateMachine::actionDisable, this));
    _disabled.setTransition (enableEvent,         &_idle,          std::bind(&StepperMotorStateMachine::actionEnable, this));
    _error.setTransition    (resetToIdleEvent,    &_idle,          std::bind(&StepperMotorStateMachine::actionResetError, this));
    _error.setTransition    (resetToDisableEvent, &_disabled,      std::bind(&StepperMotorStateMachine::actionResetError, this));
  }

  StepperMotorStateMachine::~StepperMotorStateMachine(){}

  void StepperMotorStateMachine::waitForStandstill(){

    if(!_motorControler->isMotorMoving()){

      // Set the propagated state or return to idle by stop event
      // Either of them does not take effect
      _asyncActionActive.exchange(false);
      moveToRequestedState();
      //TODO Perform error checking (setpoint = actual position) here?
      performTransition(stopEvent);
      _internalEventCallback = []{};
    }
  }

  void StepperMotorStateMachine::actionIdleToMove(){
    _asyncActionActive.exchange(true);
    _motorControler->setTargetPosition(_stepperMotor._targetPositionInSteps);
    _internalEventCallback = std::bind(&StepperMotorStateMachine::waitForStandstill, this);
  }

  void StepperMotorStateMachine::actionMovetoStop(){
    int currentPos = _motorControler->getActualPosition();
    _motorControler->setTargetPosition(currentPos);
  }

  void StepperMotorStateMachine::actionMoveToFullStep(){
    bool isFullStepping = _motorControler->isFullStepping();
    _motorControler->enableFullStepping(true);
    actionMovetoStop();
    _motorControler->enableFullStepping(isFullStepping);
  }

  void StepperMotorStateMachine::actionEmergencyStop(){
    _motorControler->setMotorCurrentEnabled(false);
    _motorControler->setEndSwitchPowerEnabled(false);
    actionMovetoStop();
    _motorControler->setCalibrationTime(0);
  }

  void StepperMotorStateMachine::actionEnable(){
    _motorControler->setMotorCurrentEnabled(true);
    _motorControler->setEndSwitchPowerEnabled(true);
  }

  void StepperMotorStateMachine::actionDisable(){
    actionMoveToFullStep();
    _motorControler->setMotorCurrentEnabled(false);
    _motorControler->setEndSwitchPowerEnabled(false);
  }

  void StepperMotorStateMachine::actionResetError(){}

}
