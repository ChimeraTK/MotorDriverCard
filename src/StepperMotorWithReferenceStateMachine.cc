/*
 * StepperMotorWithReferenceStateMachine.cc
 *
 *  Created on: Feb 17, 2017
 *      Author: vitimic
 */

#include "StepperMotorWithReferenceStateMachine.h"
#include "StepperMotorWithReference.h"

namespace ChimeraTK{

  Event StepperMotorWithReferenceStateMachine::calibEvent("calibEvent");
  Event StepperMotorWithReferenceStateMachine::calcToleranceEvent("calcToleranceEvent");

  StepperMotorWithReferenceStateMachine::StepperMotorWithReferenceStateMachine(StepperMotorWithReference &stepperMotorWithReference) :
	      StateMachine("StepperMotorWithReferenceStateMachine"),
	      _calibrating("calibrating"),
	      _calculatingTolerance("calculatingTolerance"),
	      _interruptingAction("interruptingAction"),
	      _baseStateMachine(stepperMotorWithReference),
	      _stepperMotorWithReference(stepperMotorWithReference),
	      _future(),
	      _stopAction(false),
	      _moveInterrupted(false){
    _initState.setTransition(StateMachine::noEvent, &_baseStateMachine, [](){});
    _baseStateMachine.setTransition(StepperMotorWithReferenceStateMachine::calibEvent,
                                    &_calibrating,
                                    std::bind(&StepperMotorWithReferenceStateMachine::actionStartCalib, this));
    _baseStateMachine.setTransition(StepperMotorWithReferenceStateMachine::calcToleranceEvent,
                                    &_calculatingTolerance,
                                    std::bind(&StepperMotorWithReferenceStateMachine::actionStartCalcTolercance, this));
    _calibrating.setTransition(StateMachine::noEvent,
                               &_calibrating,
                               std::bind(&StepperMotorWithReferenceStateMachine::getActionCompletedEvent, this));
    _calibrating.setTransition(StepperMotorStateMachine::actionCompleteEvent,
                               &_baseStateMachine,
                               [](){});
    _calibrating.setTransition(StepperMotorStateMachine::stopEvent,
                               &_interruptingAction,
                               std::bind(&StepperMotorWithReferenceStateMachine::actionStop, this));
    _calculatingTolerance.setTransition(StateMachine::noEvent,
                                        &_calculatingTolerance,
                                        std::bind(&StepperMotorWithReferenceStateMachine::getActionCompletedEvent, this));
    _calculatingTolerance.setTransition(StepperMotorStateMachine::actionCompleteEvent,
                                        &_baseStateMachine,
                                        [](){});
    _calculatingTolerance.setTransition(StepperMotorStateMachine::stopEvent,
                                        &_interruptingAction,
                                        std::bind(&StepperMotorWithReferenceStateMachine::actionStop, this));
    _interruptingAction.setTransition(StateMachine::noEvent,
                                      &_interruptingAction,
                                      std::bind(&StepperMotorWithReferenceStateMachine::getActionCompletedEvent, this));
    _interruptingAction.setTransition(StepperMotorStateMachine::actionCompleteEvent,
                                      &_baseStateMachine,
                                      [](){});
  }

  StepperMotorWithReferenceStateMachine::~StepperMotorWithReferenceStateMachine(){}

  void StepperMotorWithReferenceStateMachine::actionStop(){
    _stopAction = true;
    return;
  }

  void StepperMotorWithReferenceStateMachine::actionStartCalib(){
    if (!_future.valid() || (_future.valid() && _future.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
      _future = std::async(std::launch::async, &StepperMotorWithReferenceStateMachine::calibrationThreadFunction, this);
    }
  }

  void StepperMotorWithReferenceStateMachine::actionStartCalcTolercance(){
    if (!_future.valid() || (_future.valid() && _future.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
      _future = std::async(std::launch::async, &StepperMotorWithReferenceStateMachine::toleranceCalcThreadFunction, this);
    }
  }

  void StepperMotorWithReferenceStateMachine::getActionCompletedEvent(){
    if (!_future.valid() || (_future.valid() && _future.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
      _internEvent = StepperMotorStateMachine::actionCompleteEvent;
      return;
    }
    return;
  }

  void StepperMotorWithReferenceStateMachine::calibrationThreadFunction(){
    _stepperMotorWithReference._calibrationFailed = false;
    _stopAction = false;
    _moveInterrupted = false;
    if(!(_stepperMotorWithReference._positiveEndSwitchEnabled && _stepperMotorWithReference._negativeEndSwitchEnabled)){
      _stepperMotorWithReference._motorControler->setCalibrationTime(0);
      _stepperMotorWithReference._calibrationFailed = true;
      _stepperMotorWithReference. _calibrationMode.store(StepperMotorCalibrationMode::NONE);
      return;
    }
    findEndSwitch(POSITIVE);
    _stepperMotorWithReference._calibPositiveEndSwitchInSteps = _stepperMotorWithReference._motorControler->getActualPosition();
    findEndSwitch(NEGATIVE);
    _stepperMotorWithReference._calibNegativeEndSwitchInSteps = _stepperMotorWithReference._motorControler->getActualPosition();

    if (_moveInterrupted || _stopAction){
      _stepperMotorWithReference._motorControler->setCalibrationTime(0);
      _stepperMotorWithReference._calibrationFailed = true;
      _stepperMotorWithReference. _calibrationMode.store(StepperMotorCalibrationMode::NONE);
      return;
    }else{
      _stepperMotorWithReference._calibPositiveEndSwitchInSteps = _stepperMotorWithReference._calibPositiveEndSwitchInSteps -
      _stepperMotorWithReference._calibNegativeEndSwitchInSteps;
      _stepperMotorWithReference._calibNegativeEndSwitchInSteps = 0;
      _stepperMotorWithReference._motorControler->setCalibrationTime(time(NULL));
      _stepperMotorWithReference. _calibrationMode.store(StepperMotorCalibrationMode::FULL);
      _stepperMotorWithReference.resetPositionMotorController(0);
      return;
    }
  }

  void StepperMotorWithReferenceStateMachine::findEndSwitch(Sign sign){
    while (!isEndSwitchActive(sign)){
      if (_stopAction || _moveInterrupted){
        return;
      }else if (_stepperMotorWithReference._motorControler->getTargetPosition() != _stepperMotorWithReference._motorControler->getActualPosition() &&
                !_stepperMotorWithReference._motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
                !_stepperMotorWithReference._motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
        _moveInterrupted = true;
        return;
      }
      moveToEndSwitch(sign);
    }
    return;
  }

  void StepperMotorWithReferenceStateMachine::moveToEndSwitch(Sign sign){
    _stepperMotorWithReference._motorControler->setTargetPosition(_stepperMotorWithReference._motorControler->getActualPosition() + sign*50000);
    while (_stepperMotorWithReference._motorControler->isMotorMoving()){
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
  }

  bool StepperMotorWithReferenceStateMachine::isEndSwitchActive(Sign sign){
    if (sign == POSITIVE){
      if (_stepperMotorWithReference._motorControler->getReferenceSwitchData().getPositiveSwitchActive()){
        return true;
      }else{
        return false;
      }
    }else{
      if (_stepperMotorWithReference._motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
        return true;
      }else{
        return false;
      }
    }
  }

  void StepperMotorWithReferenceStateMachine::toleranceCalcThreadFunction(){
    _stepperMotorWithReference._toleranceCalcFailed = false;
    _stepperMotorWithReference._toleranceCalculated = false;
    _stopAction = false;
    _moveInterrupted = false;
    if (_stepperMotorWithReference._motorControler->getCalibrationTime() == 0){
      _stepperMotorWithReference._toleranceCalculated = false;
      _stepperMotorWithReference._toleranceCalcFailed = true;
      return;
    }

    _stepperMotorWithReference._tolerancePositiveEndSwitch = getToleranceEndSwitch(POSITIVE);
    _stepperMotorWithReference._toleranceNegativeEndSwitch = getToleranceEndSwitch(NEGATIVE);

    if (_moveInterrupted || _stopAction){
      _stepperMotorWithReference._toleranceCalculated = false;
      _stepperMotorWithReference._toleranceCalcFailed = true;
      return;
    }
    _stepperMotorWithReference._toleranceCalculated = true;
    return;
  }

  int StepperMotorWithReferenceStateMachine::getPositionEndSwitch(Sign sign){
    if (sign == POSITIVE){
      return _stepperMotorWithReference._calibPositiveEndSwitchInSteps;
    }else{
      return _stepperMotorWithReference._calibNegativeEndSwitchInSteps;
    }
  }

  double StepperMotorWithReferenceStateMachine::getToleranceEndSwitch(Sign sign){
    double meanMeasurement = 0;
    double stdMeasurement = 0;
    double measurements[10];

    int endSwitchPosition = getPositionEndSwitch(sign);

    //FIXME _index is only used in the test of the tolerance calculation
    //      -> Find a better way to write the test and remove _index
    for (_stepperMotorWithReference._index=0; _stepperMotorWithReference._index<10; _stepperMotorWithReference._index++){
      if (_stopAction || _moveInterrupted){
        break;
      }
      _stepperMotorWithReference._motorControler->setTargetPosition(endSwitchPosition - sign*1000);
      while (_stepperMotorWithReference._motorControler->isMotorMoving()){
        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
      if (_stepperMotorWithReference._motorControler->getTargetPosition() != _stepperMotorWithReference._motorControler->getActualPosition() &&
            !_stepperMotorWithReference._motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
            !_stepperMotorWithReference._motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
        _moveInterrupted = true;
        break;
      }
      _stepperMotorWithReference._motorControler->setTargetPosition(endSwitchPosition + sign*1000);
      while (_stepperMotorWithReference._motorControler->isMotorMoving()){
        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
      if (!isEndSwitchActive(sign)){
        _moveInterrupted = true;
        break;
      }

      meanMeasurement += _stepperMotorWithReference._motorControler->getActualPosition() / 10.;
      measurements[_stepperMotorWithReference._index] = _stepperMotorWithReference._motorControler->getActualPosition();
    }

    if (!(_stopAction || _moveInterrupted) ){
      for (unsigned int i=0; i<10; i++){
        stdMeasurement += ((measurements[i] - meanMeasurement) / 9) * (measurements[i] - meanMeasurement);
      }
    }

    return sqrt(stdMeasurement);
  }
}

