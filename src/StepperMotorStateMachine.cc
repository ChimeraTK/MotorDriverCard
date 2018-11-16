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

  Event StepperMotorStateMachine::initialEvent("initialEvent");
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
      _disable("disabledState"),
      _error("error"),
      _stepperMotor(stepperMotor),
      _motorControler(stepperMotor._motorControler)
  {
    _initState.setTransition(initialEvent,        &_idle,          []{});
    _idle.setTransition     (moveEvent,           &_moving,        std::bind(&StepperMotorStateMachine::actionIdleToMove, this));
    _idle.setTransition     (disableEvent,        &_disable,       std::bind(&StepperMotorStateMachine::actionMoveToFullStep, this));
    //_moving.setTransition       (noEvent,             &_moving,        std::bind(&StepperMotorStateMachine::actionWaitForStandstill, this));  /* Reset to getActionCompleteEvent? */
    //_moving.setTransition       (actionCompleteEvent, &_idle,          std::bind(&StepperMotorStateMachine::actionToIdle, this));
    _moving.setTransition   (stopEvent,           &_idle,          std::bind(&StepperMotorStateMachine::actionMovetoStop, this));
    _moving.setTransition   (emergencyStopEvent,  &_error,         std::bind(&StepperMotorStateMachine::actionEmergencyStop, this));
    _moving.setTransition   (disableEvent,        &_disable,       std::bind(&StepperMotorStateMachine::actionMoveToFullStep, this));
    //_disable.setTransition      (noEvent,             &_disable,       std::bind(&StepperMotorStateMachine::actionWaitForStandstill, this));  /* Reset to getActionCompleteEvent? */
    _disable.setTransition  (enableEvent,         &_idle,          std::bind(&StepperMotorStateMachine::actionEnable, this));
    //_stop.setTransition         (noEvent,             &_stop,          std::bind(&StepperMotorStateMachine::actionWaitForStandstill, this));  /* Reset to getActionCompleteEvent? */
    //_stop.setTransition         (actionCompleteEvent, &_idle,          std::bind(&StepperMotorStateMachine::actionToIdle, this));
    //_emergencyStop.setTransition(noEvent,             &_emergencyStop, std::bind(&StepperMotorStateMachine::actionWaitForStandstill, this));  /* Reset to getActionCompleteEvent? */
    //_emergencyStop.setTransition(actionCompleteEvent, &_idle,          std::bind(&StepperMotorStateMachine::actionToIdle, this));
  }

  StepperMotorStateMachine::~StepperMotorStateMachine(){}

  void StepperMotorStateMachine::waitForStandstill(){
//    while(_motorControler->isMotorMoving()){
//      std::this_thread::sleep_for(std::chrono::seconds(1));
//    }
    if(!_motorControler->isMotorMoving()){

      // Set the propagated state or return to idle by stop event
      // Either of them does not take effect
      moveToRequestedState();
      performTransition(stopEvent);
      _internalEventCallback = []{};
    }
  }

  void StepperMotorStateMachine::actionIdleToMove(){
//    if (!_asyncActionActive.valid() || (_asyncActionActive.valid() && _asyncActionActive.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
//
//      _asyncActionActive = std::async(std::launch::async, &StepperMotorStateMachine::waitForStandstill, this);
//    }
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
    _motorControler->setMotorCurrentEnabled(false);
    _motorControler->setEndSwitchPowerEnabled(false);
  }

}
