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
                                                       std::string motorDriverCardConfigFileName):
                                                		    StepperMotor(),
                                                		    _positiveEndSwitchEnabled(false),
                                                		    _negativeEndSwitchEnabled(false),
                                                		    _calibrationFailed(false),
                                                		    _toleranceCalcFailed(false),
                                                		    _toleranceCalculated(false),
                                                		    _calibNegativeEndSwitchInSteps(-std::numeric_limits<int>::max()),
                                                		    _calibPositiveEndSwitchInSteps(std::numeric_limits<int>::max()),
                                                		    _tolerancePositiveEndSwitch(0),
                                                		    _toleranceNegativeEndSwitch(0){
    _motorDriverCardDeviceName = motorDriverCardDeviceName;
    _motorDriverId = motorDriverId;
    _motorDriverCard = mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(motorDriverCardDeviceName, moduleName, motorDriverCardConfigFileName);
    _motorControler = _motorDriverCard->getMotorControler(_motorDriverId);
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
  }

  void StepperMotorWithReference::translateAxisInSteps(int translationInSteps){
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (!stateMachineInIdleAndNoEvent()){
      throw MotorDriverException("state machine not in idle", MotorDriverException::NOT_IMPLEMENTED);
    }
    int actualPosition = _motorControler->getActualPosition();
    resetPositionMotorController(actualPosition+translationInSteps);
    if (checkIfOverflow(_maxPositionLimitInSteps, translationInSteps)){
      _maxPositionLimitInSteps = std::numeric_limits<int>::max();
    }else{
      _maxPositionLimitInSteps = _maxPositionLimitInSteps + translationInSteps;
    }
    if (checkIfOverflow(_minPositionLimitInSteps, translationInSteps)){
      _minPositionLimitInSteps = std::numeric_limits<int>::min();
    }else{
      _minPositionLimitInSteps = _minPositionLimitInSteps + translationInSteps;
    }
    if(retrieveCalibrationTime() != 0){
      if (checkIfOverflow(_calibPositiveEndSwitchInSteps, translationInSteps) ||
	  checkIfOverflow(_calibNegativeEndSwitchInSteps, translationInSteps)){
	throw MotorDriverException("overflow for positive and/or negative reference", MotorDriverException::NOT_IMPLEMENTED);
      }else{
	_calibPositiveEndSwitchInSteps = _calibPositiveEndSwitchInSteps + translationInSteps;
	_calibNegativeEndSwitchInSteps = _calibNegativeEndSwitchInSteps + translationInSteps;
      }
    }
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
    if (stateMachineInIdleAndNoEvent()){
      if (_toleranceCalcFailed || _calibrationFailed){
	return ACTION_ERROR;
      }else if ((_motorControler->getTargetPosition() != _motorControler->getActualPosition()) &&
	  !isPositiveReferenceActive() &&
	  !isNegativeReferenceActive()){
	return ACTION_ERROR;
      }
    }
    if (_toleranceCalculated){
      if (isPositiveReferenceActive() && fabs(_motorControler->getActualPosition() -  _calibPositiveEndSwitchInSteps) > 3 * _tolerancePositiveEndSwitch){
	return CALIBRATION_LOST;
      }else if(isNegativeReferenceActive() && fabs(_motorControler->getActualPosition() -  _calibNegativeEndSwitchInSteps) > 3 * _toleranceNegativeEndSwitch){
	return CALIBRATION_LOST;
      }
    }
    return NO_ERROR;
  }

  int StepperMotorWithReference::getPositiveEndReferenceInSteps(){
    return _calibPositiveEndSwitchInSteps;
  }

  float StepperMotorWithReference::getPositiveEndReference(){
    return recalculateStepsInUnits(_calibPositiveEndSwitchInSteps);
  }

  int StepperMotorWithReference::getNegativeEndReferenceInSteps(){
    return _calibNegativeEndSwitchInSteps;
  }

  float StepperMotorWithReference::getNegativeEndReference(){
    return recalculateStepsInUnits(_calibNegativeEndSwitchInSteps);
  }

  float StepperMotorWithReference::getTolerancePositiveEndSwitch(){
    return _tolerancePositiveEndSwitch;
  }

  float StepperMotorWithReference::getToleranceNegativeEndSwitch(){
    return _toleranceNegativeEndSwitch;
  }

  bool StepperMotorWithReference::isPositiveReferenceActive(){
    return _motorControler->getReferenceSwitchData().getPositiveSwitchActive();
  }

  bool StepperMotorWithReference::isNegativeReferenceActive(){
    return _motorControler->getReferenceSwitchData().getNegativeSwitchActive();
  }

  bool StepperMotorWithReference::isPositiveEndSwitchEnabled(){
    return _positiveEndSwitchEnabled;
  }

  bool StepperMotorWithReference::isNegativeEndSwitchEnabled(){
    return _negativeEndSwitchEnabled;
  }
}

