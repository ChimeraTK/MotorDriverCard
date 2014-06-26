/* 
 * File:   StepperMotorCalibrationStatus.h
 * Author: tsk
 *
 * Created on May 16, 2014, 5:25 PM
 */
#ifndef STEPPER_MOTOR_CALIBRATION_STATUS_H
#define	STEPPER_MOTOR_CALIBRATION_STATUS_H

#include <string>
#include "GeneralStatus.h"

namespace mtca4u {
    
    
    class StepperMotorCalibrationStatus;
    
     class StepperMotorCalibrationStatusType {
     public:
        static const StepperMotorCalibrationStatus M_CALIBRATED;
        static const StepperMotorCalibrationStatus M_NOT_CALIBRATED;
        static const StepperMotorCalibrationStatus M_CALIBRATION_UNKNOWN;
        static const StepperMotorCalibrationStatus M_CALIBRATION_FAILED;
        static const StepperMotorCalibrationStatus M_CALIBRATION_IN_PROGRESS;
        static const StepperMotorCalibrationStatus M_CALIBRATION_STOPED_BY_USER;
        static const StepperMotorCalibrationStatus M_CALIBRATION_NOT_AVAILABLE;
    };   
    

    class StepperMotorCalibrationStatus : public GeneralStatus {

    public:
        //default constructor
        StepperMotorCalibrationStatus();
        //param constructor
        StepperMotorCalibrationStatus(int id);
        //copy constructor
        StepperMotorCalibrationStatus(const StepperMotorCalibrationStatus &status);

        virtual std::string asString() const;
    };

} 

#endif	/* STEPPER_MOTOR_CALIBRATION_STATUS_H */

