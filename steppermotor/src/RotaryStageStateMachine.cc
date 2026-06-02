#include "RotaryStageStateMachine.h"

#include "MotorControler.h"
#include "RotaryStage.h"

namespace ChimeraTK::MotorDriver {

  RotaryStageStateMachine::RotaryStageStateMachine(RotaryStage& motor) : ReferenceStateMachine(motor) {}

  void RotaryStageStateMachine::performCalibration() {
    _motor._calibrationFailed.exchange(false);
    _stopAction.exchange(false);
    _moveInterrupted.exchange(false);
    try {
      // Local variables for calibrated position
      // TODO Move to else scope to remove cppcheck warning
      int calibPositiveEndSwitchInSteps = 0;
      int calibNegativeEndSwitchInSteps = 0;

      // At least one end switch disabled -> calibration not possible
      // not required for rotary
      if(!(_motor._positiveEndSwitchEnabled.load() && _motor._negativeEndSwitchEnabled.load())) {
        _motor._motorController->setCalibrationTime(0);
        _motor._calibrationFailed.exchange(true);
        _motor._calibrationMode.exchange(CalibrationMode::NONE);
      }
      else {
        findEndSwitch(Sign::POSITIVE); // reuse as "home direction search"
        int homePosition = _motor.getCurrentPositionInSteps();

        if(_moveInterrupted.load() || _stopAction.load()) {
          _motor._motorController->setCalibrationTime(0);
          _motor._calibrationFailed.exchange(true);
          _motor._errorMode.exchange(Error::CALIBRATION_ERROR);
          _motor._calibrationMode.exchange(CalibrationMode::NONE);
        }
        else {
          // Define positive axis with negative end switch as zero
          calibPositiveEndSwitchInSteps = homePosition; // unused in rotary
          calibNegativeEndSwitchInSteps = 0;
          // instead
          //_motor._homeOffsetInSteps.exchange(homePosition);
          _motor._motorController->setCalibrationTime(time(nullptr));
          _motor._motorController->setPositiveReferenceSwitchCalibration(calibPositiveEndSwitchInSteps);
          _motor._motorController->setNegativeReferenceSwitchCalibration(0);
          _motor._calibPositiveEndSwitchInSteps.exchange(homePosition);
          _motor._calibNegativeEndSwitchInSteps.exchange(0);

          _motor._calibrationMode.exchange(CalibrationMode::FULL);
          //_motor._calibrationMode.exchange(CalibrationMode::HOMED);
          _motor.resetMotorControllerPositions(homePosition);
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

  void RotaryStageStateMachine::calculateToleranceValues() {
    // Rotary
    _motor._tolerancePositiveEndSwitch = getToleranceEndSwitch(Sign::POSITIVE);
    _motor._toleranceNegativeEndSwitch = _motor._tolerancePositiveEndSwitch.load();
  }

  void RotaryStageStateMachine::findEndSwitch(Sign sign) {
    // Prevent infinite rotation in rotary systems
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

  int RotaryStageStateMachine::getPositionEndSwitch(Sign sign) {
    return _motor._calibNegativeEndSwitchInSteps.load();
  }

} // namespace ChimeraTK::MotorDriver
