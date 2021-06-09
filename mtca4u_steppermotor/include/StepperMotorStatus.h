/*
 * File:   StepperMotorStatus.h
 * Author: tsk
 *
 * Created on May 16, 2014, 5:16 PM
 */

#ifndef STEPPER_MOTOR_STATUS_H
#define STEPPER_MOTOR_STATUS_H

#include "GeneralStatus.h"
#include <string>

namespace mtca4u {

  class StepperMotorStatus;
  class LinearStepperMotorStatus;

  class StepperMotorStatusTypes {
   public:
    static const StepperMotorStatus M_OK;
    static const StepperMotorStatus M_DISABLED;
    static const StepperMotorStatus M_IN_MOVE;
    static const StepperMotorStatus M_NOT_IN_POSITION;
    static const StepperMotorStatus M_SOFT_POSITIVE_END_SWITCHED_ON;
    static const StepperMotorStatus M_SOFT_NEGATIVE_END_SWITCHED_ON;
    static const StepperMotorStatus M_ERROR;

    virtual ~StepperMotorStatusTypes() {}
  };

  class StepperMotorStatus : public GeneralStatus {
   public:
    // default constructor uses different default value thatn GeneralStatus
    StepperMotorStatus();
    // inherit the other constructors
    using GeneralStatus::GeneralStatus;

    std::string asString() const override;
  };

  class LinearStepperMotorStatusTypes : public StepperMotorStatusTypes {
   public:
    static const LinearStepperMotorStatus M_POSITIVE_END_SWITCHED_ON;
    static const LinearStepperMotorStatus M_NEGATIVE_END_SWITCHED_ON;
  };

  class LinearStepperMotorStatus : public StepperMotorStatus {
   public:
    // inherit all constructors
    using StepperMotorStatus::StepperMotorStatus;

    // overload of asString method
    std::string asString() const override;
  };

} // namespace mtca4u

#endif /* STEPPER_MOTOR_STATUS_H */
