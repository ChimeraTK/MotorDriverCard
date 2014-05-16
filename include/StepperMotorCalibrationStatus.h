/* 
 * File:   StepperMotorCalibrationStatus.h
 * Author: tsk
 *
 * Created on May 16, 2014, 5:25 PM
 */
#ifndef STEPPER_MOTOR_CALIBRATION_STATUS_H
#define	STEPPER_MOTOR_CALIBRATION_STATUS_H

#include <string>
#include "StatusGeneral.h"

namespace mtca4u {
    
    
    class StepperMotorCalibrationStatus;
    
     class StepperMotorCalibrationStatusType {
     public:
        static StepperMotorCalibrationStatus M_CALIBRATED;
        static StepperMotorCalibrationStatus M_NOT_CALIBRATED;
        static StepperMotorCalibrationStatus M_CALIBRATION_UNKNOWN;
        static StepperMotorCalibrationStatus M_CALIBRATION_FAILED;
        static StepperMotorCalibrationStatus M_CALIBRATION_IN_PROGRESS;
        static StepperMotorCalibrationStatus M_CALIBRATION_STOPED_BY_USER;
    };   
    

    class StepperMotorCalibrationStatus : public StatusGeneral {
    
    public:
        //default constructor
        StepperMotorCalibrationStatus();
        //param constructor
        StepperMotorCalibrationStatus(int itemId, std::string itemName);
        //copy constructor
        StepperMotorCalibrationStatus(const StepperMotorCalibrationStatus &status);
        
    };

} 

#endif	/* STEPPER_MOTOR_CALIBRATION_STATUS_H */

