// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "LinearStepperMotorStateMachine.h"

#include "LinearStepperMotor.h"
#include "MotorControler.h"

namespace ChimeraTK::MotorDriver {
  LinearStepperMotorStateMachine::LinearStepperMotorStateMachine(LinearStepperMotor& motor)
  : ReferenceStateMachine(motor) {}
  void LinearStepperMotorStateMachine::performCalibration() {
    _motor._calibrationFailed.exchange(false);
    _stopAction.exchange(false);
    _moveInterrupted.exchange(false);

    try {
      // Local variables for calibrated position
      int calibPositiveEndSwitchInSteps = 0;
      int calibNegativeEndSwitchInSteps = 0;

      // At least one end switch disabled -> calibration not possible
      if(!(_motor._positiveEndSwitchEnabled.load() && _motor._negativeEndSwitchEnabled.load())) {
        _motor._motorController->setCalibrationTime(0);

        _motor._calibrationFailed.exchange(true);
        _motor._calibrationMode.exchange(CalibrationMode::NONE);
      }
      else {
        findEndSwitch(Sign::POSITIVE);
        calibPositiveEndSwitchInSteps = _motor.getCurrentPositionInSteps();
        findEndSwitch(Sign::NEGATIVE);
        calibNegativeEndSwitchInSteps = _motor.getCurrentPositionInSteps();

        if(_moveInterrupted.load() || _stopAction.load()) {
          _motor._motorController->setCalibrationTime(0);

          _motor._calibrationFailed.exchange(true);
          _motor._errorMode.exchange(Error::CALIBRATION_ERROR);
          _motor._calibrationMode.exchange(CalibrationMode::NONE);
        }
        else {
          // Define positive axis with negative end switch as zero
          calibPositiveEndSwitchInSteps = calibPositiveEndSwitchInSteps - calibNegativeEndSwitchInSteps;
          calibNegativeEndSwitchInSteps = 0;

          _motor._motorController->setCalibrationTime(time(nullptr));
          _motor._motorController->setPositiveReferenceSwitchCalibration(calibPositiveEndSwitchInSteps);
          _motor._motorController->setNegativeReferenceSwitchCalibration(0);
          _motor._calibPositiveEndSwitchInSteps.exchange(calibPositiveEndSwitchInSteps);
          _motor._calibNegativeEndSwitchInSteps.exchange(0);

          _motor._calibrationMode.exchange(CalibrationMode::FULL);
          _motor.resetMotorControllerPositions(0);
        }
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
  void LinearStepperMotorStateMachine::calculateToleranceValues() {
    // Linear
    _motor._tolerancePositiveEndSwitch.exchange(getToleranceEndSwitch(Sign::POSITIVE));
    _motor._toleranceNegativeEndSwitch.exchange(getToleranceEndSwitch(Sign::NEGATIVE));
  }

  void LinearStepperMotorStateMachine::findEndSwitch(Sign sign) {
    while(!_motor.isEndSwitchActive(sign)) {
      if(_stopAction.load() || _moveInterrupted.load()) {
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

  int LinearStepperMotorStateMachine::getPositionEndSwitch(Sign sign) {
    if(sign == Sign::POSITIVE) {
      return _motor._calibPositiveEndSwitchInSteps.load();
    }
    else {
      return _motor._calibNegativeEndSwitchInSteps.load();
    }
  }

} // namespace ChimeraTK::MotorDriver
