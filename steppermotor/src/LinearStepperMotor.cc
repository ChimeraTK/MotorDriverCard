/*
 * StepperMotorWithReference.cc
 *
 *  Created on: Feb 17, 2017
 *      Author: vitimic
 */

#include "LinearStepperMotor.h"

#include "MotorControler.h"
#include "MotorDriverCard.h"
#include "MotorDriverCardFactory.h"
#include "StepperMotorUtil.h"

using LockGuard = boost::lock_guard<boost::mutex>;

namespace ChimeraTK { namespace MotorDriver {

  LinearStepperMotor::LinearStepperMotor(const StepperMotorParameters& parameters)
  : BasicStepperMotor(), _positiveEndSwitchEnabled(false), _negativeEndSwitchEnabled(false), _calibrationFailed(false),
    _toleranceCalcFailed(false), _toleranceCalculated(false),
    _calibNegativeEndSwitchInSteps(-std::numeric_limits<int>::max()),
    _calibPositiveEndSwitchInSteps(std::numeric_limits<int>::max()), _tolerancePositiveEndSwitch(0),
    _toleranceNegativeEndSwitch(0) {
    _motorDriverCard = mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(
        parameters.deviceName, parameters.moduleName, parameters.configFileName);
    _motorControler = _motorDriverCard->getMotorControler(parameters.driverId);
    _stepperMotorUnitsConverter = parameters.motorUnitsConverter;
    _encoderUnitsConverter = parameters.encoderUnitsConverter;
    _stateMachine.reset(new StateMachine(*this));
    _targetPositionInSteps = _motorControler->getTargetPosition();
    _negativeEndSwitchEnabled = _motorControler->getReferenceSwitchData().getNegativeSwitchEnabled();
    _positiveEndSwitchEnabled = _motorControler->getReferenceSwitchData().getPositiveSwitchEnabled();
    initStateMachine();
    loadEndSwitchCalibration();
  }

  LinearStepperMotor::~LinearStepperMotor() = default;

  bool LinearStepperMotor::motorActive() {
    std::string stateName = _stateMachine->getCurrentState()->getName();
    if(stateName == "moving" || stateName == "calibrating" || stateName == "calculatingTolerance") {
      return true;
    }
    else {
      return false;
    }
  }

  bool LinearStepperMotor::limitsOK(int newPositionInSteps) {
    if(newPositionInSteps >= _calibNegativeEndSwitchInSteps.load() &&
        newPositionInSteps <= _calibPositiveEndSwitchInSteps.load()) {
      return BasicStepperMotor::limitsOK(newPositionInSteps);
    }
    else {
      return false;
    }
  }

  ExitStatus LinearStepperMotor::checkNewPosition(int newPositionInSteps) {
    if(_stateMachine->getCurrentState()->getName() == "calibrating") {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    return BasicStepperMotor::checkNewPosition(newPositionInSteps);
  }

  ExitStatus LinearStepperMotor::setActualPositionInSteps(int actualPositionInSteps) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    setActualPositionActions(actualPositionInSteps);

    // Overwrite end switch calibration and reset calibration mode
    _calibNegativeEndSwitchInSteps.exchange(std::numeric_limits<int>::min());
    _calibPositiveEndSwitchInSteps.exchange(std::numeric_limits<int>::max());

    return ExitStatus::SUCCESS;
  }

  ExitStatus LinearStepperMotor::translateAxisInSteps(int translationInSteps) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }

    translateAxisActions(translationInSteps);

    // If motor has been calibrated, translate also end switch positions
    if(this->_calibrationMode.load() == CalibrationMode::FULL) {
      if(utility::checkIfOverflow(_calibPositiveEndSwitchInSteps.load(), translationInSteps) ||
          utility::checkIfOverflow(_calibNegativeEndSwitchInSteps.load(), translationInSteps)) {
        return ExitStatus::ERR_INVALID_PARAMETER;
      }

      _calibPositiveEndSwitchInSteps.fetch_add(translationInSteps);
      _calibNegativeEndSwitchInSteps.fetch_add(translationInSteps);
    }
    return ExitStatus::SUCCESS;
  }

  ExitStatus LinearStepperMotor::calibrate() {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    _stateMachine->setAndProcessUserEvent(StateMachine::calibEvent);
    return ExitStatus::SUCCESS;
  }

  ExitStatus LinearStepperMotor::determineTolerance() {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    _stateMachine->setAndProcessUserEvent(StateMachine::calcToleranceEvent);
    return ExitStatus::SUCCESS;
  }

  Error LinearStepperMotor::getError() {
    LockGuard guard(_mutex);
    //    if (motorActive() &&
    //        _motorControler->getReferenceSwitchData().getPositiveSwitchActive()
    //        &&
    //        _motorControler->getReferenceSwitchData().getNegativeSwitchActive()){
    //      return Error::BOTH_END_SWITCHES_ON;
    //    }
    //    if (!motorActive()){
    //      if (_toleranceCalcFailed.load() || _calibrationFailed.load()){
    //        return Error::ACTION_ERROR;
    //      }
    //    }
    //    if (_toleranceCalculated){
    //      if
    //      (_motorControler->getReferenceSwitchData().getPositiveSwitchActive()
    //      &&
    //          std::abs(_motorControler->getActualPosition() -
    //          _calibPositiveEndSwitchInSteps.load()) > 3 *
    //          _tolerancePositiveEndSwitch.load()){
    //        return Error::CALIBRATION_ERROR;
    //      }else
    //      if(_motorControler->getReferenceSwitchData().getNegativeSwitchActive()
    //      &&
    //          std::abs(_motorControler->getActualPosition() -
    //          _calibNegativeEndSwitchInSteps.load()) > 3 *
    //          _toleranceNegativeEndSwitch.load()){
    //        return Error::CALIBRATION_ERROR;
    //      }
    //    }
    return _errorMode.load();
  }

  bool LinearStepperMotor::hasHWReferenceSwitches() {
    return true;
  }

  int LinearStepperMotor::getPositiveEndReferenceInSteps() {
    LockGuard guard(_mutex);
    return _calibPositiveEndSwitchInSteps.load();
  }

  float LinearStepperMotor::getPositiveEndReference() {
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_calibPositiveEndSwitchInSteps.load());
  }

  int LinearStepperMotor::getNegativeEndReferenceInSteps() {
    LockGuard guard(_mutex);
    return _calibNegativeEndSwitchInSteps.load();
  }

  float LinearStepperMotor::getNegativeEndReference() {
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(_calibNegativeEndSwitchInSteps.load());
  }

  float LinearStepperMotor::getTolerancePositiveEndSwitch() {
    LockGuard guard(_mutex);
    return _tolerancePositiveEndSwitch.load();
  }

  float LinearStepperMotor::getToleranceNegativeEndSwitch() {
    LockGuard guard(_mutex);
    return _toleranceNegativeEndSwitch.load();
  }

  bool LinearStepperMotor::isPositiveReferenceActive() {
    LockGuard guard(_mutex);
    return isEndSwitchActive(Sign::POSITIVE);
    ;
  }

  bool LinearStepperMotor::isNegativeReferenceActive() {
    LockGuard guard(_mutex);
    return isEndSwitchActive(Sign::NEGATIVE);
  }

  bool LinearStepperMotor::isPositiveEndSwitchEnabled() {
    LockGuard guard(_mutex);
    return _positiveEndSwitchEnabled.load();
  }

  bool LinearStepperMotor::isNegativeEndSwitchEnabled() {
    LockGuard guard(_mutex);
    return _negativeEndSwitchEnabled.load();
  }

  CalibrationMode LinearStepperMotor::getCalibrationMode() {
    return _calibrationMode.load();
  }

  void LinearStepperMotor::loadEndSwitchCalibration() {
    LockGuard guard(_mutex);

    if(_motorControler->getCalibrationTime() != 0) {
      int calibPositiveEndSwitchInSteps = _motorControler->getPositiveReferenceSwitchCalibration();
      int calibNegativeEndSwitchInSteps = _motorControler->getNegativeReferenceSwitchCalibration();
      _calibPositiveEndSwitchInSteps.exchange(calibPositiveEndSwitchInSteps);
      _calibNegativeEndSwitchInSteps.exchange(calibNegativeEndSwitchInSteps);

      if(calibPositiveEndSwitchInSteps == std::numeric_limits<int>::max() &&
          calibNegativeEndSwitchInSteps == std::numeric_limits<int>::min()) {
        _calibrationMode.exchange(CalibrationMode::SIMPLE);
      }
      else {
        _calibrationMode.exchange(CalibrationMode::FULL);
      }
    }
  }

  bool LinearStepperMotor::verifyMoveAction() {
    bool endSwitchActive = _motorControler->getReferenceSwitchData().getPositiveSwitchActive() ||
        _motorControler->getReferenceSwitchData().getNegativeSwitchActive();
    bool positionMatch = _motorControler->getTargetPosition() == _motorControler->getActualPosition();

    return endSwitchActive || positionMatch;
  }

  bool LinearStepperMotor::isEndSwitchActive(Sign sign) {
    bool posActive = _motorControler->getReferenceSwitchData().getPositiveSwitchActive() == 1U;
    bool negActive = _motorControler->getReferenceSwitchData().getNegativeSwitchActive() == 1U;

    if(posActive && negActive) {
      _errorMode.exchange(Error::BOTH_END_SWITCHES_ON);
      _stateMachine->setAndProcessUserEvent(StateMachine::errorEvent);
    }
    else {
      // The error mode was BOTH_END_SWITCHES_ON, reset the error mode. Otherwise this will stick unless the application
      // is re-started.
      auto expected = Error::BOTH_END_SWITCHES_ON;
      auto replace = Error::NO_ERROR;
      _errorMode.compare_exchange_strong(expected, replace);
    }

    if(_toleranceCalculated) {
      if(posActive) {
        if(std::abs(_motorControler->getActualPosition() - _calibPositiveEndSwitchInSteps.load()) >
            3 * _tolerancePositiveEndSwitch.load()) {
          _errorMode.exchange(Error::CALIBRATION_ERROR);
        }
      }
      else if(negActive) {
        if(std::abs(_motorControler->getActualPosition() - _calibNegativeEndSwitchInSteps.load()) >
            3 * _toleranceNegativeEndSwitch.load()) {
          _errorMode.exchange(Error::CALIBRATION_ERROR);
        }
      }
    }

    if(sign == Sign::POSITIVE) {
      return posActive;
    }

    return negActive;
  }
}} // namespace ChimeraTK::MotorDriver
