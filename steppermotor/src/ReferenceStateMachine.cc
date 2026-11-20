// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ReferenceStateMachine.h"

#include "MotorController.h"

#include <array>
#include <thread>

constexpr unsigned wakeupPeriodInMilliseconds = 500U;

namespace ChimeraTK::MotorDriver {

  const utility::StateMachine::Event ReferenceStateMachine::calibEvent("calibEvent");
  const utility::StateMachine::Event ReferenceStateMachine::calcToleranceEvent("calcToleranceEvent");

  ReferenceStateMachine::ReferenceStateMachine(ReferenceStepperMotor& stepperMotorWithReference)
  : BasicStepperMotor::StateMachine(stepperMotorWithReference), _calibrating("calibrating"),
    _calculatingTolerance("calculatingTolerance"), _motor(stepperMotorWithReference), _stopAction(false),
    _moveInterrupted(false) {
    _idle.setTransition(calibEvent, &_calibrating, [this]() { actionStartCalib(); }, [this]() { actionEndCallback(); });
    _idle.setTransition(
        ReferenceStateMachine::calcToleranceEvent, &_calculatingTolerance, [this] { actionStartCalcTolercance(); },
        [this] { actionEndCallback(); });

    _calibrating.setTransition(BasicStepperMotor::StateMachine::stopEvent, &_idle, [this] { actionStop(); });
    _calibrating.setTransition(BasicStepperMotor::StateMachine::emergencyStopEvent, &_error, [this] {
      _stopAction.exchange(true);
      _moveInterrupted.exchange(true);
      actionEmergencyStop();
    });
    _calibrating.setTransition(BasicStepperMotor::StateMachine::errorEvent, &_error, [] {});

    _calculatingTolerance.setTransition(BasicStepperMotor::StateMachine::stopEvent, &_idle, [this] { actionStop(); });
    _calculatingTolerance.setTransition(BasicStepperMotor::StateMachine::emergencyStopEvent, &_error, [this] {
      _stopAction.exchange(true);
      _moveInterrupted.exchange(true);
      actionEmergencyStop();
    });
  }

  void ReferenceStateMachine::actionStop() {
    _stopAction.exchange(true);
  }

  void ReferenceStateMachine::actionStartCalib() {
    _asyncActionActive.store(true);
    std::thread(&ReferenceStateMachine::calibrationThreadFunction, this).detach();
  }

  void ReferenceStateMachine::calibrationThreadFunction() {
    /*try {
        performCalibration();
    }
    catch(const ChimeraTK::runtime_error&) {
        handleCalibrationFailure();
    }

    _asyncActionActive.store(false);*/
    performCalibration();
  }

  void ReferenceStateMachine::actionEndCallback() {
    if(!_asyncActionActive.load()) {
      if(hasRequestedState()) {
        moveToRequestedState();
      }

      auto stateExitEvent = stopEvent;
      // TODO include
      //      if(_moveInterrupted.load()){
      //        stateExitEvent = errorEvent;
      //      }
      performTransition(stateExitEvent);
    }
  }

  void ReferenceStateMachine::actionStartCalcTolercance() {
    _asyncActionActive.exchange(true);
    std::thread toleranceCalcThread(&ReferenceStateMachine::toleranceCalcThreadFunction, this);
    toleranceCalcThread.detach();
  }

  void ReferenceStateMachine::moveToEndSwitch(Sign sign) {
    {
      boost::lock_guard<boost::mutex&> lck(_motor._mutex);
      _motor._motorController->setTargetPosition(
          _motor._motorController->getActualPosition() + static_cast<int>(sign) * getOffset());
    }
    while(_motor._motorController->isMotorMoving()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(wakeupPeriodInMilliseconds));
    }
  }

  void ReferenceStateMachine::toleranceCalcThreadFunction() {
    _motor._toleranceCalcFailed.exchange(false);
    _motor._toleranceCalculated.exchange(false);
    _stopAction.exchange(false);
    _moveInterrupted.exchange(false);

    // This makes only sense once the motor is fully calibrated
    if(_motor._calibrationMode != CalibrationMode::FULL) {
      _motor._toleranceCalculated.exchange(false);
      _motor._toleranceCalcFailed.exchange(true);
    }
    else {
      calculateToleranceValues();
      if(_stopAction.load()) {
        _motor._toleranceCalculated.exchange(false);
      }
      else if(_moveInterrupted.load()) {
        _motor._toleranceCalculated.exchange(false);
        _motor._toleranceCalcFailed.exchange(true);

        // TODO include
        //        _asyncActionActive.exchange(false);
        //        _motorController->setTargetPosition(_motorController->getActualPosition());
        _motor._errorMode = Error::CALIBRATION_ERROR;
        //        return;
      }
      else {
        _motor._toleranceCalculated.exchange(true);
      }
    }
    _asyncActionActive.exchange(false);
  }

  int ReferenceStateMachine::getPositionEndSwitch(Sign sign) {
    if(sign == Sign::POSITIVE) {
      return _motor._calibPositiveEndSwitchInSteps.load();
    }
    return _motor._calibNegativeEndSwitchInSteps.load();
  }

  double ReferenceStateMachine::getToleranceEndSwitch(Sign sign) {
    double meanMeasurement = 0;
    double stdMeasurement = 0;
    const int N_TOLERANCE_CALC_SAMPLES = 10;
    std::array<double, N_TOLERANCE_CALC_SAMPLES> measurements{};

    int endSwitchPosition = getPositionEndSwitch(sign);

    // Get 10 samples for tolerance calculation
    for(double& measurement : measurements) {
      if(_stopAction.load() || _moveInterrupted.load()) {
        break;
      }

      // Move close to end switch
      {
        boost::lock_guard<boost::mutex> lck(_motor._mutex);
        _motor._motorController->setTargetPosition(endSwitchPosition - static_cast<int>(sign) * 1000);
      }
      while(_motor._motorController->isMotorMoving()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(wakeupPeriodInMilliseconds));
      }

      // Check if in expected position
      if(!_motor.verifyMoveAction()) {
        _moveInterrupted.exchange(true);
        break;
      }

      // Try to move beyond end switch
      {
        boost::lock_guard<boost::mutex> lck(_motor._mutex);
        _motor._motorController->setTargetPosition(endSwitchPosition + static_cast<int>(sign) * 1000);
      }
      while(_motor._motorController->isMotorMoving()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(wakeupPeriodInMilliseconds));
      }
      if(!_motor.isEndSwitchActive(sign)) {
        _moveInterrupted.exchange(true);
        break;
      }

      // Mean calculation
      meanMeasurement += static_cast<double>(_motor.getCurrentPositionInSteps()) / N_TOLERANCE_CALC_SAMPLES;
      measurement = _motor.getCurrentPositionInSteps();
    } /* for (i in [0,9]) */

    // Compute variance
    if(!(_stopAction.load() || _moveInterrupted.load())) {
      for(double measurement : measurements) {
        stdMeasurement +=
            ((measurement - meanMeasurement) / (N_TOLERANCE_CALC_SAMPLES - 1)) * (measurement - meanMeasurement);
      }
    }

    return sqrt(stdMeasurement);
  }
} // namespace ChimeraTK::MotorDriver
