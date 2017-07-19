/*
 * StepperMotorStateMachine.cc
 *
 *  Created on: Feb 2, 2017
 *      Author: vitimic
 */

#include "StepperMotorStateMachine.h"
#include "StepperMotor.h"
#include <memory>

namespace ChimeraTK{

  Event StepperMotorStateMachine::moveEvent("moveEvent");
  Event StepperMotorStateMachine::stopEvent("stopEvent");
  Event StepperMotorStateMachine::emergencyStopEvent("emergencyStopEvent");
  Event StepperMotorStateMachine::actionCompleteEvent("actionCompletedEvent");
  Event StepperMotorStateMachine::disableEvent("disableEvent");

  StepperMotorStateMachine::StepperMotorStateMachine(ChimeraTK::StepperMotor &stepperMotor) :
	  StateMachine("StepperMotorStateMachine"),
	  _moving("movingState"),
	  _idle("idleState"),
	  _stop("stopState"),
	  _emergencyStop("emergencyStop"),
	  _disable("disable"),
	  _stepperMotor(stepperMotor),
	  _motorControler(stepperMotor._motorControler){
    _initState.setTransition    (noEvent,             &_idle,          std::bind(&StepperMotorStateMachine::actionToIdle, this));
    _idle.setTransition         (moveEvent,           &_moving,        std::bind(&StepperMotorStateMachine::actionIdleToMove, this));
    _idle.setTransition         (disableEvent,        &_disable,       std::bind(&StepperMotorStateMachine::actionMoveToFullStep, this));
    _moving.setTransition       (noEvent,             &_moving,        std::bind(&StepperMotorStateMachine::getActionCompleteEvent, this));
    _moving.setTransition       (actionCompleteEvent, &_idle,          std::bind(&StepperMotorStateMachine::actionToIdle, this));
    _moving.setTransition       (stopEvent,           &_stop,          std::bind(&StepperMotorStateMachine::actionMovetoStop, this));
    _moving.setTransition       (emergencyStopEvent,  &_emergencyStop, std::bind(&StepperMotorStateMachine::actionEmergencyStop, this));
    _moving.setTransition       (disableEvent,        &_disable,       std::bind(&StepperMotorStateMachine::actionMoveToFullStep, this));
    _disable.setTransition      (noEvent,             &_disable,       std::bind(&StepperMotorStateMachine::getActionCompleteEvent, this));
    _disable.setTransition      (actionCompleteEvent, &_idle,          std::bind(&StepperMotorStateMachine::actionDisable, this));
    _stop.setTransition         (noEvent,             &_stop,          std::bind(&StepperMotorStateMachine::getActionCompleteEvent, this));
    _stop.setTransition         (actionCompleteEvent, &_idle,          std::bind(&StepperMotorStateMachine::actionToIdle, this));
    _emergencyStop.setTransition(noEvent,             &_emergencyStop, std::bind(&StepperMotorStateMachine::getActionCompleteEvent, this));
    _emergencyStop.setTransition(actionCompleteEvent, &_idle,          std::bind(&StepperMotorStateMachine::actionToIdle, this));
  }

  StepperMotorStateMachine::~StepperMotorStateMachine(){}

  void StepperMotorStateMachine::getActionCompleteEvent(){
    if (!_motorControler->isMotorMoving()){
      _internEvent=StepperMotorStateMachine::actionCompleteEvent;
    }else{
      _internEvent=StateMachine::noEvent;
    }
  }

  void StepperMotorStateMachine::actionIdleToMove(){
    _motorControler->setTargetPosition(_stepperMotor._targetPositionInSteps);
  }

  void StepperMotorStateMachine::actionToIdle(){}

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
    _motorControler->setCalibrationTime(0);
  }

  void StepperMotorStateMachine::actionDisable(){
    _motorControler->setMotorCurrentEnabled(false);
  }

  bool StepperMotorStateMachine::propagateEvent(){
    if (_currentState->isEventUnknown() && _currentState->getName() == "idleState"){
      return true;
    }else{
      return false;
    }
  }
}
