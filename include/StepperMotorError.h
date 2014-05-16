/* 
 * File:   StepperMotorError.h
 * Author: tsk
 *
 * Created on May 16, 2014, 5:04 PM
 */

#ifndef STEPPER_MOTOR_ERROR_H
#define	STEPPER_MOTOR_ERROR_H

#include <string>
#include "StatusGeneral.h"


namespace mtca4u {
    class StepperMotorError;
    
    class StepperMotorErrorTypes {
    public:
        static StepperMotorError MOTOR_NO_ERROR;
        static StepperMotorError MOTOR_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX;
        static StepperMotorError MOTOR_BOTH_ENDSWICHTED_ON;
        static StepperMotorError MOTOR_COMMUNICATION_LOST;
    };


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // StepperMotorError class !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    class StepperMotorError : public StatusGeneral {
    public:
        //default constructor
        StepperMotorError();
        //constructor with params
        StepperMotorError(int itemId, std::string itemName);
        //copy constructor
        StepperMotorError(const StepperMotorError &error);

    };


} //namespace mtca4u

#endif	/* STEPPER_MOTOR_ERROR_H */

