/*
 * StepperMotorWithReference.cc
 *
 *  Created on: Feb 17, 2017
 *      Author: vitimic
 */

#include "StepperMotorWithReference.h"
#include "StepperMotorWithReferenceStateMachine.h"

namespace ChimeraTK{
  StepperMotorWithReference::StepperMotorWithReference(std::string const & motorDriverCardDeviceName,
                                                       std::string const & moduleName,
                                                       unsigned int motorDriverId,
                                                       std::string motorDriverCardConfigFileName,
                                                       std::shared_ptr<StepperMotorUnitsConverter> motorUnitsConverter,
                                                       double encoderUnitsToStepsRatio):
                                                           StepperMotor(),
                                                           _positiveEndSwitchEnabled(false),
                                                           _negativeEndSwitchEnabled(false),
                                                           _calibrationFailed(false),
                                                           _toleranceCalcFailed(false),
                                                           _toleranceCalculated(false),
                                                           _calibNegativeEndSwitchInSteps(-std::numeric_limits<int>::max()),
                                                           _calibPositiveEndSwitchInSteps(std::numeric_limits<int>::max()),
                                                           _calibrationMode(StepperMotorCalibrationMode::NONE),
                                                           _tolerancePositiveEndSwitch(0),
                                                           _toleranceNegativeEndSwitch(0),
                                                           _index(0){
    _motorDriverCardDeviceName = motorDriverCardDeviceName;
    _motorDriverId = motorDriverId;
    _motorDriverCard = mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(motorDriverCardDeviceName, moduleName, motorDriverCardConfigFileName);
    _motorControler = _motorDriverCard->getMotorControler(_motorDriverId);
    _stepperMotorUnitsConverter = motorUnitsConverter,
    _encoderUnitToStepsRatio = encoderUnitsToStepsRatio,
    createStateMachine();
    _targetPositionInSteps = _motorControler->getTargetPosition();
    _negativeEndSwitchEnabled = _motorControler->getReferenceSwitchData().getNegativeSwitchEnabled();
    _positiveEndSwitchEnabled = _motorControler->getReferenceSwitchData().getPositiveSwitchEnabled();
  }

  StepperMotorWithReference::~StepperMotorWithReference(){}

  void StepperMotorWithReference::createStateMachine(){
    _stateMachine.reset(new StepperMotorWithReferenceStateMachine(*this));
    _stateMachineThread = std::thread(&StepperMotorWithReference::stateMachineThreadFunction, this);
  }

  bool StepperMotorWithReference::stateMachineInIdleAndNoEvent(){
    if (_stateMachine->getUserEvent() == StateMachine::noEvent && _stateMachine->getCurrentState()->getName() == "StepperMotorStateMachine"){
      if ((dynamic_cast<StepperMotorStateMachine*>(_stateMachine->getCurrentState()))->getCurrentState()->getName() == "idleState"){
        return true;
      }else{
        return false;
      }
    }else{
      return false;
    }
  }

  bool StepperMotorWithReference::limitsOK(int newPositionInSteps){
    if (newPositionInSteps >= _calibNegativeEndSwitchInSteps && newPositionInSteps <= _calibPositiveEndSwitchInSteps) {
      return StepperMotor::limitsOK(newPositionInSteps);
    }else{
      return false;
    }
  }

  void StepperMotorWithReference::setActualPositionInSteps(int actualPositionInSteps){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    resetPositionMotorController(actualPositionInSteps);

    // Overwrite end switch calibration and reset calibration mode
    _calibNegativeEndSwitchInSteps = -std::numeric_limits<int>::max();
    _calibPositiveEndSwitchInSteps = std::numeric_limits<int>::max();
    _calibrationMode = StepperMotorCalibrationMode::SIMPLE;
  }

  void StepperMotorWithReference::setActualPosition(float actualPosition){
    setActualPositionInSteps(_stepperMotorUnitsConverter->unitsToSteps(actualPosition));
  }

  void StepperMotorWithReference::resetMotorControlerAndCheckOverFlowSoftLimits(int translationInSteps){

    StepperMotor::resetMotorControlerAndCheckOverFlowSoftLimits(translationInSteps);

    if(_motorControler->getCalibrationTime() != 0){
      if(checkIfOverflow(_calibPositiveEndSwitchInSteps, translationInSteps) ||
         checkIfOverflow(_calibNegativeEndSwitchInSteps, translationInSteps))
      {
        throw MotorDriverException("overflow for positive and/or negative reference", MotorDriverException::NOT_IMPLEMENTED);
      }else{
        _calibPositiveEndSwitchInSteps = _calibPositiveEndSwitchInSteps + translationInSteps;
        _calibNegativeEndSwitchInSteps = _calibNegativeEndSwitchInSteps + translationInSteps;
      }
    }
  }

  void StepperMotorWithReference::translateAxisInSteps(int translationInSteps){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    resetMotorControlerAndCheckOverFlowSoftLimits(translationInSteps);
  }

  void StepperMotorWithReference::translateAxis(float translationInUnits){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    resetMotorControlerAndCheckOverFlowSoftLimits(_stepperMotorUnitsConverter->unitsToSteps(translationInUnits));
  }

  void StepperMotorWithReference::calibrate(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    _stateMachine->setUserEvent(StepperMotorWithReferenceStateMachine::calibEvent);
  }

  void StepperMotorWithReference::determineTolerance(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    _stateMachine->setUserEvent(StepperMotorWithReferenceStateMachine::calcToleranceEvent);
  }

  StepperMotorError StepperMotorWithReference::getError(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (_motorControler->getReferenceSwitchData().getPositiveSwitchActive() && _motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
      return BOTH_END_SWITCHES_ON;
    }
    if (stateMachineInIdleAndNoEvent()){
      if (_toleranceCalcFailed || _calibrationFailed){
        return ACTION_ERROR;
      }else if ((_motorControler->getTargetPosition() != _motorControler->getActualPosition()) &&
                !_motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
                !_motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
        return ACTION_ERROR;
      }
    }
    if (_toleranceCalculated){
      if (_motorControler->getReferenceSwitchData().getPositiveSwitchActive() &&
          std::abs(_motorControler->getActualPosition() -  _calibPositiveEndSwitchInSteps) > 3 * _tolerancePositiveEndSwitch){
        return CALIBRATION_LOST;
      }else if(_motorControler->getReferenceSwitchData().getNegativeSwitchActive() &&
          std::abs(_motorControler->getActualPosition() -  _calibNegativeEndSwitchInSteps) > 3 * _toleranceNegativeEndSwitch){
        return CALIBRATION_LOST;
      }
    }
    return NO_ERROR;
  }

  int StepperMotorWithReference::getPositiveEndReferenceInSteps(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _calibPositiveEndSwitchInSteps;
  }

  float StepperMotorWithReference::getPositiveEndReference(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_calibPositiveEndSwitchInSteps);
  }

  int StepperMotorWithReference::getNegativeEndReferenceInSteps(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _calibNegativeEndSwitchInSteps;
  }

  float StepperMotorWithReference::getNegativeEndReference(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_calibNegativeEndSwitchInSteps);
  }

  float StepperMotorWithReference::getTolerancePositiveEndSwitch(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _tolerancePositiveEndSwitch;
  }

  float StepperMotorWithReference::getToleranceNegativeEndSwitch(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _toleranceNegativeEndSwitch;
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
    return _positiveEndSwitchEnabled;
  }

  bool StepperMotorWithReference::isNegativeEndSwitchEnabled(){
    boost::lock_guard<boost::mutex> guard(_mutex);
    return _negativeEndSwitchEnabled;
  }

  StepperMotorCalibrationMode StepperMotorWithReference::getCalibrationMode(){
    return std::atomic_load(&_calibrationMode);
  }
}

