/* 
 * File:   LinearStepperMotor.h
 * Author: tsk
 *
 * Created on May 23, 2014, 10:07 AM
 */

#ifndef MTCA4U_LINEAR_STEPPER_MOTOR_H
#define	MTCA4U_LINEAR_STEPPER_MOTOR_H

#include "StepperMotor.h"



namespace mtca4u {



    class LinearStepperMotor : public StepperMotor {
    
    public:

        /**
         * @brief  Constructor of the class object
         * @param  motorDriverCardDeviceName - name of the device in DMAP file
         * @param  motorDriverId - each Motor Card Driver has two independent Motor Drivers (can drive two physical motors). ID defines which motor should be represented by this class instantiation  
         * @param  motorDriverCardConfigFileName - name of configuration file
         * @return
         */
        LinearStepperMotor(std::string motorDriverCardDeviceName, unsigned int motorDriverId, std::string motorDriverCardConfigFileName);
        
        /**
         * @brief  Destructor of the class object
         */
        ~LinearStepperMotor();

        virtual void setCurrentPositionAs(float newPosition);
        virtual MotorStatusAndError moveToPosition(float newPosition);
        
        virtual void stop();
        
        /**
         * @brief Perform calibration of the motor (blocking). 
         * @param
         * @return void
         * 
         * @details 
         * TO BE ADDED
         */ 
        virtual StepperMotorCalibrationStatus calibrateMotor();


        
        //end switches positions getters
        float getPositiveEndSwitchPosition() const;
        float getNegativeEndSwitchPosition() const;

         LinearStepperMotorStatus getStatus();
         LinearStepperMotorError getError();
        
    protected: // methods

        virtual MotorStatusAndError determineMotorStatusAndError();

    
    private: // fields

        
        //calibration
        int _calibNegativeEndSwitchInSteps;
        float _calibNegativeEndSwitchInUnits;
        
        int _calibPositiveEndSwitchInSteps;
        float _calibPositiveEndSwitchInUnits;
        
        
        bool _stopMotorCalibration;
          
    };

} //namespace mtca4u

#endif	/* MTCA4U_LINEAR_STEPPER_MOTOR_H */

