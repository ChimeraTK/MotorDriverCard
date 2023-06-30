/*
 * StepperMotorWithReferenceStateMachine.cc
 *
 *  Created on: Feb 17, 2017
 *      Author: vitimic
 */

#include "LinearStepperMotor.h"
#include "MotorControler.h"

#include <ChimeraTK/Exception.h>

#include <mutex>

static const unsigned wakeupPeriodInMilliseconds = 500U;

namespace ChimeraTK::MotorDriver {

  using utility::StateMachine;

  const StateMachine::Event LinearStepperMotor::StateMachine::calibEvent("calibEvent");
  const StateMachine::Event LinearStepperMotor::StateMachine::calcToleranceEvent("calcToleranceEvent");

  LinearStepperMotor::StateMachine::StateMachine(LinearStepperMotor& stepperMotorWithReference)
  : BasicStepperMotor::StateMachine(stepperMotorWithReference), _calibrating("calibrating"),
    _calculatingTolerance("calculatingTolerance"), _motor(stepperMotorWithReference), _stopAction(false),
    _moveInterrupted(false) {
    _idle.setTransition(calibEvent, &_calibrating, std::bind(&LinearStepperMotor::StateMachine::actionStartCalib, this),
        std::bind(&LinearStepperMotor::StateMachine::actionEndCallback, this));
    _idle.setTransition(LinearStepperMotor::StateMachine::calcToleranceEvent,
        &_calculatingTolerance,
        std::bind(&LinearStepperMotor::StateMachine::actionStartCalcTolercance, this),
        std::bind(&LinearStepperMotor::StateMachine::actionEndCallback, this));

    _calibrating.setTransition(BasicStepperMotor::StateMachine::stopEvent, &_idle,
        std::bind(&LinearStepperMotor::StateMachine::actionStop, this));
    _calibrating.setTransition(
        BasicStepperMotor::StateMachine::emergencyStopEvent, &_error, [this] { actionEmergencyStop(); });
    _calibrating.setTransition(BasicStepperMotor::StateMachine::errorEvent, &_error, [] {});

    _calculatingTolerance.setTransition(BasicStepperMotor::StateMachine::stopEvent, &_idle, [this] { actionStop(); });
    _calculatingTolerance.setTransition(
        BasicStepperMotor::StateMachine::emergencyStopEvent, &_error, [this] { actionEmergencyStop(); });
  }

  LinearStepperMotor::StateMachine::~StateMachine() {}

  void LinearStepperMotor::StateMachine::actionStop() {
    _stopAction.exchange(true);
    return;
  }

  void LinearStepperMotor::StateMachine::actionStartCalib() {
    _asyncActionActive.exchange(true);
    std::thread calibrationThread(&LinearStepperMotor::StateMachine::calibrationThreadFunction, this);
    calibrationThread.detach();
  }

  void LinearStepperMotor::StateMachine::actionEndCallback() {
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

  void LinearStepperMotor::StateMachine::actionStartCalcTolercance() {
    _asyncActionActive.exchange(true);
    std::thread toleranceCalcThread(&LinearStepperMotor::StateMachine::toleranceCalcThreadFunction, this);
    toleranceCalcThread.detach();
  }

  void LinearStepperMotor::StateMachine::calibrationThreadFunction() {
    _motor._calibrationFailed.exchange(false);
    _stopAction.exchange(false);
    _moveInterrupted.exchange(false);

    // Local variables for calibrated position
    // TODO Move to else scope to remove cppcheck warning
    int calibPositiveEndSwitchInSteps = 0;
    int calibNegativeEndSwitchInSteps = 0;

    // At least one end switch disabled -> calibration not possible
    if(!(_motor._positiveEndSwitchEnabled.load() && _motor._negativeEndSwitchEnabled.load())) {
      _motor._motorControler->setCalibrationTime(0);

      _motor._calibrationFailed.exchange(true);
      _motor._calibrationMode.exchange(CalibrationMode::NONE);

      return;
    }

    try {
      findEndSwitch(Sign::POSITIVE);
      calibPositiveEndSwitchInSteps = _motor.getCurrentPositionInSteps();
      findEndSwitch(Sign::NEGATIVE);
      calibNegativeEndSwitchInSteps = _motor.getCurrentPositionInSteps();

      if(_moveInterrupted.load() || _stopAction.load()) {
        _motor._motorControler->setCalibrationTime(0);

        _motor._calibrationFailed.exchange(true);
        _motor._errorMode.exchange(Error::CALIBRATION_ERROR);
        _motor._calibrationMode.exchange(CalibrationMode::NONE);
      }
      else {
        // Define positive axis with negative end switch as zero
        calibPositiveEndSwitchInSteps = calibPositiveEndSwitchInSteps - calibNegativeEndSwitchInSteps;
        calibNegativeEndSwitchInSteps = 0;

        _motor._motorControler->setCalibrationTime(time(nullptr));
        _motor._motorControler->setPositiveReferenceSwitchCalibration(calibPositiveEndSwitchInSteps);
        _motor._motorControler->setNegativeReferenceSwitchCalibration(0);
        _motor._calibPositiveEndSwitchInSteps.exchange(calibPositiveEndSwitchInSteps);
        _motor._calibNegativeEndSwitchInSteps.exchange(0);

        _motor._calibrationMode.exchange(CalibrationMode::FULL);
        _motor.resetMotorControllerPositions(0);
      }
    }
    catch(ChimeraTK::runtime_error& e) {
      _motor._calibrationFailed.exchange(true);
      _motor._errorMode.exchange(Error::CALIBRATION_ERROR);
      _motor._calibrationMode.exchange(CalibrationMode::NONE);
    }

    _asyncActionActive.exchange(false);
  }

  void LinearStepperMotor::StateMachine::findEndSwitch(Sign sign) {
    while(!_motor.isEndSwitchActive(sign)) {
      if(_stopAction.load() || _moveInterrupted.load()) {
        return;
      }
      else if(_motor.getTargetPositionInSteps() != _motor.getCurrentPositionInSteps() &&
          !_motor.isPositiveReferenceActive() && !_motor.isNegativeReferenceActive()) {
        _moveInterrupted.exchange(true);
        return;
      }
      moveToEndSwitch(sign);
    }
    return;
  }

  void LinearStepperMotor::StateMachine::moveToEndSwitch(Sign sign) {
    {
      boost::lock_guard<boost::mutex&> lck(_motor._mutex);
      _motor._motorControler->setTargetPosition(
          _motor._motorControler->getActualPosition() + static_cast<int>(sign) * 50000);
    }
    while(_motor._motorControler->isMotorMoving()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(wakeupPeriodInMilliseconds));
    }
  }

  void LinearStepperMotor::StateMachine::toleranceCalcThreadFunction() {
    _motor._toleranceCalcFailed.exchange(false);
    _motor._toleranceCalculated.exchange(false);
    _stopAction.exchange(false);
    _moveInterrupted.exchange(false);

    try {
    // This makes only sense once the motor is fully calibrated
      if(_motor._calibrationMode != CalibrationMode::FULL) {
        _motor._toleranceCalculated.exchange(false);
        _motor._toleranceCalcFailed.exchange(true);
      }
      else {
        _motor._tolerancePositiveEndSwitch.exchange(getToleranceEndSwitch(Sign::POSITIVE));
        _motor._toleranceNegativeEndSwitch.exchange(getToleranceEndSwitch(Sign::NEGATIVE));

        if(_stopAction.load()) {
          _motor._toleranceCalculated.exchange(false);
        }
        else if(_moveInterrupted.load()) {
          _motor._toleranceCalculated.exchange(false);
          _motor._toleranceCalcFailed.exchange(true);

          // TODO include
          //        _asyncActionActive.exchange(false);
          //        _motorControler->setTargetPosition(_motorControler->getActualPosition());
          _motor._errorMode = Error::CALIBRATION_ERROR;
          //        return;
        }
        else {
          _motor._toleranceCalculated.exchange(true);
        }
      }
    }
    catch(ChimeraTK::runtime_error&) {
      _motor._toleranceCalculated.exchange(false);
      _motor._toleranceCalcFailed.exchange(true);
    }

    _asyncActionActive.exchange(false);
  }

  int LinearStepperMotor::StateMachine::getPositionEndSwitch(Sign sign) {
    if(sign == Sign::POSITIVE) {
      return _motor._calibPositiveEndSwitchInSteps.load();
    }
    else {
      return _motor._calibNegativeEndSwitchInSteps.load();
    }
  }

  double LinearStepperMotor::StateMachine::getToleranceEndSwitch(Sign sign) {
    double meanMeasurement = 0;
    double stdMeasurement = 0;
    const int N_TOLERANCE_CALC_SAMPLES = 10;
    double measurements[N_TOLERANCE_CALC_SAMPLES];

    int endSwitchPosition = getPositionEndSwitch(sign);

    // Get 10 samples for tolerance calculation
    for(unsigned i = 0; i < N_TOLERANCE_CALC_SAMPLES; i++) {
      if(_stopAction.load() || _moveInterrupted.load()) {
        break;
      }

      // Move close to end switch
      {
        boost::lock_guard<boost::mutex> lck(_motor._mutex);
        _motor._motorControler->setTargetPosition(endSwitchPosition - static_cast<int>(sign) * 1000);
      }
      while(_motor._motorControler->isMotorMoving()) {
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
        _motor._motorControler->setTargetPosition(endSwitchPosition + static_cast<int>(sign) * 1000);
      }
      while(_motor._motorControler->isMotorMoving()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(wakeupPeriodInMilliseconds));
      }
      if(!_motor.isEndSwitchActive(sign)) {
        _moveInterrupted.exchange(true);
        break;
      }

      // Mean calculation
      meanMeasurement += static_cast<double>(_motor.getCurrentPositionInSteps()) / N_TOLERANCE_CALC_SAMPLES;
      measurements[i] = _motor.getCurrentPositionInSteps();
    } /* for (i in [0,9]) */

    // Compute variance
    if(!(_stopAction.load() || _moveInterrupted.load())) {
      for(unsigned int i = 0; i < N_TOLERANCE_CALC_SAMPLES; i++) {
        stdMeasurement += ((measurements[i] - meanMeasurement) / (N_TOLERANCE_CALC_SAMPLES - 1)) *
            (measurements[i] - meanMeasurement);
      }
    }

    return sqrt(stdMeasurement);
  }
} // namespace ChimeraTK::MotorDriver
