/* 
 * File:   StepperMotorError.h
 * Author: tsk
 *
 * Created on May 16, 2014, 5:04 PM
 */

#ifndef STEPPER_MOTOR_ERROR_H
#define	STEPPER_MOTOR_ERROR_H

#include <string>
#include "GeneralStatus.h"


namespace mtca4u {
    class StepperMotorError;
    class LinearStepperMotorError;
    
    
    

    class StepperMotorErrorTypes {
    public:
        
        static const StepperMotorError M_NO_ERROR;
        static const StepperMotorError M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX;        
        static const StepperMotorError M_COMMUNICATION_LOST;
        static const StepperMotorError M_NO_REACTION_ON_COMMAND;
        static const StepperMotorError M_HARDWARE_NOT_CONNECTED;
	virtual ~StepperMotorErrorTypes(){}
    };


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // StepperMotorError class !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    class StepperMotorError : public GeneralStatus {
    public:
        //default constructor
        StepperMotorError();
        //constructor with params
        StepperMotorError(int id);
        //copy constructor
        StepperMotorError(const StepperMotorError &error);

        virtual std::string asString() const;
        
    };

    
    
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // LinearStepperMotorError class !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    class LinearStepperMotorError : public StepperMotorError {
    public:
        //default constructor
        LinearStepperMotorError();
        //constructor with params
        LinearStepperMotorError(int id);
        //copy constructor
        LinearStepperMotorError(const LinearStepperMotorError &error);

        virtual std::string asString() const;
        
    };
    
    
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // LinearStepperMotorError class !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    class LinearStepperMotorErrorTypes : public StepperMotorErrorTypes {
    public:
        static const LinearStepperMotorError M_BOTH_END_SWITCH_ON;
    };

} //namespace mtca4u

#endif	/* STEPPER_MOTOR_ERROR_H */

