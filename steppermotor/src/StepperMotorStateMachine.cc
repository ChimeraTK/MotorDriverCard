/*
 * StepperMotorStateMachine.cc
 *
 *  Created on: Feb 2, 2017
 *      Author: vitimic
 */

#include "StepperMotorStateMachine.h"
#include "BasicStepperMotor.h"
#include <memory>
#include <thread>
#include <chrono>

namespace ChimeraTK {
namespace motordriver{
namespace utility {

  const StateMachine::Event StepperMotorStateMachine::initialEvent("initialEvent");
  const StateMachine::Event StepperMotorStateMachine::moveEvent("moveEvent");
  const StateMachine::Event StepperMotorStateMachine::stopEvent("stopEvent");
  const StateMachine::Event StepperMotorStateMachine::errorEvent("errorEvent");
  const StateMachine::Event StepperMotorStateMachine::emergencyStopEvent("emergencyStopEvent");
  const StateMachine::Event StepperMotorStateMachine::actionCompleteEvent("actionCompletedEvent");
  const StateMachine::Event StepperMotorStateMachine::enableEvent("enableEvent");
  const StateMachine::Event StepperMotorStateMachine::disableEvent("disableEvent");
  const StateMachine::Event StepperMotorStateMachine::resetToIdleEvent("resetToIdleEvent");
  const StateMachine::Event StepperMotorStateMachine::resetToDisableEvent("resetToDisableEvent");

  StepperMotorStateMachine::StepperMotorStateMachine(BasicStepperMotor &stepperMotor) :
      StateMachine(),
      _moving("moving"),
      _idle("idle"),
      _disabled("disabled"),
      _error("error"),
      _stepperMotor(stepperMotor),
      _motorControler(stepperMotor._motorControler)
  {
    _initState.setTransition(initialEvent,        &_disabled,      []{});
    _idle.setTransition     (moveEvent,           &_moving,        std::bind(&StepperMotorStateMachine::actionIdleToMove, this),
                                                                   std::bind(&StepperMotorStateMachine::waitForStandstill, this));
    _idle.setTransition     (disableEvent,        &_disabled,      std::bind(&StepperMotorStateMachine::actionDisable, this));
    _moving.setTransition   (stopEvent,           &_idle,          std::bind(&StepperMotorStateMachine::actionMovetoStop, this));
    _moving.setTransition   (errorEvent,          &_error,         []{});
    _moving.setTransition   (emergencyStopEvent,  &_error,         std::bind(&StepperMotorStateMachine::actionEmergencyStop, this));
    _moving.setTransition   (disableEvent,        &_disabled,      std::bind(&StepperMotorStateMachine::actionDisable, this));
    _disabled.setTransition (enableEvent,         &_idle,          std::bind(&StepperMotorStateMachine::actionEnable, this));
    _error.setTransition    (resetToIdleEvent,    &_idle,          std::bind(&StepperMotorStateMachine::actionResetError, this));
    _error.setTransition    (resetToDisableEvent, &_disabled,      std::bind(&StepperMotorStateMachine::actionResetError, this));
  }

  StepperMotorStateMachine::~StepperMotorStateMachine(){}

  void StepperMotorStateMachine::waitForStandstill(){

    if(!_motorControler->isMotorMoving()){

      _asyncActionActive.exchange(false);

      // Move may be stopped by
      // user requesting state transition
      if(hasRequestedState()){
        moveToRequestedState();
        return;
      }

      Event stateExitEvnt = stopEvent;

      // Motor stopped by itself
      if(!_stepperMotor.verifyMoveAction()){
        _stepperMotor._errorMode.exchange(StepperMotorError::MOVE_INTERRUPTED);

        _motorControler->setTargetPosition(_motorControler->getActualPosition());
        stateExitEvnt = errorEvent;
      }
      performTransition(stateExitEvnt);
      return;
    }
  }

  void StepperMotorStateMachine::actionIdleToMove(){
    _asyncActionActive.exchange(true);
    _motorControler->setTargetPosition(_stepperMotor._targetPositionInSteps);
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
    _stepperMotor._calibrationMode.exchange(StepperMotorCalibrationMode::NONE);
    _stepperMotor._errorMode.exchange(StepperMotorError::EMERGENCY_STOP);
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
} // namespace motordriver
} // namespace ChimeraTK
