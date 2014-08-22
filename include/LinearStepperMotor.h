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

    /**
     * @class LinearStepperMotorStatusAndError
     * @details Container class for status and error of LinearStepperMotor. These two object describes the state of motor and create a pair.
     */ 
    class LinearStepperMotorStatusAndError {
    public:
        LinearStepperMotorStatus status;
        LinearStepperMotorError error;

        LinearStepperMotorStatusAndError(StepperMotorStatus statusParam, StepperMotorError errorParam) {
            status = static_cast<LinearStepperMotorStatus>(statusParam);
            error = static_cast<LinearStepperMotorError>(errorParam);
        }

        LinearStepperMotorStatusAndError() {
        }
    };


    class LinearStepperMotor : public StepperMotor {
    
    public:

        /**
         * @brief  Constructor of the class object
         * @param  motorDriverCardDeviceName - name of the device in DMAP file
         * @param  motorDriverId - each Motor Card Driver has two independent Motor Drivers (can drive two physical motors). ID defines which motor should be represented by this class instantiation  
         * @param  motorDriverCardConfigFileName - name of configuration file
         * @return
         */
        LinearStepperMotor(std::string motorDriverCardDeviceName, unsigned int motorDriverId, std::string motorDriverCardConfigFileName, std::string pathToDmapFile = std::string("."));
        
        /**
         * @brief  Destructor of the class object
         */
        ~LinearStepperMotor();

        /**
         * @brief Sending motor to new position (blocking).
         * @param  newPosition - new position for the motor 
         * @return LinearStepperMotorStatusAndError - status of the motor after movement routine finish  
         * 
         * @details 
         * This method sends motor to new position. It is blocking blocking method. It will finish when motor movement will end with success or error. 
         * Function can be interrupted by using 'void stopMotor()' method.\n
         * Function automatically detects end switches conditions.\n 
         * It allows to perform movement in case NEGATIVE END SWITCH is active but target position in grater than current one.\n
         * It allows to perform movement in case POSITIVE END SWITCH is active but target position in smaller than current one.\n
         * Function returns StepperMotorStatus object which is status of the motor after movement routine finished. Should be LinearStepperMotorStatusTypes::M_OK in case of success.
         * 
         */
        LinearStepperMotorStatusAndError moveToPosition(float newPosition);
                
        /**
         * @brief Stop the motor 
         * @param
         * @return void
         * 
         * @details 
         * Function stop the motor in a way that real position from the motor is set as a target position.\n
         * It also interrupts all blocking functions as 'LinearStepperMotorStatusAndError moveToPosition(float newPosition)' or 'StepperMotorCalibrationStatus calibrateMotor()'.\n
         * Delays in communication can trigger effect that motor will move a few steps in current movement direction and later will move couple steps back, but this issue can be filtered by motor hysteresis effect.\n
         */ 
        virtual void stop();
        
        
        /**
         * @brief Emergency stop the motor by disabling driver.
         * @param
         * @return void
         * 
         * @details 
         * Function stops the motor by disabling driver. It means that it is done in the fastest possible way.\n
         * After emergency stop motor calibration can be lost so execution of this method influence calibration status.\n
         * Function internally calls 'void stopMotor()' method so all blocking functions will be interrupted.\n
         */  
        virtual void emergencyStop();

        /**
         * @brief Set position passed as parameter as current position of the motor
         * @param  newPosition - new position for the motor expressed in arbitrary units
         * @return void
         * 
         * @details 
         * Set given position as a current position of the motor.\n
         * Position is expressed in arbitrary units and internally recalculated into steps.\n
         * Can be used in example to zeroed current position.\n
         * When motor is calibrated it also will recalculate physical end switches positions to make then still valid.
         */
        virtual void setCurrentPositionAs(float newPosition);
        
        
        /**
         * @brief Perform calibration of the motor (blocking). 
         * @param
         * @return void
         * 
         * @details 
         * Calibration is done in order to find positions of the physical end switches.
         */ 
        virtual StepperMotorCalibrationStatus calibrateMotor();

        /**
         * @brief When motor calibrated, function returns positive end switch position expressed in arbitrary units
         * @param
         * @return void
         * 
         * @details 
         */
        float getPositiveEndSwitchPosition() const;

        /**
         * @brief When motor calibrated, function returns negative end switch position expressed in arbitrary units
         * @param
         * @return void
         * 
         * @details 
         */
        float getNegativeEndSwitchPosition() const;
        
                 
        /**
         * @brief Return object which contains current status and error.
         * @param
         * @return LinearStepperMotorStatusAndError - object which contains of status and error code
         * 
         * @details 
         * Return current motor status and error as LinearStepperMotorStatusAndError object.\n
         * It give possibility to check what was an error type when status is set to LinearStepperMotorStatusAndError::M_ERROR.
         * Status and error create pair which describes current state of the object.
         * Every time it is called it checks status of the motor. 
         * 
         * Additionally to status of StepperMotor extra state types has been added to :\n
         * 1) StepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON - hardware positive end switch has been triggered. Motor is on positive end.\n
         * 2) StepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON - hardware negative end switch has been triggered. Motor is on negative end.\n
         * 
         * Additionally to errors of StepperMotor extra error type has been added to :\n
         * 1) StepperMotorErrorTypes::M_BOTCH_END_SWITCH_ON -both end switch active. Hardware is missing or in error state.\n
         * */
        LinearStepperMotorStatusAndError getStatusAndError();
 
        
    protected: // methods

        LinearStepperMotorStatusAndError determineMotorStatusAndError();
        void moveToEndSwitch(LinearStepperMotorStatus targetStatus);
    
    private: // fields

        //calibration
        //position of end switches expressed in steps and units
        int _calibNegativeEndSwitchInSteps;
        float _calibNegativeEndSwitchInUnits;
        
        int _calibPositiveEndSwitchInSteps;
        float _calibPositiveEndSwitchInUnits;
        
        bool _stopMotorCalibration;          
    };

} //namespace mtca4u

#endif	/* MTCA4U_LINEAR_STEPPER_MOTOR_H */

