/*
 * StepperMotorWithReferenceStateMachine.cc
 *
 *  Created on: Feb 17, 2017
 *      Author: vitimic
 */

#include "StepperMotorWithReferenceStateMachine.h"
#include "StepperMotorWithReference.h"

static const unsigned wakeupPeriodInMilliseconds = 500U;

namespace ChimeraTK{

  const Event StepperMotorWithReferenceStateMachine::calibEvent("calibEvent");
  const Event StepperMotorWithReferenceStateMachine::calcToleranceEvent("calcToleranceEvent");

  StepperMotorWithReferenceStateMachine::StepperMotorWithReferenceStateMachine(StepperMotorWithReference &stepperMotorWithReference) :
      StepperMotorStateMachine(stepperMotorWithReference),
      _calibrating("calibrating"),
      _calculatingTolerance("calculatingTolerance"),
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

    _calibrating.setTransition(StepperMotorStateMachine::stopEvent,
                               &_idle,
                               std::bind(&StepperMotorWithReferenceStateMachine::actionStop, this));
    _calibrating.setTransition(StepperMotorStateMachine::emergencyStopEvent, &_error, [this]{ actionEmergencyStop(); });

    _calculatingTolerance.setTransition(StepperMotorStateMachine::stopEvent, &_idle, [this]{ actionStop(); });
    _calculatingTolerance.setTransition(StepperMotorStateMachine::emergencyStopEvent, &_error, [this]{actionEmergencyStop();});
  }

  StepperMotorWithReferenceStateMachine::~StepperMotorWithReferenceStateMachine(){}

  void StepperMotorWithReferenceStateMachine::actionStop(){
    _stopAction.exchange(true);
    return;
  }

  void StepperMotorWithReferenceStateMachine::actionStartCalib(){
    _boolAsyncActionActive.exchange(true);
    _internalEventCallback = [this]{
                                      if(!_boolAsyncActionActive.load()){
                                        moveToRequestedState();
                                        performTransition(stopEvent);
                                        _internalEventCallback = []{};
                                      }
                                   };
      _asyncActionActive = std::async(std::launch::async, &StepperMotorWithReferenceStateMachine::calibrationThreadFunction, this);
  }

  void StepperMotorWithReferenceStateMachine::actionStartCalcTolercance(){
    _boolAsyncActionActive.exchange(true);
    _internalEventCallback = [this]{
                                      if(!_boolAsyncActionActive.load()){
                                        moveToRequestedState();
                                        performTransition(stopEvent);
                                        _internalEventCallback = []{};
                                      }
                                   };
      _asyncActionActive = std::async(std::launch::async, &StepperMotorWithReferenceStateMachine::toleranceCalcThreadFunction, this);
  }


  void StepperMotorWithReferenceStateMachine::calibrationThreadFunction(){
    _motor._calibrationFailed.exchange(false);
    _stopAction.exchange(false);
    _moveInterrupted.exchange(false);

    // At least one end switch disabled -> calibration not possible
    if(!(_motor._positiveEndSwitchEnabled.load() && _motor._negativeEndSwitchEnabled.load())){
      _motor._motorControler->setCalibrationTime(0);
      _motor._calibrationFailed.exchange(true);
      _motor._calibrationMode.exchange(StepperMotorCalibrationMode::NONE);
    }
    else{
      findEndSwitch(POSITIVE);
      _motor._calibPositiveEndSwitchInSteps.exchange(_motor._motorControler->getActualPosition());
      findEndSwitch(NEGATIVE);
      _motor._calibNegativeEndSwitchInSteps.exchange(_motor._motorControler->getActualPosition());

      if (_moveInterrupted.load() || _stopAction.load()){
        _motor._motorControler->setCalibrationTime(0);
        _motor._calibrationFailed.exchange(true);
        _motor._calibrationMode.exchange(StepperMotorCalibrationMode::NONE);
      }else{
        // Define positive axis with negative end switch as zero
        _motor._calibPositiveEndSwitchInSteps.exchange(_motor._calibPositiveEndSwitchInSteps.load() -
                                                       _motor._calibNegativeEndSwitchInSteps.load());
        _motor._calibNegativeEndSwitchInSteps.exchange(0);
        _motor._motorControler->setCalibrationTime(time(nullptr));
        _motor._calibrationMode.exchange(StepperMotorCalibrationMode::FULL);
        _motor.resetPositionMotorController(0);
      }
    }

    _boolAsyncActionActive.exchange(false);
    return;
  }

  void StepperMotorWithReferenceStateMachine::findEndSwitch(Sign sign){
    while (!isEndSwitchActive(sign)){
      if (_stopAction.load() || _moveInterrupted.load()){
        return;
      }else if (_motor._motorControler->getTargetPosition() != _motor._motorControler->getActualPosition() &&
                !_motor._motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
                !_motor._motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
        _moveInterrupted.exchange(true);
        return;
      }
      moveToEndSwitch(sign);
    }
    return;
  }

  void StepperMotorWithReferenceStateMachine::moveToEndSwitch(Sign sign){
    _motor._motorControler->setTargetPosition(_motor._motorControler->getActualPosition() + sign*50000);
    while (_motor._motorControler->isMotorMoving()){
      std::this_thread::sleep_for(std::chrono::milliseconds(wakeupPeriodInMilliseconds));
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
    _motor._toleranceCalcFailed.exchange(false);
    _motor._toleranceCalculated.exchange(false);
    _stopAction.exchange(false);
    _moveInterrupted.exchange(false);

    // This makes only sense once the motor is fully calibrated
    if (_motor._calibrationMode != StepperMotorCalibrationMode::FULL){
      _motor._toleranceCalculated.exchange(false);
      _motor._toleranceCalcFailed.exchange(true);
    }
    else{
      _motor._tolerancePositiveEndSwitch.exchange(getToleranceEndSwitch(POSITIVE));
      _motor._toleranceNegativeEndSwitch.exchange(getToleranceEndSwitch(NEGATIVE));

      if (_moveInterrupted.load() || _stopAction.load()){
        _motor._toleranceCalculated.exchange(false);
        _motor._toleranceCalcFailed.exchange(true);
      }
      else{
        _motor._toleranceCalculated.exchange(true);
      }
    }
    _boolAsyncActionActive.exchange(false);
    return;
  }

  int StepperMotorWithReferenceStateMachine::getPositionEndSwitch(Sign sign){
    if (sign == POSITIVE){
      return _motor._calibPositiveEndSwitchInSteps.load();
    }else{
      return _motor._calibNegativeEndSwitchInSteps.load();
    }
  }

  double StepperMotorWithReferenceStateMachine::getToleranceEndSwitch(Sign sign){
    double meanMeasurement = 0;
    double stdMeasurement = 0;
    const int N_TOLERANCE_CALC_SAMPLES = 10;
    double measurements[N_TOLERANCE_CALC_SAMPLES];

    int endSwitchPosition = getPositionEndSwitch(sign);

    //FIXME _index is only used in the test of the tolerance calculation
    //      -> Find a better way to write the test and remove _index
    for (_motor._index=0; _motor._index<N_TOLERANCE_CALC_SAMPLES; _motor._index++){
      if (_stopAction || _moveInterrupted){
        break;
      }
      //Move close to end switch
      _motor._motorControler->setTargetPosition(endSwitchPosition - sign*1000);
      while (_motor._motorControler->isMotorMoving()){
        std::this_thread::sleep_for(std::chrono::milliseconds(wakeupPeriodInMilliseconds));
      }
      //Check if in expected position
      if (_motor._motorControler->getTargetPosition() != _motor._motorControler->getActualPosition() &&
            !_motor._motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
            !_motor._motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
        _moveInterrupted.exchange(true);
        break;
      }
      // Try to move beyond end switch
      _motor._motorControler->setTargetPosition(endSwitchPosition + sign*1000);
      while (_motor._motorControler->isMotorMoving()){
        std::this_thread::sleep_for(std::chrono::milliseconds(wakeupPeriodInMilliseconds));
      }
      if (!isEndSwitchActive(sign)){
        _moveInterrupted.exchange(true);
        break;
      }

      // Mean calculation
      meanMeasurement += _motor._motorControler->getActualPosition() / N_TOLERANCE_CALC_SAMPLES;
      measurements[_motor._index] = _motor._motorControler->getActualPosition();
    } /* for (_motor._index=0; _motor._index<N_TOLERANCE_CALC_SAMPLES; _motor._index++) */

    // Compute variance
    if (!(_stopAction.load() || _moveInterrupted.load()) ){
      for (unsigned int i=0; i<N_TOLERANCE_CALC_SAMPLES; i++){
        stdMeasurement += ((measurements[i] - meanMeasurement) / (N_TOLERANCE_CALC_SAMPLES-1)) * (measurements[i] - meanMeasurement);
      }
    }

    return sqrt(stdMeasurement);
  }
}

