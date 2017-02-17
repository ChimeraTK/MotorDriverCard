/*
 * StepperMotorWithReferenceStateMachine.cc
 *
 *  Created on: Feb 17, 2017
 *      Author: vitimic
 */

#include "StepperMotorWithReferenceStateMachine.h"

namespace ChimeraTK{
  void StepperMotorWithReferenceStateMachine::calibrationThreadFunction(){
    _stepperMotorWithReference._calibrationFailed = false;
    _stopAction = false;
    _moveInterrupted = false;
    if(!(_stepperMotorWithReference._positiveEndSwitchEnabled && _stepperMotorWithReference._negativeEndSwitchEnabled)){
      _stepperMotorWithReference._calibrated = false;
      _stepperMotorWithReference._calibrationFailed = true;
      return;
    }
    findEndSwitch(POSITIVE);
    _stepperMotorWithReference._calibPositiveEndSwitchInSteps = _stepperMotorWithReference._motorControler->getActualPosition();
    findEndSwitch(NEGATIVE);
    _stepperMotorWithReference._calibNegativeEndSwitchInSteps = _stepperMotorWithReference._motorControler->getActualPosition();

    if (_moveInterrupted || _stopAction){
      _stepperMotorWithReference._calibrated = false;
      _stepperMotorWithReference._calibrationFailed = true;
      return;
    }else{
      _stepperMotorWithReference._calibPositiveEndSwitchInSteps = _stepperMotorWithReference._calibPositiveEndSwitchInSteps -
	  _stepperMotorWithReference._calibNegativeEndSwitchInSteps;
      _stepperMotorWithReference._calibNegativeEndSwitchInSteps = 0;
      _stepperMotorWithReference._calibrated = true;
      _stepperMotorWithReference.setActualPositionInSteps(0);
      return;
    }
  }

  void StepperMotorWithReferenceStateMachine::findEndSwitch(Sign sign){
    while (!isEndSwitchActive(sign)){
      if (_stopAction || _moveInterrupted){
	return;
      }else if (_stepperMotorWithReference._motorControler->getTargetPosition() !=
	  _stepperMotorWithReference._motorControler->getActualPosition()){
	_moveInterrupted = true;
	return;
      }
      moveToEndSwitch(sign);
    }
    return;
  }

  void StepperMotorWithReferenceStateMachine::moveToEndSwitch(Sign sign){
    _stepperMotorWithReference._motorControler->setTargetPosition(_stepperMotorWithReference._motorControler->getActualPosition() + sign*500000);
    while (_stepperMotorWithReference._motorControler->isMotorMoving()){}
  }

  bool StepperMotorWithReferenceStateMachine::isEndSwitchActive(Sign sign){
    if (sign == POSITIVE){
      if (_stepperMotorWithReference.isPositiveSwitchActive()){
	return true;
      }else{
	return false;
      }
    }else{
      if (_stepperMotorWithReference.isNegativeSwitchActive()){
	return true;
      }else{
	return false;
      }
    }
  }

  void StepperMotorWithReferenceStateMachine::toleranceCalcThreadFunction(){
    _stepperMotorWithReference._toleranceCalcFailed = false;
    _stopAction = false;
    _moveInterrupted = false;

    if (_stepperMotorWithReference._calibrated == false){
      _stepperMotorWithReference._toleranceCalculated = false;
      _stepperMotorWithReference._toleranceCalcFailed = true;
    }

    _stepperMotorWithReference._tolerancePositiveEndSwitch = getToleranceEndSwitch(POSITIVE);
    _stepperMotorWithReference._toleranceNegativeEndSwitch = getToleranceEndSwitch(NEGATIVE);

    if (_moveInterrupted || _stopAction){
      _stepperMotorWithReference._toleranceCalculated = false;
      _stepperMotorWithReference._toleranceCalcFailed = true;
    }else{
      _stepperMotorWithReference._toleranceCalculated = true;
    }
  }

  int StepperMotorWithReferenceStateMachine::getPositionEndSwitch(Sign sign){
    if (sign == POSITIVE){
      return _stepperMotorWithReference._calibPositiveEndSwitchInSteps;
    }else{
      return _stepperMotorWithReference._calibNegativeEndSwitchInSteps;
    }
  }

  double StepperMotorWithReferenceStateMachine::getToleranceEndSwitch(Sign sign){
    double squareMean = 0;
    double meanSquare = 0;

    int endSwitchPosition = getPositionEndSwitch(sign);

    for (unsigned int i=0; i<10; i++){
      if (_stopAction || _moveInterrupted){
	break;
      }
      _stepperMotorWithReference._motorControler->setTargetPosition(endSwitchPosition - sign*10000);
      while (_stepperMotorWithReference._motorControler->isMotorMoving()){}
      if (_stepperMotorWithReference._motorControler->getTargetPosition() != _stepperMotorWithReference._motorControler->getActualPosition()){
	_moveInterrupted = true;
	break;
      }
      _stepperMotorWithReference._motorControler->setTargetPosition(endSwitchPosition + sign*10000);
      while (_stepperMotorWithReference._motorControler->isMotorMoving()){}
      if (!isEndSwitchActive(sign)){
	_moveInterrupted = true;
	break;
      }
      squareMean += _stepperMotorWithReference._motorControler->getActualPosition();
      meanSquare += _stepperMotorWithReference._motorControler->getActualPosition() * _stepperMotorWithReference._motorControler->getActualPosition();
    }

    squareMean = squareMean / 10;
    meanSquare = meanSquare / 10;

    return meanSquare - squareMean * squareMean;
  }


}

