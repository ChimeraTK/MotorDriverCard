/*
 * StepperMotorWithReference.cc
 *
 *  Created on: Feb 17, 2017
 *      Author: vitimic
 */

#include "StepperMotorWithReference.h"
#include "StepperMotorWithReferenceStateMachine.h"

namespace ChimeraTK{
  StepperMotorWithReference::StepperMotorWithReference(const StepperMotorParameters & parameters)
    : BasicStepperMotor(),
      _positiveEndSwitchEnabled(false),
      _negativeEndSwitchEnabled(false),
      _calibrationFailed(false),
      _toleranceCalcFailed(false),
      _toleranceCalculated(false),
      _calibNegativeEndSwitchInSteps(-std::numeric_limits<int>::max()),
      _calibPositiveEndSwitchInSteps(std::numeric_limits<int>::max()),
      _tolerancePositiveEndSwitch(0),
      _toleranceNegativeEndSwitch(0)
  {
    _motorDriverCard = mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(
          parameters.deviceName,
          parameters.moduleName, parameters. configFileName);
    _motorControler = _motorDriverCard->getMotorControler(parameters.driverId);
    _stepperMotorUnitsConverter = parameters.motorUnitsConverter;
    _encoderUnitsConverter = parameters.encoderUnitsConverter;
    _stateMachine.reset(new StepperMotorWithReferenceStateMachine(*this));
    _targetPositionInSteps = _motorControler->getTargetPosition();
    _negativeEndSwitchEnabled = _motorControler->getReferenceSwitchData().getNegativeSwitchEnabled();
    _positiveEndSwitchEnabled = _motorControler->getReferenceSwitchData().getPositiveSwitchEnabled();
    initStateMachine();
    loadEndSwitchCalibration();
  }

  StepperMotorWithReference::~StepperMotorWithReference(){}

  bool StepperMotorWithReference::limitsOK(int newPositionInSteps){
    if (newPositionInSteps >= _calibNegativeEndSwitchInSteps.load() && newPositionInSteps <= _calibPositiveEndSwitchInSteps.load()) {
      return BasicStepperMotor::limitsOK(newPositionInSteps);
    }
    else{
      return false;
    }
  }

  StepperMotorRet StepperMotorWithReference::setActualPositionInSteps(int actualPositionInSteps){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }
    setActualPositionActions(actualPositionInSteps);

    // Overwrite end switch calibration and reset calibration mode
    _calibNegativeEndSwitchInSteps.exchange(std::numeric_limits<int>::min());
    _calibPositiveEndSwitchInSteps.exchange(std::numeric_limits<int>::max());

    return StepperMotorRet::SUCCESS;
  }

  StepperMotorRet StepperMotorWithReference::translateAxisInSteps(int translationInSteps){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }

    translateAxisActions(translationInSteps);

    // If motor has been calibrated, translate also end switch positions
    if(this->_calibrationMode.load() == StepperMotorCalibrationMode::FULL){
      if(checkIfOverflow(_calibPositiveEndSwitchInSteps.load(), translationInSteps) ||
         checkIfOverflow(_calibNegativeEndSwitchInSteps.load(), translationInSteps))
      {
        return StepperMotorRet::ERROR_INVALID_PARAMETER;
      }
      else{
        _calibPositiveEndSwitchInSteps.fetch_add(translationInSteps);
        _calibNegativeEndSwitchInSteps.fetch_add(translationInSteps);
      }
    }
    return StepperMotorRet::SUCCESS;
  }

  StepperMotorRet StepperMotorWithReference::calibrate(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }
    _stateMachine->setAndProcessUserEvent(StepperMotorWithReferenceStateMachine::calibEvent);
    return StepperMotorRet::SUCCESS;
  }

  StepperMotorRet StepperMotorWithReference::determineTolerance(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERROR_SYSTEM_IN_ACTION;
    }
    _stateMachine->setAndProcessUserEvent(StepperMotorWithReferenceStateMachine::calcToleranceEvent);
    return StepperMotorRet::SUCCESS;
  }

  // FIXME Combine this with getError from base class
  StepperMotorError StepperMotorWithReference::getError(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (motorActive() &&
        _motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
        _motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
      return BOTH_END_SWITCHES_ON;
    }
    if (!motorActive()){
      if (_toleranceCalcFailed.load() || _calibrationFailed.load()){
        return ACTION_ERROR;
      }else if ((_motorControler->getTargetPosition() != _motorControler->getActualPosition()) &&
                !_motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
                !_motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
        return ACTION_ERROR;
      }
    }
    if (_toleranceCalculated){
      if (_motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
          std::abs(_motorControler->getActualPosition() -  _calibPositiveEndSwitchInSteps.load()) > 3 * _tolerancePositiveEndSwitch.load()){
        return CALIBRATION_LOST;
      }else if(_motorControler->getReferenceSwitchData().getNegativeSwitchActive() &&
          std::abs(_motorControler->getActualPosition() -  _calibNegativeEndSwitchInSteps.load()) > 3 * _toleranceNegativeEndSwitch.load()){
        return CALIBRATION_LOST;
      }
    }
    return NO_ERROR;
  }

  bool StepperMotorWithReference::hasHWReferenceSwitches(){
    return true;
  }

  int StepperMotorWithReference::getPositiveEndReferenceInSteps(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _calibPositiveEndSwitchInSteps.load();
  }

  float StepperMotorWithReference::getPositiveEndReference(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_calibPositiveEndSwitchInSteps.load());
  }

  int StepperMotorWithReference::getNegativeEndReferenceInSteps(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _calibNegativeEndSwitchInSteps.load();
  }

  float StepperMotorWithReference::getNegativeEndReference(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_calibNegativeEndSwitchInSteps.load());
  }

  float StepperMotorWithReference::getTolerancePositiveEndSwitch(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _tolerancePositiveEndSwitch.load();
  }

  float StepperMotorWithReference::getToleranceNegativeEndSwitch(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _toleranceNegativeEndSwitch.load();
  }

  bool StepperMotorWithReference::isPositiveReferenceActive(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _motorControler->getReferenceSwitchData().getPositiveSwitchActive();
  }

  bool StepperMotorWithReference::isNegativeReferenceActive(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _motorControler->getReferenceSwitchData().getNegativeSwitchActive();
  }

  bool StepperMotorWithReference::isPositiveEndSwitchEnabled(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _positiveEndSwitchEnabled.load();
  }

  bool StepperMotorWithReference::isNegativeEndSwitchEnabled(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _negativeEndSwitchEnabled.load();
  }

  StepperMotorCalibrationMode StepperMotorWithReference::getCalibrationMode(){
    return _calibrationMode.load();
  }

  void StepperMotorWithReference::loadEndSwitchCalibration(){
    boost::lock_guard<boost::mutex> guard(_mutex);

    if(_motorControler->getCalibrationTime() != 0){

      int calibPositiveEndSwitchInSteps = _motorControler->getPositiveReferenceSwitchCalibration();
      int calibNegativeEndSwitchInSteps = _motorControler->getNegativeReferenceSwitchCalibration();
      _calibPositiveEndSwitchInSteps.exchange(calibPositiveEndSwitchInSteps);
      _calibNegativeEndSwitchInSteps.exchange(calibNegativeEndSwitchInSteps);

      if(calibPositiveEndSwitchInSteps == std::numeric_limits<int>::max()
         && calibNegativeEndSwitchInSteps == std::numeric_limits<int>::min()){
        _calibrationMode.exchange(StepperMotorCalibrationMode::SIMPLE);
      }
      else{
        _calibrationMode.exchange(StepperMotorCalibrationMode::FULL);
      }
    }
  }
}

