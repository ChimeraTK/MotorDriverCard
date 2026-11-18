// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RotaryStepperMotorStateMachine.h"

#include "MotorControler.h"
#include "RotaryStepperMotor.h"
#include "TMC429Words.h"

#include <exception>
#include <iostream>
namespace ChimeraTK::MotorDriver {

  RotaryStepperMotorStateMachine::RotaryStepperMotorStateMachine(RotaryStepperMotor& motor)
  : ReferenceStateMachine(motor) {}

  void RotaryStepperMotorStateMachine::performCalibration() {
    // This function runs in a detached thread (see ReferenceStateMachine::actionStartCalib)
    // and must not let an exception escape: that would terminate the process
    // and leave _asyncActionActive stuck, locking the machine in "calibrating".
    // The guard guarantees the flag is cleared on every exit path.
    struct AsyncActionGuard {
      std::atomic<bool>& flag;
      ~AsyncActionGuard() { flag.exchange(false); }
    } asyncActionGuard{_asyncActionActive};
    std::cerr << "rotary calibration: starting home search" << std::endl;
    _motor._calibrationFailed.exchange(false);
    _stopAction.exchange(false);
    _moveInterrupted.exchange(false);
    // Shared failure handling. Safe to run even if the communication link is
    // down: the cleanup I/O is allowed to fail.
    auto failCalibration = [this]() {
      try {
        // Clear the calibration data in the firmware and re-base the hardware
        // counter, so that a subsequent calibration attempt starts from a
        // known origin instead of drifting further with every retry.
        _motor._motorController->setCalibrationTime(0);
        _motor._motorController->setPositiveReferenceSwitchCalibration(0);
        _motor._motorController->setNegativeReferenceSwitchCalibration(0);
        _motor.resetMotorControllerPositions(0);
      }
      catch(const std::exception& e) {
        std::cerr << "RotaryStepperMotorStateMachine::performCalibration(): additionally failed to reset the "
                  << "calibration data: " << e.what() << std::endl;
      }
      _motor._calibPositiveEndSwitchInSteps.exchange(0);
      _motor._calibNegativeEndSwitchInSteps.exchange(0);
      _motor._calibrationFailed.exchange(true);
      _motor._errorMode.exchange(Error::CALIBRATION_ERROR);
      _motor._calibrationMode.exchange(CalibrationMode::NONE);
    };
    auto savedConfig = mtca4u::ReferenceConfigAndRampModeData();
    bool registerSwapped = false;
    try {
      // Establish a fixed origin for the home search: re-base the hardware
      // counter (actual and target) to 0. Homing then always starts from a
      // known position, and a stale target left over from an interrupted move
      // cannot abort the search with a "position mismatch" either.
      {
        boost::lock_guard<boost::mutex&> lck(_motor._mutex);
        _motor.resetMotorControllerPositions(0);
      }
      // During the home search enable the TMC429 hardware stop for the
      // positive (home) reference switch and only disable the stop for the
      // (usually unconnected) negative switch, so the motor reliably stops
      // when it reaches the switch. The configured register value is
      // restored on all exit paths.
      savedConfig = _motor._motorController->getReferenceConfigAndRampModeData();
      // 0x100 = DISABLE_STOP_L only: HW stop ON for the home switch (REF_R)
      _motor._motorController->setReferenceConfigAndRampModeData(mtca4u::ReferenceConfigAndRampModeData(0x100));
      registerSwapped = true;
      findEndSwitch(Sign::POSITIVE); // "home direction search"
      int homePosition = _motor.getCurrentPositionInSteps();

      if(_moveInterrupted.load() || _stopAction.load()) {
        std::cerr << "rotary calibration: home search interrupted before completion" << std::endl;
        failCalibration();
      }
      else {
        // The counter has been re-based to 0 at the start of the search, so the
        // home switch is the origin of the coordinate system. Store the
        // calibration relative to it (i.e. 0) instead of the raw hardware
        // counter value, so the persisted calibration stays valid across a
        // restart.
        std::cerr << "rotary calibration: home switch found at position " << homePosition << " steps" << std::endl;
        int calibPositiveEndSwitchInSteps = 0;
        int calibNegativeEndSwitchInSteps = 0;

        // Re-base the counter to the home switch as origin BEFORE declaring the
        // calibration valid: the search left the counter at the search offset,
        // so the raw counter is not the origin. Doing this first means the
        // firmware never holds a valid CAL_TIME together with a counter that is
        // about to change.
        _motor.resetMotorControllerPositions(0);

        _motor._motorController->setPositiveReferenceSwitchCalibration(calibPositiveEndSwitchInSteps);
        _motor._motorController->setNegativeReferenceSwitchCalibration(calibNegativeEndSwitchInSteps);
        _motor._calibPositiveEndSwitchInSteps.exchange(calibPositiveEndSwitchInSteps);
        _motor._calibNegativeEndSwitchInSteps.exchange(calibNegativeEndSwitchInSteps);
        // Written last, so that a restart never sees a valid CAL_TIME together
        // with half-updated end switch registers (CAL_TIME has no write-back
        // validity flag in the firmware map, the end switch registers do).
        _motor._motorController->setCalibrationTime(time(nullptr));

        _motor._calibrationMode.exchange(CalibrationMode::FULL);
      }
    }
    catch(ChimeraTK::runtime_error& e) {
      std::cerr << "RotaryStepperMotorStateMachine::performCalibration(): homing failed: " << e.what() << std::endl;
      failCalibration();
    }
    catch(const std::exception& e) {
      std::cerr << "RotaryStepperMotorStateMachine::performCalibration(): unexpected exception during homing: "
                << e.what() << std::endl;
      failCalibration();
    }
    if(registerSwapped) {
      try {
        _motor._motorController->setReferenceConfigAndRampModeData(savedConfig);
      }
      catch(const std::exception& e) {
        // The motor would keep the homing configuration (hardware stop active
        // on the home switch) instead of the configured value. Calibration
        // must leave the motor in its configured operating state, so report
        // this as a failure.
        std::cerr << "RotaryStepperMotorStateMachine::performCalibration(): could not restore the configured "
                  << "reference register: " << e.what() << "; calibration marked as failed" << std::endl;
        failCalibration();
      }
    }
  }

  void RotaryStepperMotorStateMachine::calculateToleranceValues() {
    _motor._tolerancePositiveEndSwitch = getToleranceEndSwitch(Sign::POSITIVE);
    _motor._toleranceNegativeEndSwitch = _motor._tolerancePositiveEndSwitch.load();
  }

  void RotaryStepperMotorStateMachine::findEndSwitch(Sign sign) {
    auto startTime = std::chrono::steady_clock::now();
    constexpr int HOMING_TIMEOUT_MS = 60000;
    int loopIterations = 0;
    int lastPosition = _motor.getCurrentPositionInSteps();
    while(!_motor.isEndSwitchActive(sign)) {
      ++loopIterations;
      if(_stopAction.load() || _moveInterrupted.load()) {
        std::cerr << "findEndSwitch: stopped by user after " << loopIterations << " iterations" << std::endl;
        return;
      }
      auto now = std::chrono::steady_clock::now();
      if(std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() > HOMING_TIMEOUT_MS) {
        std::cerr << "findEndSwitch: timeout after " << loopIterations << " iterations, end switch not found"
                  << std::endl;
        _moveInterrupted.exchange(true);
        return;
      }
      if(_motor.getTargetPositionInSteps() != _motor.getCurrentPositionInSteps() &&
          !_motor.isPositiveReferenceActive() && !_motor.isNegativeReferenceActive()) {
        std::cerr << "findEndSwitch: position mismatch (motor did not move) after " << loopIterations << " iterations"
                  << std::endl;
        _moveInterrupted.exchange(true);
        return;
      }
      moveToEndSwitch(sign);
      // moveToEndSwitch() blocks while the motor is moving. A motor that cannot
      // move at all (e.g. blocked, no current) reports "not moving" instantly, so
      // without this guard the loop would spin at full speed until the 60 s
      // timeout. Detect a stalled motor instead.
      int position = _motor.getCurrentPositionInSteps();
      if(position == lastPosition) {
        std::cerr << "findEndSwitch: motor stalled at position " << position << " after " << loopIterations
                  << " iterations" << std::endl;
        _moveInterrupted.exchange(true);
        return;
      }
      lastPosition = position;
    }
    std::cerr << "findEndSwitch: end switch active after " << loopIterations << " iterations" << std::endl;
  }

  int RotaryStepperMotorStateMachine::getPositionEndSwitch(Sign sign) {
    if(sign == Sign::POSITIVE) {
      return _motor._calibPositiveEndSwitchInSteps.load();
    }
    return _motor._calibNegativeEndSwitchInSteps.load();
  }

} // namespace ChimeraTK::MotorDriver
