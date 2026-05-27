// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RotaryStepperMotorStateMachine.h"

#include "MotorControler.h"
#include "RotaryStepperMotor.h"

namespace ChimeraTK::MotorDriver {

  RotaryStepperMotorStateMachine::RotaryStepperMotorStateMachine(RotaryStepperMotor& motor)
  : ReferenceStateMachine(motor) {}

  void RotaryStepperMotorStateMachine::performCalibration() {
    _motor._calibrationFailed.exchange(false);
    _stopAction.exchange(false);
    _moveInterrupted.exchange(false);
    try {
      findEndSwitch(Sign::POSITIVE); // "home direction search"
      int homePosition = _motor.getCurrentPositionInSteps();

      if(_moveInterrupted.load() || _stopAction.load()) {
        _motor._motorController->setCalibrationTime(0);
        _motor._calibrationFailed.exchange(true);
        _motor._errorMode.exchange(Error::CALIBRATION_ERROR);
        _motor._calibrationMode.exchange(CalibrationMode::NONE);
      }
      else {
        int calibPositiveEndSwitchInSteps = homePosition;
        int calibNegativeEndSwitchInSteps = homePosition;
        _motor._motorController->setCalibrationTime(time(nullptr));
        _motor._motorController->setPositiveReferenceSwitchCalibration(calibPositiveEndSwitchInSteps);
        _motor._motorController->setNegativeReferenceSwitchCalibration(calibNegativeEndSwitchInSteps);
        _motor._calibPositiveEndSwitchInSteps.exchange(homePosition);
        _motor._calibNegativeEndSwitchInSteps.exchange(homePosition);

        _motor._calibrationMode.exchange(CalibrationMode::FULL);
      }
    }
    catch(ChimeraTK::runtime_error&) {
      _motor._motorController->setCalibrationTime(0);

      _motor._calibrationFailed.exchange(true);
      _motor._errorMode.exchange(Error::CALIBRATION_ERROR);
      _motor._calibrationMode.exchange(CalibrationMode::NONE);
    }

    _asyncActionActive.exchange(false);
  }

  void RotaryStepperMotorStateMachine::calculateToleranceValues() {
    _motor._tolerancePositiveEndSwitch = getToleranceEndSwitch(Sign::POSITIVE);
    _motor._toleranceNegativeEndSwitch = _motor._tolerancePositiveEndSwitch.load();
  }

  void RotaryStepperMotorStateMachine::findEndSwitch(Sign sign) {
    auto startTime = std::chrono::steady_clock::now();
    constexpr int HOMING_TIMEOUT_MS = 60000;
    while(!_motor.isEndSwitchActive(sign)) {
      if(_stopAction.load() || _moveInterrupted.load()) {
        return;
      }
      auto now = std::chrono::steady_clock::now();
      if(std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() > HOMING_TIMEOUT_MS) {
        _moveInterrupted.exchange(true);
        return;
      }
      if(_motor.getTargetPositionInSteps() != _motor.getCurrentPositionInSteps() &&
          !_motor.isPositiveReferenceActive() && !_motor.isNegativeReferenceActive()) {
        _moveInterrupted.exchange(true);
        return;
      }
      moveToEndSwitch(sign);
    }
  }

  int RotaryStepperMotorStateMachine::getPositionEndSwitch(Sign sign) {
    if(sign == Sign::POSITIVE) {
      return _motor._calibPositiveEndSwitchInSteps.load();
    }
    return _motor._calibNegativeEndSwitchInSteps.load();
  }

} // namespace ChimeraTK::MotorDriver
