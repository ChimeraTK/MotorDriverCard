// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ReferenceStateMachine.h"

#include "MotorControler.h"

#include <ChimeraTK/Exception.h>

#include <thread>

constexpr unsigned wakeupPeriodInMilliseconds = 500U;

namespace ChimeraTK::MotorDriver {

  const utility::StateMachine::Event ReferenceStateMachine::calibEvent("calibEvent");

  ReferenceStateMachine::ReferenceStateMachine(ReferenceStepperMotor& stepperMotorWithReference)
  : BasicStepperMotor::StateMachine(stepperMotorWithReference), _calibrating("calibrating"),
    _motor(stepperMotorWithReference), _stopAction(false), _moveInterrupted(false) {
    _idle.setTransition(calibEvent, &_calibrating, [this]() { actionStartCalib(); }, [this]() { actionEndCallback(); });

    _calibrating.setTransition(
        BasicStepperMotor::StateMachine::stopEvent, &_idle, std::bind(&ReferenceStateMachine::actionStop, this));
    _calibrating.setTransition(BasicStepperMotor::StateMachine::emergencyStopEvent, &_error, [this] {
      _stopAction.exchange(true);
      _moveInterrupted.exchange(true);
      actionEmergencyStop();
    });
    _calibrating.setTransition(BasicStepperMotor::StateMachine::errorEvent, &_error, [] {});
  }

  void ReferenceStateMachine::actionStop() {
    _stopAction.exchange(true);
    return;
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

  int ReferenceStateMachine::getPositionEndSwitch(Sign sign) {
    if(sign == Sign::POSITIVE) {
      return _motor._calibPositiveEndSwitchInSteps.load();
    }
    else {
      return _motor._calibNegativeEndSwitchInSteps.load();
    }
  }

} // namespace ChimeraTK::MotorDriver
