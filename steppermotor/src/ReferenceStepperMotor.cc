// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ReferenceStepperMotor.h"

// NOLINTNEXTLINE(misc-include-cleaner) the calls through it need the complete type
#include "MotorController.h"
// NOLINTNEXTLINE(misc-include-cleaner) the calls through it need the complete type
#include "MotorDriverCard.h"
#include "ReferenceStateMachine.h"
#include "StepperMotorUtil.h"

#include <memory>

using LockGuard = std::scoped_lock<std::mutex>;

namespace ChimeraTK::MotorDriver {

  ReferenceStepperMotor::ReferenceStepperMotor(
      const StepperMotorParameters& parameters, std::shared_ptr<utility::StateMachine> stateMachine)
  : BasicStepperMotor(parameters) {
    _stateMachine = std::move(stateMachine);
    initStateMachine();
    negativeEndSwitchEnabled = _motorController->getReferenceSwitchData().getNegativeSwitchEnabled();
    positiveEndSwitchEnabled = _motorController->getReferenceSwitchData().getPositiveSwitchEnabled();
    loadEndSwitchCalibration();
  }

  ReferenceStepperMotor::~ReferenceStepperMotor() = default;

  bool ReferenceStepperMotor::motorActive() {
    std::string stateName = _stateMachine->getCurrentState()->getName();
    return (stateName == "moving" || stateName == "calibrating" || stateName == "calculatingTolerance");
  }

  bool ReferenceStepperMotor::limitsOK(int newPositionInSteps) {
    if(newPositionInSteps >= calibNegativeEndSwitchInSteps.load() &&
        newPositionInSteps <= calibPositiveEndSwitchInSteps.load()) {
      return BasicStepperMotor::limitsOK(newPositionInSteps);
    }

    return false;
  }

  ExitStatus ReferenceStepperMotor::checkNewPosition(int newPositionInSteps) {
    if(_stateMachine->getCurrentState()->getName() == "calibrating") {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    return BasicStepperMotor::checkNewPosition(newPositionInSteps);
  }

  ExitStatus ReferenceStepperMotor::setActualPositionInSteps(int actualPositionInSteps) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    setActualPositionActions(actualPositionInSteps);

    // Overwrite end switch calibration and reset calibration mode
    calibNegativeEndSwitchInSteps.exchange(std::numeric_limits<int>::min());
    calibPositiveEndSwitchInSteps.exchange(std::numeric_limits<int>::max());

    return ExitStatus::SUCCESS;
  }

  ExitStatus ReferenceStepperMotor::translateAxisInSteps(int translationInSteps) {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }

    translateAxisActions(translationInSteps);

    // If motor has been calibrated, translate also end switch positions
    if(this->_calibrationMode.load() == CalibrationMode::FULL) {
      if(utility::checkIfOverflow(calibPositiveEndSwitchInSteps.load(), translationInSteps) ||
          utility::checkIfOverflow(calibNegativeEndSwitchInSteps.load(), translationInSteps)) {
        return ExitStatus::ERR_INVALID_PARAMETER;
      }

      calibPositiveEndSwitchInSteps.fetch_add(translationInSteps);
      calibNegativeEndSwitchInSteps.fetch_add(translationInSteps);
    }
    return ExitStatus::SUCCESS;
  }

  ExitStatus ReferenceStepperMotor::calibrate() {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    _stateMachine->setAndProcessUserEvent(ReferenceStateMachine::calibEvent);
    return ExitStatus::SUCCESS;
  }

  ExitStatus ReferenceStepperMotor::determineTolerance() {
    LockGuard guard(_mutex);
    if(motorActive()) {
      return ExitStatus::ERR_SYSTEM_IN_ACTION;
    }
    _stateMachine->setAndProcessUserEvent(ReferenceStateMachine::calcToleranceEvent);
    return ExitStatus::SUCCESS;
  }

  Error ReferenceStepperMotor::getError() {
    LockGuard guard(_mutex);
    //    if (motorActive() &&
    //        _motorController->getReferenceSwitchData().getPositiveSwitchActive()
    //        &&
    //        _motorController->getReferenceSwitchData().getNegativeSwitchActive()){
    //      return Error::BOTH_END_SWITCHES_ON;
    //    }
    //    if (!motorActive()){
    //      if (toleranceCalcFailed.load() || calibrationFailed.load()){
    //        return Error::ACTION_ERROR;
    //      }
    //    }
    //    if (toleranceCalculated){
    //      if
    //      (_motorController->getReferenceSwitchData().getPositiveSwitchActive()
    //      &&
    //          std::abs(_motorController->getActualPosition() -
    //          calibPositiveEndSwitchInSteps.load()) > 3 *
    //          tolerancePositiveEndSwitch.load()){
    //        return Error::CALIBRATION_ERROR;
    //      }else
    //      if(_motorController->getReferenceSwitchData().getNegativeSwitchActive()
    //      &&
    //          std::abs(_motorController->getActualPosition() -
    //          calibNegativeEndSwitchInSteps.load()) > 3 *
    //          toleranceNegativeEndSwitch.load()){
    //        return Error::CALIBRATION_ERROR;
    //      }
    //    }
    return _errorMode.load();
  }

  bool ReferenceStepperMotor::hasHWReferenceSwitches() {
    return true;
  }

  int ReferenceStepperMotor::getPositiveEndReferenceInSteps() {
    LockGuard guard(_mutex);
    return calibPositiveEndSwitchInSteps.load();
  }

  float ReferenceStepperMotor::getPositiveEndReference() {
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(calibPositiveEndSwitchInSteps.load());
  }

  int ReferenceStepperMotor::getNegativeEndReferenceInSteps() {
    LockGuard guard(_mutex);
    return calibNegativeEndSwitchInSteps.load();
  }

  float ReferenceStepperMotor::getNegativeEndReference() {
    LockGuard guard(_mutex);
    return _stepperMotorUnitsConverter->stepsToUnits(calibNegativeEndSwitchInSteps.load());
  }

  float ReferenceStepperMotor::getTolerancePositiveEndSwitch() {
    LockGuard guard(_mutex);
    return tolerancePositiveEndSwitch.load();
  }

  float ReferenceStepperMotor::getToleranceNegativeEndSwitch() {
    LockGuard guard(_mutex);
    return toleranceNegativeEndSwitch.load();
  }

  bool ReferenceStepperMotor::isPositiveReferenceActive() {
    LockGuard guard(_mutex);
    return isEndSwitchActive(Sign::POSITIVE);
    ;
  }

  bool ReferenceStepperMotor::isNegativeReferenceActive() {
    LockGuard guard(_mutex);
    return isEndSwitchActive(Sign::NEGATIVE);
  }

  bool ReferenceStepperMotor::isPositiveEndSwitchEnabled() {
    LockGuard guard(_mutex);
    return positiveEndSwitchEnabled.load();
  }

  bool ReferenceStepperMotor::isNegativeEndSwitchEnabled() {
    LockGuard guard(_mutex);
    return negativeEndSwitchEnabled.load();
  }

  CalibrationMode ReferenceStepperMotor::getCalibrationMode() {
    return _calibrationMode.load();
  }

  void ReferenceStepperMotor::loadEndSwitchCalibration() {
    LockGuard guard(_mutex);

    if(_motorController->getCalibrationTime() != 0) {
      int positiveCalibration = _motorController->getPositiveReferenceSwitchCalibration();
      int negativeCalibration = _motorController->getNegativeReferenceSwitchCalibration();
      calibPositiveEndSwitchInSteps.exchange(positiveCalibration);
      calibNegativeEndSwitchInSteps.exchange(negativeCalibration);

      if(calibPositiveEndSwitchInSteps == std::numeric_limits<int>::max() &&
          calibNegativeEndSwitchInSteps == std::numeric_limits<int>::min()) {
        _calibrationMode.exchange(CalibrationMode::SIMPLE);
      }
      else {
        _calibrationMode.exchange(CalibrationMode::FULL);
      }
    }
  }

  bool ReferenceStepperMotor::verifyMoveAction() {
    bool endSwitchActive = _motorController->getReferenceSwitchData().getPositiveSwitchActive() ||
        _motorController->getReferenceSwitchData().getNegativeSwitchActive();
    bool positionMatch = _motorController->getTargetPosition() == _motorController->getActualPosition();

    return endSwitchActive || positionMatch;
  }

  bool ReferenceStepperMotor::isEndSwitchActive(Sign sign) {
    bool posActive = positiveSwitchActive();
    bool negActive = negativeSwitchActive();

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

    if(toleranceCalculated) {
      if(posActive) {
        if(static_cast<float>(std::abs(_motorController->getActualPosition() - calibPositiveEndSwitchInSteps.load())) >
            3 * tolerancePositiveEndSwitch.load()) {
          _errorMode.exchange(Error::CALIBRATION_ERROR);
        }
      }
      else if(negActive) {
        if(static_cast<float>(std::abs(_motorController->getActualPosition() - calibNegativeEndSwitchInSteps.load())) >
            3 * toleranceNegativeEndSwitch.load()) {
          _errorMode.exchange(Error::CALIBRATION_ERROR);
        }
      }
    }

    if(sign == Sign::POSITIVE) {
      return posActive;
    }

    return negActive;
  }

  bool ReferenceStepperMotor::positiveSwitchActive() const {
    return _motorController->getReferenceSwitchData().getPositiveSwitchActive() == 1U;
  }

  bool ReferenceStepperMotor::negativeSwitchActive() const {
    return _motorController->getReferenceSwitchData().getNegativeSwitchActive() == 1U;
  }

} // namespace ChimeraTK::MotorDriver
