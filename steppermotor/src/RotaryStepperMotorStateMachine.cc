// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "RotaryStepperMotorStateMachine.h"

#include "MotorControler.h"
#include "RotaryStepperMotor.h"

#include <ChimeraTK/cppext/finally.hpp>
#include <ChimeraTK/Exception.h>

#include <chrono>
#include <cmath>
#include <exception>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
namespace ChimeraTK::MotorDriver {

  RotaryStepperMotorStateMachine::RotaryStepperMotorStateMachine(RotaryStepperMotor& motor)
  : ReferenceStateMachine(motor) {}

  void RotaryStepperMotorStateMachine::performCalibration() {
    // Publish worker completion after all homing cleanup.
    auto asyncActionGuard = cppext::finally([&] { _asyncActionActive.store(false); });
    std::cerr << "rotary calibration: starting home search" << std::endl;
    _motor._calibrationFailed.exchange(false);
    _stopAction.exchange(false);
    _moveInterrupted.exchange(false);
    // Confirm the target is cleared before releasing the hardware stop.
    bool searchTargetCleared = false;
    auto clearSearchTarget = [&] {
      searchTargetCleared = false;
      _motor.resetMotorControllerPositions(0);
      const int target = _motor._motorController->getTargetPosition();
      searchTargetCleared = target == 0;
      if(!searchTargetCleared) {
        std::cerr << "Rotary homing target readback: expected 0, got " << target << std::endl;
      }
    };
    auto failCalibration = [&] {
      std::lock_guard<boost::mutex> lock(_motor._mutex);
      try {
        clearSearchTarget();
      }
      catch(const std::exception& e) {
        std::cerr << "Could not reset rotary homing target: " << e.what() << std::endl;
      }
      invalidateCalibration();
      try {
        _motor._motorController->setPositiveReferenceSwitchCalibration(0);
        _motor._motorController->setNegativeReferenceSwitchCalibration(0);
      }
      catch(const std::exception& e) {
        std::cerr << "RotaryStepperMotorStateMachine::performCalibration(): additionally failed to reset the "
                  << "calibration data: " << e.what() << std::endl;
      }
      _motor._calibPositiveEndSwitchInSteps.exchange(0);
      _motor._calibNegativeEndSwitchInSteps.exchange(0);
    };
    bool calibrationInvalidated = false;
    try {
      double savedSpeed;
      {
        std::lock_guard<boost::mutex> lock(_motor._mutex);
        savedSpeed = _motor._motorController->getUserSpeedLimit();
        if(!std::isfinite(savedSpeed) || savedSpeed <= 0 || !_motor._motorController->isMotorCurrentEnabled()) {
          std::cerr << "Rotary homing requires a positive speed limit and enabled motor current" << std::endl;
          invalidateCalibration();
          return;
        }

        // Invalidate the previous origin before changing the hardware counters.
        _motor._motorController->setCalibrationTime(0);
        calibrationInvalidated = true;
        _motor._calibrationMode.store(CalibrationMode::NONE);
        _motor.resetMotorControllerPositions(0);
      }
      bool stopDisabled = false;
      bool speedRestored = true;
      int homePosition = 0;
      {
        auto restoreStop = cppext::finally([&] {
          if(!searchTargetCleared) {
            std::cerr << "Leaving rotary home stop enabled because the search target could not be cleared" << std::endl;
            return;
          }
          try {
            _motor._motorController->setPositiveReferenceSwitchEnabled(false);
            stopDisabled = true;
          }
          catch(const std::exception& e) {
            std::cerr << "Could not disable rotary home stop: " << e.what() << std::endl;
          }
        });
        auto restoreSpeed = cppext::finally([&] {
          if(!speedRestored) {
            try {
              _motor._motorController->setUserSpeedLimit(savedSpeed);
              speedRestored = true;
            }
            catch(const std::exception& e) {
              std::cerr << "Could not restore rotary speed limit: " << e.what() << std::endl;
            }
          }
        });
        try {
          _motor._motorController->setPositiveReferenceSwitchEnabled(true);
          // Share one deadline across the coarse search, back-off, and fine approach.
          auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);
          // The base class never calls findEndSwitch. The virtual declaration and rotary wrapper can be removed.
          findEndSwitch(Sign::POSITIVE, deadline);

          refineHomePosition(deadline, savedSpeed, speedRestored);
          homePosition = _motor.getCurrentPositionInSteps();
          std::unique_lock<boost::mutex> lock(_motor._mutex);
          if(_moveInterrupted.load() || _stopAction.load()) {
            lock.unlock();
            failCalibration();
            return;
          }
          // Clear the search target before releasing the hardware stop.
          clearSearchTarget();
        }
        catch(const std::exception& e) {
          std::cerr << "Rotary home search failed: " << e.what() << std::endl;
          failCalibration();
          return;
        }
      }

      std::unique_lock<boost::mutex> lock(_motor._mutex);
      if(!stopDisabled || !speedRestored || _moveInterrupted.load() || _stopAction.load()) {
        lock.unlock();
        failCalibration();
      }
      else {
        // The counter was re-based at the home switch. Persist that origin
        // so the saved calibration is independent of the search offset.
        std::cerr << "rotary calibration: home switch found at position " << homePosition << " steps" << std::endl;
        constexpr int calibPositiveEndSwitchInSteps = 0;
        constexpr int calibNegativeEndSwitchInSteps = 0;

        _motor._motorController->setPositiveReferenceSwitchCalibration(calibPositiveEndSwitchInSteps);
        _motor._motorController->setNegativeReferenceSwitchCalibration(calibNegativeEndSwitchInSteps);
        _motor._calibPositiveEndSwitchInSteps.exchange(calibPositiveEndSwitchInSteps);
        _motor._calibNegativeEndSwitchInSteps.exchange(calibNegativeEndSwitchInSteps);
        // Written last, so that a restart never sees a valid CAL_TIME together
        // with half-updated end switch registers (CAL_TIME has no write-back
        // validity flag in the firmware map, the end switch registers do).
        _motor._motorController->setCalibrationTime(time(nullptr));

        // A successful retry clears only the calibration error.
        auto previousError = Error::CALIBRATION_ERROR;
        _motor._errorMode.compare_exchange_strong(previousError, Error::NO_ERROR);
        _motor._calibrationMode.exchange(CalibrationMode::FULL);
      }
    }
    catch(const std::exception& e) {
      std::cerr << "Rotary calibration failed: " << e.what() << std::endl;
      if(calibrationInvalidated) {
        failCalibration();
      }
      else {
        std::lock_guard<boost::mutex> lock(_motor._mutex);
        invalidateCalibration();
      }
    }
  }

  void RotaryStepperMotorStateMachine::refineHomePosition(
      HomingDeadline deadline, double savedSpeed, bool& speedRestored) {
    // Move backward until the positive switch clears, including on an active start.
    constexpr int backOffSteps = 100;
    const int switchActivePosition = _motor.getCurrentPositionInSteps();
    for(int attempt = 0; _motor.isEndSwitchActive(Sign::POSITIVE); ++attempt) {
      if(attempt == 50) {
        throw ChimeraTK::runtime_error("Rotary home switch did not clear during back-off");
      }
      moveForHoming(-backOffSteps, deadline);
    }
    // Leave clearance before reversing for the final approach.
    moveForHoming(-backOffSteps, deadline);
    if(_motor.isEndSwitchActive(Sign::POSITIVE)) {
      throw ChimeraTK::runtime_error("Rotary home switch active after clearance move");
    }

    // Approach the same switch edge at reduced speed from either starting state.
    speedRestored = false;
    const double appliedSpeed = _motor._motorController->setUserSpeedLimit(savedSpeed / 4);
    if(!std::isfinite(appliedSpeed) || appliedSpeed <= 0 || appliedSpeed >= savedSpeed) {
      throw ChimeraTK::runtime_error("Controller cannot apply a reduced rotary homing speed");
    }
    // Search back to the known active position with one increment of margin.
    const int fineStart = _motor.getCurrentPositionInSteps();
    const int fineMoveSteps = switchActivePosition - fineStart + backOffSteps;
    if(fineMoveSteps <= 0) {
      throw ChimeraTK::runtime_error("Rotary back-off did not establish a forward approach");
    }
    moveForHoming(fineMoveSteps, deadline);
    if(!_motor.isEndSwitchActive(Sign::POSITIVE)) {
      throw ChimeraTK::runtime_error(
          "Rotary home switch did not activate during final approach (known active position " +
          std::to_string(switchActivePosition) + ", start " + std::to_string(fineStart) + ", requested forward steps " +
          std::to_string(fineMoveSteps) + ", final position " + std::to_string(_motor.getCurrentPositionInSteps()) +
          ")");
    }
  }

  void RotaryStepperMotorStateMachine::checkHomingDeadline(HomingDeadline deadline) {
    if(_stopAction.load() || _moveInterrupted.load()) {
      throw ChimeraTK::runtime_error("Rotary homing cancelled");
    }
    if(std::chrono::steady_clock::now() >= deadline) {
      throw ChimeraTK::runtime_error("Rotary homing timed out");
    }
  }

  void RotaryStepperMotorStateMachine::moveForHoming(int offset, HomingDeadline deadline) {
    int startPosition;
    {
      std::lock_guard<boost::mutex> lock(_motor._mutex);
      checkHomingDeadline(deadline);
      startPosition = _motor._motorController->getActualPosition();
      _motor._motorController->setTargetPosition(startPosition + offset);
    }
    while(_motor._motorController->isMotorMoving()) {
      checkHomingDeadline(deadline);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    checkHomingDeadline(deadline);
    if(_motor.getCurrentPositionInSteps() == startPosition &&
        !(offset > 0 && _motor.isEndSwitchActive(Sign::POSITIVE))) {
      throw ChimeraTK::runtime_error("Rotary motor made no progress during homing");
    }
  }

  void RotaryStepperMotorStateMachine::findEndSwitch(Sign sign, HomingDeadline deadline) {
    if(sign != Sign::POSITIVE) {
      throw ChimeraTK::logic_error("Rotary homing requires the positive reference input");
    }
    for(int attempt = 0; !_motor.isEndSwitchActive(sign); ++attempt) {
      if(attempt == 100) {
        throw ChimeraTK::runtime_error("Rotary home switch not found within search travel");
      }
      moveForHoming(getOffset(), deadline);
    }
    checkHomingDeadline(deadline);
  }

  void RotaryStepperMotorStateMachine::findEndSwitch(Sign sign) {
    findEndSwitch(sign, std::chrono::steady_clock::now() + std::chrono::seconds(60));
  }

  int RotaryStepperMotorStateMachine::getPositionEndSwitch(Sign sign) {
    if(sign == Sign::POSITIVE) {
      return _motor._calibPositiveEndSwitchInSteps.load();
    }
    return _motor._calibNegativeEndSwitchInSteps.load();
  }

} // namespace ChimeraTK::MotorDriver
