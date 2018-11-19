/*
 * StepperMotorWithReferenceStateMachine.cc
 *
 *  Created on: Feb 17, 2017
 *      Author: vitimic
 */

#include "StepperMotorWithReferenceStateMachine.h"
#include "StepperMotorWithReference.h"

namespace ChimeraTK{

  const Event StepperMotorWithReferenceStateMachine::calibEvent("calibEvent");
  const Event StepperMotorWithReferenceStateMachine::calcToleranceEvent("calcToleranceEvent");

  StepperMotorWithReferenceStateMachine::StepperMotorWithReferenceStateMachine(StepperMotorWithReference &stepperMotorWithReference) :
      StepperMotorStateMachine(stepperMotorWithReference),
      _calibrating("calibrating"),
      _calculatingTolerance("calculatingTolerance"),
      //_interruptingAction("interruptingAction"),
      _motor(stepperMotorWithReference),
      _stopAction(false),
      _moveInterrupted(false)
  {
    _idle.setTransition(calibEvent,
                                    &_calibrating,
                                    std::bind(&StepperMotorWithReferenceStateMachine::actionStartCalib, this));
    _idle.setTransition(StepperMotorWithReferenceStateMachine::calcToleranceEvent,
                                    &_calculatingTolerance,
                                    std::bind(&StepperMotorWithReferenceStateMachine::actionStartCalcTolercance, this));
//    _calibrating.setTransition(StateMachine::noEvent,
//                               &_calibrating,
//                               std::bind(&StepperMotorWithReferenceStateMachine::actionWaitForStandstill, this));
//    _calibrating.setTransition(StepperMotorStateMachine::actionCompleteEvent,
//                               &_idle, //_baseStateMachine,
//                               [](){});
    _calibrating.setTransition(StepperMotorStateMachine::stopEvent,
                               &_idle,
                               std::bind(&StepperMotorWithReferenceStateMachine::actionStop, this));
//    _calculatingTolerance.setTransition(StateMachine::noEvent,
//                                        &_calculatingTolerance,
//                                        std::bind(&StepperMotorWithReferenceStateMachine::actionWaitForStandstill, this));
//    _calculatingTolerance.setTransition(StepperMotorStateMachine::actionCompleteEvent,
//                                        &_idle, //_baseStateMachine,
//                                        [](){});
    _calculatingTolerance.setTransition(StepperMotorStateMachine::stopEvent,
                                        &_idle,
                                        std::bind(&StepperMotorWithReferenceStateMachine::actionStop, this));
    //TODO Introduce emergency stop transitions

//    _interruptingAction.setTransition(StateMachine::noEvent,
//                                      &_interruptingAction,
//                                      std::bind(&StepperMotorWithReferenceStateMachine::actionWaitForStandstill, this));
//    _interruptingAction.setTransition(StepperMotorStateMachine::actionCompleteEvent,
//                                      &_idle, //_baseStateMachine,
//                                      [](){});


    TransitionTable tT = _idle.getTransitionTable();
    auto tableSize = tT.size();
    std::cout << "  In /wRefCtor, addr this " << this << ", addr idle  " << &_idle << std::endl;
    std::cout << "  Table size " << tableSize << std::endl;
  }

  StepperMotorWithReferenceStateMachine::~StepperMotorWithReferenceStateMachine(){}

  void StepperMotorWithReferenceStateMachine::actionStop(){
    _stopAction = true;
    return;
  }

  void StepperMotorWithReferenceStateMachine::actionStartCalib(){
    if(!_asyncActionActive.valid() || (_asyncActionActive.valid() && _asyncActionActive.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
      _asyncActionActive = std::async(std::launch::async, &StepperMotorWithReferenceStateMachine::calibrationThreadFunction, this);
    }
  }

  void StepperMotorWithReferenceStateMachine::actionStartCalcTolercance(){
    if (!_asyncActionActive.valid() || (_asyncActionActive.valid() && _asyncActionActive.wait_for(std::chrono::microseconds(0)) == std::future_status::ready)){
      _asyncActionActive = std::async(std::launch::async, &StepperMotorWithReferenceStateMachine::toleranceCalcThreadFunction, this);
    }
  }


  void StepperMotorWithReferenceStateMachine::calibrationThreadFunction(){
    _motor._calibrationFailed = false;
    _stopAction = false;
    _moveInterrupted = false;

    // At least one end switch disabled -> calibration not possible
    if(!(_motor._positiveEndSwitchEnabled && _motor._negativeEndSwitchEnabled)){
      _motor._motorControler->setCalibrationTime(0);
      _motor._calibrationFailed.store(true);
      _motor._calibrationMode.store(StepperMotorCalibrationMode::NONE);
    }
    else{
      findEndSwitch(POSITIVE);
      _motor._calibPositiveEndSwitchInSteps = _motor._motorControler->getActualPosition();
      findEndSwitch(NEGATIVE);
      _motor._calibNegativeEndSwitchInSteps = _motor._motorControler->getActualPosition();

      if (_moveInterrupted || _stopAction){
        _motor._motorControler->setCalibrationTime(0);
        _motor._calibrationFailed.store(true);
        _motor._calibrationMode.store(StepperMotorCalibrationMode::NONE);
      }else{
        _motor._calibPositiveEndSwitchInSteps = _motor._calibPositiveEndSwitchInSteps -
        _motor._calibNegativeEndSwitchInSteps;
        _motor._calibNegativeEndSwitchInSteps = 0;
        _motor._motorControler->setCalibrationTime(time(NULL));
        _motor._calibrationMode.store(StepperMotorCalibrationMode::FULL);
        _motor.resetPositionMotorController(0);
      }
    }

    waitForStandstill();
    return;
  }

  void StepperMotorWithReferenceStateMachine::findEndSwitch(Sign sign){
    while (!isEndSwitchActive(sign)){
      if (_stopAction || _moveInterrupted){
        return;
      }else if (_motor._motorControler->getTargetPosition() != _motor._motorControler->getActualPosition() &&
                !_motor._motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
                !_motor._motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
        _moveInterrupted = true;
        return;
      }
      moveToEndSwitch(sign);
    }
    return;
  }

  void StepperMotorWithReferenceStateMachine::moveToEndSwitch(Sign sign){
    _motor._motorControler->setTargetPosition(_motor._motorControler->getActualPosition() + sign*50000);
    while (_motor._motorControler->isMotorMoving()){
      std::this_thread::sleep_for(std::chrono::microseconds(100));       // TODO Increase period
    }
  }

  bool StepperMotorWithReferenceStateMachine::isEndSwitchActive(Sign sign){
    if (sign == POSITIVE){
      if (_motor._motorControler->getReferenceSwitchData().getPositiveSwitchActive()){
        return true;
      }else{
        return false;
      }
    }else{
      if (_motor._motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
        return true;
      }else{
        return false;
      }
    }
  }

  void StepperMotorWithReferenceStateMachine::toleranceCalcThreadFunction(){
    _motor._toleranceCalcFailed = false;
    _motor._toleranceCalculated = false;
    _stopAction = false;
    _moveInterrupted = false;

    // This makes only sense once the motor is calibrated
    if (_motor._motorControler->getCalibrationTime() == 0){ //FIXME Use the SW flag
      _motor._toleranceCalculated = false;
      _motor._toleranceCalcFailed = true;
    }
    else{
      _motor._tolerancePositiveEndSwitch = getToleranceEndSwitch(POSITIVE);
      _motor._toleranceNegativeEndSwitch = getToleranceEndSwitch(NEGATIVE);

      if (_moveInterrupted || _stopAction){
        _motor._toleranceCalculated = false;
        _motor._toleranceCalcFailed = true;
      }
      else{
        _motor._toleranceCalculated = true;
      }
    }
    waitForStandstill();
    return;
  }

  int StepperMotorWithReferenceStateMachine::getPositionEndSwitch(Sign sign){
    if (sign == POSITIVE){
      return _motor._calibPositiveEndSwitchInSteps;
    }else{
      return _motor._calibNegativeEndSwitchInSteps;
    }
  }

  double StepperMotorWithReferenceStateMachine::getToleranceEndSwitch(Sign sign){
    double meanMeasurement = 0;
    double stdMeasurement = 0;
    double measurements[10];

    int endSwitchPosition = getPositionEndSwitch(sign);

    //FIXME _index is only used in the test of the tolerance calculation
    //      -> Find a better way to write the test and remove _index
    for (_motor._index=0; _motor._index<10; _motor._index++){
      if (_stopAction || _moveInterrupted){
        break;
      }
      _motor._motorControler->setTargetPosition(endSwitchPosition - sign*1000);
      while (_motor._motorControler->isMotorMoving()){
        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
      if (_motor._motorControler->getTargetPosition() != _motor._motorControler->getActualPosition() &&
            !_motor._motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
            !_motor._motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
        _moveInterrupted = true;
        break;
      }
      _motor._motorControler->setTargetPosition(endSwitchPosition + sign*1000);
      while (_motor._motorControler->isMotorMoving()){
        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
      if (!isEndSwitchActive(sign)){
        _moveInterrupted = true;
        break;
      }

      meanMeasurement += _motor._motorControler->getActualPosition() / 10.;
      measurements[_motor._index] = _motor._motorControler->getActualPosition();
    }

    if (!(_stopAction || _moveInterrupted) ){
      for (unsigned int i=0; i<10; i++){
        stdMeasurement += ((measurements[i] - meanMeasurement) / 9) * (measurements[i] - meanMeasurement);
      }
    }

    return sqrt(stdMeasurement);
  }
}

