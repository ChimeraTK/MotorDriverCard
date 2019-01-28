/*
 * StepperMotorWithReference.cc
 *
 *  Created on: Feb 17, 2017
 *      Author: vitimic
 */

#include "StepperMotorWithReference.h"

#include "StepperMotorUtil.h"
#include "StepperMotorWithReferenceStateMachine.h"
#include "MotorDriverCard.h"
#include "MotorDriverCardFactory.h"


using LockGuard = boost::lock_guard<boost::mutex>;


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

  bool StepperMotorWithReference::motorActive(){
    std::string stateName =  _stateMachine->getCurrentState()->getName();
    if (stateName == "moving" || stateName == "calibrating" || stateName == "calculatingTolerance"){
      return true;
    }else{
      return false;
    }
  }

  bool StepperMotorWithReference::limitsOK(int newPositionInSteps){
    if (newPositionInSteps >= _calibNegativeEndSwitchInSteps.load() && newPositionInSteps <= _calibPositiveEndSwitchInSteps.load()) {
      return BasicStepperMotor::limitsOK(newPositionInSteps);
    }
    else{
      return false;
    }
  }

  StepperMotorRet StepperMotorWithReference::checkNewPosition(int newPositionInSteps){

    if(_stateMachine->getCurrentState()->getName() == "calibrating"){
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
    }
    return BasicStepperMotor::checkNewPosition(newPositionInSteps);
  }

  StepperMotorRet StepperMotorWithReference::setActualPositionInSteps(int actualPositionInSteps){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
    }
    setActualPositionActions(actualPositionInSteps);

    // Overwrite end switch calibration and reset calibration mode
    _calibNegativeEndSwitchInSteps.exchange(std::numeric_limits<int>::min());
    _calibPositiveEndSwitchInSteps.exchange(std::numeric_limits<int>::max());

    return StepperMotorRet::SUCCESS;
  }

  StepperMotorRet StepperMotorWithReference::translateAxisInSteps(int translationInSteps){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
    }

    translateAxisActions(translationInSteps);

    // If motor has been calibrated, translate also end switch positions
    if(this->_calibrationMode.load() == StepperMotorCalibrationMode::FULL){
      if(checkIfOverflow(_calibPositiveEndSwitchInSteps.load(), translationInSteps) ||
         checkIfOverflow(_calibNegativeEndSwitchInSteps.load(), translationInSteps))
      {
        return StepperMotorRet::ERR_INVALID_PARAMETER;
      }
      else{
        _calibPositiveEndSwitchInSteps.fetch_add(translationInSteps);
        _calibNegativeEndSwitchInSteps.fetch_add(translationInSteps);
      }
    }
    return StepperMotorRet::SUCCESS;
  }

  StepperMotorRet StepperMotorWithReference::calibrate(){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
    }
    _stateMachine->setAndProcessUserEvent(StepperMotorWithReferenceStateMachine::calibEvent);
    return StepperMotorRet::SUCCESS;
  }

  StepperMotorRet StepperMotorWithReference::determineTolerance(){
    LockGuard guard(_mutex);
    if (motorActive()){
      return StepperMotorRet::ERR_SYSTEM_IN_ACTION;
    }
    _stateMachine->setAndProcessUserEvent(StepperMotorWithReferenceStateMachine::calcToleranceEvent);
    return StepperMotorRet::SUCCESS;
  }

  StepperMotorError StepperMotorWithReference::getError(){
    LockGuard guard(_mutex);
//    if (motorActive() &&
//        _motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
//        _motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
//      return StepperMotorError::BOTH_END_SWITCHES_ON;
//    }
//    if (!motorActive()){
//      if (_toleranceCalcFailed.load() || _calibrationFailed.load()){
//        return StepperMotorError::ACTION_ERROR;
//      }
//    }
//    if (_toleranceCalculated){
//      if (_motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
//          std::abs(_motorControler->getActualPosition() -  _calibPositiveEndSwitchInSteps.load()) > 3 * _tolerancePositiveEndSwitch.load()){
//        return StepperMotorError::CALIBRATION_ERROR;
//      }else if(_motorControler->getReferenceSwitchData().getNegativeSwitchActive() &&
//          std::abs(_motorControler->getActualPosition() -  _calibNegativeEndSwitchInSteps.load()) > 3 * _toleranceNegativeEndSwitch.load()){
//        return StepperMotorError::CALIBRATION_ERROR;
//      }
//    }
    return _errorMode.load();
  }

  bool StepperMotorWithReference::hasHWReferenceSwitches(){
    return true;
  }

  int StepperMotorWithReference::getPositiveEndReferenceInSteps(){
    LockGuard guard(_mutex);
    return _calibPositiveEndSwitchInSteps.load();
  }

  float StepperMotorWithReference::getPositiveEndReference(){
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_calibPositiveEndSwitchInSteps.load());
  }

  int StepperMotorWithReference::getNegativeEndReferenceInSteps(){
    LockGuard guard(_mutex);
    return _calibNegativeEndSwitchInSteps.load();
  }

  float StepperMotorWithReference::getNegativeEndReference(){
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_calibNegativeEndSwitchInSteps.load());
  }

  float StepperMotorWithReference::getTolerancePositiveEndSwitch(){
    LockGuard guard(_mutex);
    return _tolerancePositiveEndSwitch.load();
  }

  float StepperMotorWithReference::getToleranceNegativeEndSwitch(){
    LockGuard guard(_mutex);
    return _toleranceNegativeEndSwitch.load();
  }

  bool StepperMotorWithReference::isPositiveReferenceActive(){
    LockGuard guard(_mutex);
    return isEndSwitchActive(Sign::POSITIVE);;
  }

  bool StepperMotorWithReference::isNegativeReferenceActive(){
    LockGuard guard(_mutex);
    return isEndSwitchActive(Sign::NEGATIVE);
  }

  bool StepperMotorWithReference::isPositiveEndSwitchEnabled(){
    LockGuard guard(_mutex);
    return _positiveEndSwitchEnabled.load();
  }

  bool StepperMotorWithReference::isNegativeEndSwitchEnabled(){
    LockGuard guard(_mutex);
    return _negativeEndSwitchEnabled.load();
  }

  StepperMotorCalibrationMode StepperMotorWithReference::getCalibrationMode(){
    return _calibrationMode.load();
  }

  void StepperMotorWithReference::loadEndSwitchCalibration(){
    LockGuard guard(_mutex);

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

  bool StepperMotorWithReference::verifyMoveAction(){

    bool endSwitchActive = _motorControler->getReferenceSwitchData().getPositiveSwitchActive() ||
                                _motorControler->getReferenceSwitchData().getNegativeSwitchActive();
    bool positionMatch     = _motorControler->getTargetPosition() == _motorControler->getActualPosition();

    return endSwitchActive || positionMatch;
  }

  bool StepperMotorWithReference::isEndSwitchActive(Sign sign){
    bool posActive = _motorControler->getReferenceSwitchData().getPositiveSwitchActive() == 1U;
    bool negActive = _motorControler->getReferenceSwitchData().getNegativeSwitchActive() == 1U;

    if(posActive && negActive){
      _errorMode.exchange(StepperMotorError::BOTH_END_SWITCHES_ON);
      _stateMachine->setAndProcessUserEvent(StepperMotorStateMachine::errorEvent);
    }

    if (_toleranceCalculated){
      if (posActive){
        if(std::abs(_motorControler->getActualPosition() -  _calibPositiveEndSwitchInSteps.load()) > 3 * _tolerancePositiveEndSwitch.load()){
          _errorMode.exchange(StepperMotorError::CALIBRATION_ERROR);
        }
      }
      else if(negActive){
        if(std::abs(_motorControler->getActualPosition() -  _calibNegativeEndSwitchInSteps.load()) > 3 * _toleranceNegativeEndSwitch.load()){
          _errorMode.exchange(StepperMotorError::CALIBRATION_ERROR);
        }
      }
    }

    if (sign == Sign::POSITIVE){
      if (posActive){
        return true;
      }else{
        return false;
      }
    }else{
      if (negActive){
        return true;
      }else{
        return false;
      }
    }
  }
}

