/* 
 * File:   StepperMotorStatus.h
 * Author: tsk
 *
 * Created on May 16, 2014, 5:16 PM
 */

#ifndef STEPPER_MOTOR_STATUS_H
#define	STEPPER_MOTOR_STATUS_H

#include <string>
#include "StatusGeneral.h"


namespace mtca4u {


    class StepperMotorStatus;
    class LinearStepperMotorStatus;
    
    class LinearStepperMotorStatusTypes {
    public:
        static LinearStepperMotorStatus M_OK;
        static LinearStepperMotorStatus M_IN_MOVE;
        static LinearStepperMotorStatus M_NOT_IN_POSITION;        
        static LinearStepperMotorStatus M_POSITIVE_END_SWITCHED_ON;
        static LinearStepperMotorStatus M_NEGATIVE_END_SWITCHED_ON;
        static LinearStepperMotorStatus M_SOFT_POSITIVE_END_SWITCHED_ON;
        static LinearStepperMotorStatus M_SOFT_NEGATIVE_END_SWITCHED_ON;
        static LinearStepperMotorStatus M_ERROR;
        static LinearStepperMotorStatus M_DISABLED;

    };
    

    class StepperMotorStatusTypes {
    public:
        static StepperMotorStatus M_OK;
        static StepperMotorStatus M_IN_MOVE;
        static StepperMotorStatus M_NOT_IN_POSITION;        
        static StepperMotorStatus M_SOFT_POSITIVE_END_SWITCHED_ON;
        static StepperMotorStatus M_SOFT_NEGATIVE_END_SWITCHED_ON;
        static StepperMotorStatus M_ERROR;
        static StepperMotorStatus M_DISABLED;

    };
    

    class StepperMotorStatus : public StatusGeneral {
    
    public:
        //default constructor
        StepperMotorStatus();
        //param constructor
        StepperMotorStatus(int itemId, std::string itemName);
        //copy constructor
        StepperMotorStatus(const StepperMotorStatus &status);
        
    };

    class LinearStepperMotorStatus : public StepperMotorStatus {
    public:
        //default constructor
        LinearStepperMotorStatus();
        //param constructor
        LinearStepperMotorStatus(int itemId, std::string itemName);
        //copy constructor
        LinearStepperMotorStatus(const LinearStepperMotorStatus &status);

    };
    

} //namespace mtca4u

#endif	/* STEPPER_MOTOR_STATUS_H */

