/*
 * StepperMotorStateMachine.cc
 *
 *  Created on: Feb 2, 2017
 *      Author: vitimic
 */

#include "StepperMotorStateMachine.h"
#include <memory>

namespace ChimeraTK{

  Event StepperMotorStateMachine::moveEvent("moveEvent");
  Event StepperMotorStateMachine::stopEvent("stopEvent");
  Event StepperMotorStateMachine::actionCompleteEvent("actionCompletedEvent");

  StepperMotorStateMachine::StepperMotorStateMachine(StepperMotor &stepperMotor) :
      StateMachine<StepperMotor>(stepperMotor, "StepperMotorStateMachine"),
      _moving("movingState"),
      _idle("idleState"),
      _stop("stopState"){
    _moving.setEventGeneration(std::bind(&StepperMotorStateMachine::getActionCompleteEvent, this));
    _stop.setEventGeneration(std::bind(&StepperMotorStateMachine::getActionCompleteEvent, this));
    _initState.setTransition(noEvent,             &_idle,   std::bind(&StepperMotorStateMachine::actionToIdle, this));
    _idle.setTransition     (moveEvent,           &_moving, std::bind(&StepperMotorStateMachine::actionIdleToMove, this));
    _moving.setTransition   (actionCompleteEvent, &_idle,   std::bind(&StepperMotorStateMachine::actionToIdle, this));
    _moving.setTransition   (stopEvent,           &_stop,   std::bind(&StepperMotorStateMachine::actionMovetoStop, this));
    _stop.setTransition     (actionCompleteEvent, &_idle,   std::bind(&StepperMotorStateMachine::actionToIdle, this));
  }

  Event StepperMotorStateMachine::getActionCompleteEvent(){
    if (!_frontEnd._motorControler->isMotorMoving()){
      return StepperMotorStateMachine::actionCompleteEvent;
    }else{
      return State::noEvent;
    }
  }

  void StepperMotorStateMachine::actionIdleToMove(){
      _frontEnd._motorControler->setTargetPosition(_frontEnd._targetPositionInSteps);
    }

  void StepperMotorStateMachine::actionToIdle(){}

  void StepperMotorStateMachine::actionMovetoStop(){
    float  currentPos = _frontEnd.getCurrentPosition();
    _frontEnd._motorControler->setTargetPosition(currentPos);
  }
}
