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
      StateMachine("StepperMotorStateMachine"),
      _moving("movingState"),
      _idle("idleState"),
      _stop("stopState"),
      _stepperMotor(stepperMotor){
//    _moving.setEventGeneration(std::bind(&StepperMotorStateMachine::getActionCompleteEvent, this));
//    _stop.setEventGeneration(std::bind(&StepperMotorStateMachine::getActionCompleteEvent, this));
    _moving.setTransition   (noEvent,             &_moving, std::bind(&StepperMotorStateMachine::getActionCompleteEvent, this));
    _stop.setTransition     (noEvent,             &_stop,   std::bind(&StepperMotorStateMachine::getActionCompleteEvent, this));
    _initState.setTransition(noEvent,             &_idle,   std::bind(&StepperMotorStateMachine::actionToIdle, this));
    _idle.setTransition     (moveEvent,           &_moving, std::bind(&StepperMotorStateMachine::actionIdleToMove, this));
    _moving.setTransition   (actionCompleteEvent, &_idle,   std::bind(&StepperMotorStateMachine::actionToIdle, this));
    _moving.setTransition   (stopEvent,           &_stop,   std::bind(&StepperMotorStateMachine::actionMovetoStop, this));
    _stop.setTransition     (actionCompleteEvent, &_idle,   std::bind(&StepperMotorStateMachine::actionToIdle, this));
  }

  void StepperMotorStateMachine::getActionCompleteEvent(){
    if (!_stepperMotor._motorControler->isMotorMoving()){
      _internEvent=StepperMotorStateMachine::actionCompleteEvent;
    }else{
      _internEvent=StateMachine::noEvent;
    }
  }

  void StepperMotorStateMachine::actionIdleToMove(){
      _stepperMotor._motorControler->setTargetPosition(_stepperMotor._targetPositionInSteps);
    }

  void StepperMotorStateMachine::actionToIdle(){
  }

  void StepperMotorStateMachine::actionMovetoStop(){
    float  currentPos = _stepperMotor.getCurrentPosition();
    _stepperMotor._motorControler->setTargetPosition(currentPos);
  }

  bool StepperMotorStateMachine::propagateEvent(){
    if (_unknownEvent && _currentState->getName() == "idleState"){
      return true;
    }else{
      return false;
    }
  }
}
