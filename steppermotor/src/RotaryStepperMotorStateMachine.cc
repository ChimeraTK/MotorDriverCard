// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * RotaryStepperMotorStateMachine.cc
 */

#include "MotorControler.h"
#include "RotaryStepperMotor.h"

#include <ChimeraTK/Exception.h>

#include <iostream>
#include <thread>

constexpr unsigned wakeupPeriodInMilliseconds = 500U;

namespace ChimeraTK::MotorDriver {

  using utility::StateMachine;

  /***************************************************
   * Events
   **************************************************/
  const StateMachine::Event RotaryStepperMotor::StateMachine::calibEvent("calibEvent");

  /***************************************************
   * Constructor
   **************************************************/
  RotaryStepperMotor::StateMachine::StateMachine(RotaryStepperMotor& motor)
  : BasicStepperMotor::StateMachine(motor), _calibrating("calibrating"), _motor(motor), _stopAction(false),
    _moveInterrupted(false) {
    /***************************************************
     * Idle → Calibrate
     **************************************************/
    _idle.setTransition(calibEvent, &_calibrating, [this]() { actionStartCalib(); }, [this]() { actionEndCallback(); });

    /***************************************************
     * Calibrating → Idle / Error
     **************************************************/
    _calibrating.setTransition(BasicStepperMotor::StateMachine::stopEvent, &_idle, [this]() { actionStop(); });

    _calibrating.setTransition(BasicStepperMotor::StateMachine::emergencyStopEvent, &_error, [this]() {
      _stopAction = true;
      _moveInterrupted = true;
      actionEmergencyStop();
    });

    _calibrating.setTransition(BasicStepperMotor::StateMachine::errorEvent, &_error, []() {});
  }

  /***************************************************
   * Stop action
   **************************************************/
  void RotaryStepperMotor::StateMachine::actionStop() {
    _stopAction = true;
  }

  /***************************************************
   * Start calibration thread
   **************************************************/
  void RotaryStepperMotor::StateMachine::actionStartCalib() {
    _asyncActionActive = true;

    std::thread(&StateMachine::calibrationThreadFunction, this).detach();
  }

  /***************************************************
   * End callback
   **************************************************/
  void RotaryStepperMotor::StateMachine::actionEndCallback() {
    if(!_asyncActionActive.load()) {
      if(hasRequestedState()) {
        moveToRequestedState();
      }

      performTransition(stopEvent);
    }
  }

  /***************************************************
   * ROTARY CALIBRATION CORE LOGIC
   **************************************************/
  void RotaryStepperMotor::StateMachine::calibrationThreadFunction() {
    _motor._calibrationFailed = false;
    _stopAction = false;
    _moveInterrupted = false;

    try {
      /***************************************************
       * ROTARY ASSUMPTION:
       * Only ONE reference (home/index switch)
       **************************************************/
      if(!_motor._homeSwitchEnabled) {
        std::cout << "Rotary calibration not possible: no home switch\n";

        _motor._motorController->setCalibrationTime(0);
        _motor._calibrationFailed = true;
        _motor._calibrationMode = CalibrationMode::NONE;
      }
      else {
        /***************************************************
         * Step until reference detected
         **************************************************/
        while(!_motor.isPositiveReferenceActive()) {
          if(_stopAction || _moveInterrupted) {
            _motor._motorController->setCalibrationTime(0);
            _motor._calibrationFailed = true;
            _motor._calibrationMode = CalibrationMode::NONE;
            _motor._errorMode = Error::CALIBRATION_ERROR;
            _asyncActionActive = false;
            return;
          }

          /***************************************************
           * Small incremental motion (SEARCH MODE)
           **************************************************/
          {
            boost::lock_guard<boost::mutex> lock(_motor._mutex);

            _motor._motorController->setTargetPosition(_motor._motorController->getActualPosition() + 200);
          }

          while(_motor._motorController->isMotorMoving()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(wakeupPeriodInMilliseconds));
          }
        }

        /***************************************************
         * Home found → define zero
         **************************************************/
        _motor._motorController->setCalibrationTime(time(nullptr));

        _motor._homeReferenceInSteps = _motor._motorController->getActualPosition();

        _motor.resetMotorControllerPositions(0);

        _motor._calibrationMode = CalibrationMode::FULL;
      }
    }
    catch(ChimeraTK::runtime_error&) {
      _motor._motorController->setCalibrationTime(0);
      _motor._calibrationFailed = true;
      _motor._calibrationMode = CalibrationMode::NONE;
      _motor._errorMode = Error::CALIBRATION_ERROR;
    }

    _asyncActionActive = false;
  }

} // namespace ChimeraTK::MotorDriver
