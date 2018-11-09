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

  Event StepperMotorStateMachine::moveEvent("moveEvent");
  Event StepperMotorStateMachine::stopEvent("stopEvent");
  Event StepperMotorStateMachine::emergencyStopEvent("emergencyStopEvent");
  Event StepperMotorStateMachine::actionCompleteEvent("actionCompletedEvent");
  Event StepperMotorStateMachine::enableEvent("enableEvent");
  Event StepperMotorStateMachine::disableEvent("disableEvent");

  StepperMotorStateMachine::StepperMotorStateMachine(ChimeraTK::StepperMotor &stepperMotor) :
      StateMachine(),
      _moving("movingState"),
      _idle("idleState"),
//      _stop("stopState"),
//      _emergencyStop("emergencyStop"),
      _disable("disable"),
      _error("error"),
      _stepperMotor(stepperMotor),
      _motorControler(stepperMotor._motorControler)
  {
    _initState.setTransition(noEvent,             &_idle,          []{});
    _idle.setTransition     (moveEvent,           &_moving,        std::bind(&StepperMotorStateMachine::actionIdleToMove, this));
    _idle.setTransition     (disableEvent,        &_disable,       std::bind(&StepperMotorStateMachine::actionMoveToFullStep, this));
    //_moving.setTransition       (noEvent,             &_moving,        std::bind(&StepperMotorStateMachine::actionWaitForStandstill, this));  /* Reset to getActionCompleteEvent? */
    //_moving.setTransition       (actionCompleteEvent, &_idle,          std::bind(&StepperMotorStateMachine::actionToIdle, this));
    _moving.setTransition   (stopEvent,           &_idle,          std::bind(&StepperMotorStateMachine::actionMovetoStop, this));
    _moving.setTransition   (emergencyStopEvent,  &_error,         std::bind(&StepperMotorStateMachine::actionEmergencyStop, this));
    _moving.setTransition   (disableEvent,        &_disable,       std::bind(&StepperMotorStateMachine::actionMoveToFullStep, this));
    //_disable.setTransition      (noEvent,             &_disable,       std::bind(&StepperMotorStateMachine::actionWaitForStandstill, this));  /* Reset to getActionCompleteEvent? */
    _disable.setTransition  (enableEvent,         &_idle,          []{}); // FIXME If done like this, there should be an enable action
    //_stop.setTransition         (noEvent,             &_stop,          std::bind(&StepperMotorStateMachine::actionWaitForStandstill, this));  /* Reset to getActionCompleteEvent? */
    //_stop.setTransition         (actionCompleteEvent, &_idle,          std::bind(&StepperMotorStateMachine::actionToIdle, this));
    //_emergencyStop.setTransition(noEvent,             &_emergencyStop, std::bind(&StepperMotorStateMachine::actionWaitForStandstill, this));  /* Reset to getActionCompleteEvent? */
    //_emergencyStop.setTransition(actionCompleteEvent, &_idle,          std::bind(&StepperMotorStateMachine::actionToIdle, this));
  }

  StepperMotorStateMachine::~StepperMotorStateMachine(){}

//  void StepperMotorStateMachine::getActionCompleteEvent(){
//    if (!_motorControler->isMotorMoving()){
//      _internEvent=StepperMotorStateMachine::actionCompleteEvent;
//    }else{
//      _internEvent=StateMachine::noEvent;
//    }
//  }
//  void StepperMotorStateMachine::getActionCompletedEvent(){
//    if (!_asyncActionActive.valid() || (_asyncActionActive.valid() && _asyncActionActive.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
//      _internEvent = StepperMotorStateMachine::actionCompleteEvent;
//      return;
//    }
//    return;
//  }

  void StepperMotorStateMachine::waitForStandstill(){
    while(_motorControler->isMotorMoving()){
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // Set the propagated state
    setRequestedState();

    // "Internal event"
//    if(*_currentState == _moving){
//      // Still moving, no propagated event
//      _currentState = &_idle;
//    }
    setAndProcessUserEvent(stopEvent);
  }

  void StepperMotorStateMachine::actionIdleToMove(){
    if (!_asyncActionActive.valid() || (_asyncActionActive.valid() && _asyncActionActive.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
      _motorControler->setTargetPosition(_stepperMotor._targetPositionInSteps);
      _asyncActionActive = std::async(std::launch::async, &StepperMotorStateMachine::waitForStandstill, this);
    }
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

  void StepperMotorStateMachine::actionDisable(){
    _motorControler->setMotorCurrentEnabled(false);
    _motorControler->setEndSwitchPowerEnabled(false);
  }

}
