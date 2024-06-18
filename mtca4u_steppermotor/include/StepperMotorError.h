/*
 * File:   StepperMotorError.h
 * Author: tsk
 *
 * Created on May 16, 2014, 5:04 PM
 */

#ifndef STEPPER_MOTOR_ERROR_H
#define STEPPER_MOTOR_ERROR_H

#include "GeneralStatus.h"

#include <string>

namespace mtca4u {
  using namespace ChimeraTK;

  class StepperMotorError;
  class LinearStepperMotorError;

  class StepperMotorErrorTypes {
   public:
    static const StepperMotorError M_NO_ERROR;
    static const StepperMotorError M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX;
    static const StepperMotorError M_COMMUNICATION_LOST;
    static const StepperMotorError M_NO_REACTION_ON_COMMAND;
    static const StepperMotorError M_HARDWARE_NOT_CONNECTED;
    virtual ~StepperMotorErrorTypes() {}
  };

  class LinearStepperMotorErrorTypes : public StepperMotorErrorTypes {
   public:
    static const LinearStepperMotorError M_BOTH_END_SWITCH_ON;
  };

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // StepperMotorError class
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  class StepperMotorError : public GeneralStatus {
   public:
    using GeneralStatus::GeneralStatus;
    // default constructor uses diffeent default value than the GeneralStatus
    StepperMotorError();

    std::string asString() const override;
  };

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // LinearStepperMotorError class
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  class LinearStepperMotorError : public StepperMotorError {
   public:
    // just inherit all constructors
    using StepperMotorError::StepperMotorError;

    std::string asString() const override;
  };
} // namespace mtca4u

#endif /* STEPPER_MOTOR_ERROR_H */
