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
        _motor._motorController->setCalibrationTime(0);
      }
      catch(const std::exception& e) {
        std::cerr << "RotaryStepperMotorStateMachine::performCalibration(): additionally failed to clear the "
                  << "calibration time: " << e.what() << std::endl;
      }
      _motor._calibrationFailed.exchange(true);
      _motor._errorMode.exchange(Error::CALIBRATION_ERROR);
      _motor._calibrationMode.exchange(CalibrationMode::NONE);
    };
    auto savedConfig = mtca4u::ReferenceConfigAndRampModeData();
    bool registerSwapped = false;
    try {
      // Sync the hardware target with the actual position so that a stale
      // target left over from an interrupted move cannot abort the
      // calibration search immediately ("position mismatch").
      {
        boost::lock_guard<boost::mutex&> lck(_motor._mutex);
        _motor._motorController->setTargetPosition(_motor._motorController->getActualPosition());
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
        std::cerr << "rotary calibration: home switch found at position " << homePosition << " steps" << std::endl;
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
