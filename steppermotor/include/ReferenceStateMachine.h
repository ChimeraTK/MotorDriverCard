// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "ReferenceStepperMotor.h"

#include <ChimeraTK/Exception.h>

namespace ChimeraTK::MotorDriver {

  /**
   * StateMachine subclass specific to this implementation of the StepperMotor
   */
  class ReferenceStateMachine : public BasicStepperMotor::StateMachine {
    friend class ReferenceStepperMotor;

   public:
    explicit ReferenceStateMachine(ReferenceStepperMotor& stepperMotorWithReference);
    ~ReferenceStateMachine() override = default;

    static const Event calibEvent;

    bool isMoveInterrupted() const { return _moveInterrupted; }

   protected:
    State _calibrating;
    ReferenceStepperMotor& _motor;
    std::atomic<bool> _stopAction;
    std::atomic<bool> _moveInterrupted;

    void actionStop();
    void actionStartCalib();
    void actionEndCallback();
    void onActionFinished();
    void calibrationThreadFunction();
    void moveToEndSwitch(Sign sign);

    virtual void performCalibration() = 0;
    virtual int getPositionEndSwitch(Sign) = 0;
    virtual int getOffset() const = 0;
    virtual void findEndSwitch(Sign sign) = 0;
    // private:
    // std::jthread _worker;
  };
} // namespace ChimeraTK::MotorDriver
