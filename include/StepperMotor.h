/* 
 * File:   StepperMotor.h
 * Author: tkozak
 *
 * Created on April 8, 2014, 11:49 AM
 */


#ifndef MTCA4U_STEPPER_MOTOR_H
#define	MTCA4U_STEPPER_MOTOR_H

#include <string>
#include <boost/shared_ptr.hpp>

//MD22 library includes
#include "MotorDriverCardConfigXML.h"
#include "MotorDriverCardImpl.h"
#include "MotorControler.h"

//Stepper Motor includes
#include "StepperMotorError.h"
#include "StepperMotorStatus.h"
#include "StepperMotorCalibrationStatus.h"

namespace mtca4u {


   
    class StepperMotorUnitsConverter {
        public:
            virtual float stepsToUnits(int steps) = 0;
            virtual int unitsToSteps(float units) = 0;
    };
    
    
     // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // StepperMotor class !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   
    
    #define DEBUG_OFF 0
    #define DEBUG_ERROR 1
    #define DEBUG_WARNING 2
    #define DEBUG_INFO 3
    #define DEBUG_DETAIL 4
    #define DEBUG_MAX_DETAIL 5

    class StepperMotor {
    
    public:

        /**
         * @brief  Constructor of the class object
         * @param  motorDriverCardDeviceName - name of the device in DMAP file
         * @param  motorDriverId - each Motor Card Driver has two independent Motor Drivers (can drive two physical motors). ID defines which motor should be represented by this class instantiation  
         * @param  motorDriverCardConfigFileName - name of configuration file
         * @return
         */
        StepperMotor(std::string motorDriverCardDeviceName, unsigned int motorDriverId, std::string motorDriverCardConfigFileName);
        
        /**
         * @brief  Destructor of the class object
         */
        ~StepperMotor();

        /**
         * @brief Sending motor to new position (blocking).
         * @param  newPosition - new position for the motor 
         * @return StepperMotorStatus - status of the motor after movement routine finish  
         * 
         * @details 
         * This method sends motor to new position. It is blocking blocking method. It will finish when motor movement will end. 
         * Function can be interrupted by using 'void stopMotor()' method.\n
         * Function automatically detects end switches conditions.\n 
         * It allows to perform movement in case NEGATIVE END SWITCH is active but target position in grater than current one.\n
         * It allows to perform movement in case POSITIVE END SWITCH is active but target position in smaller than current one.\n
         * Function returns StepperMotorStatus object which is status of the motor after movement routine finished. Should be StepperMotorStatusTypes::M_OK in case of success.
         * 
         */
        StepperMotorStatus moveToPosition(float newPosition) throw();     

        /**
         * @brief  Set new position for the motor (no blocking)
         * @param  newPosition - new position for the motor 
         * @return void
         *  
         * @details 
         * Motor behavior depends on the Autostart flag value. \n
         * When Autostart flag is set to TRUE motor will automatically start moving to desired position by calling 'void startMotor()' method.\n
         * When Autostart flag is set to FALSE motor will NOT automatically start moving to desired position. Additionally, execution of startMotor() method is necessary to start motor.\n
         * This is NO blocking method so user needs to check when motor status will change from StepperMotorStatusTypes::M_IN_MOVE to other.\n
         * Position is expressed in arbitrary units ex. mm or fs or deg or steps. Recalculation from arbitrary unit to steps is done internally according to the 'UnitsToSteps Conversion function '.\n
         * 'UnitsToSteps Conversion method ' is set by setConverionFunctions method. By default it is direct conversion: steps = 1 x units \n
         */
         void setMotorPosition(float newPosition);
        

        /**
         * @brief Return real motor position read directly from the hardware and expressed in the arbitrary units.
         * @param  
         * @return float - current, real position of motor in arbitrary units.
         * 
         * @details 
         * Return real motor position read directly from the hardware and expressed in the arbitrary units.\n
         * Calculation between steps and arbitrary units is done internally according to 'StepsToUnits Conversion method '.\n
         */     
        float getMotorPosition();

        /**
         * @brief Set position passed as parameter as current position of the motor
         * @param  newPosition - new position for the motor expressed in arbitrary units
         * @return void
         * 
         * @details 
         * Set given position as a current position of the motor.\n
         * Position is expressed in arbitrary units and internally recalculated into steps.\n
         * Can be used in example to zeroed current position.
         */
        void setCurrentMotorPositionAs(float newPosition);

        /**
         * @brief Set functions which allows convert arbitrary units to steps and steps into arbitrary units.
         * @param UnitToSteps - pointer to the function which allow to convert number of units to number of steps. Function take 'float' type as parameter and return 'int' type.
         * @param StepsToUnits - pointer to the function which allow to convert number of steps to number of units. Function take 'int' type as parameter and return 'float' type
         * 
         * @return void
         * 
         * @details 
         * By default this functions are set as: "steps = 1 * units" and "unit = 1 * steps".\n
         * To keep synchronization UnitToSteps following assumption must be fulfill:\n
         * 1) UnitToSteps(StepsToUnits(x)) must be equal to 'x'.\n
         * 2) StepsToUnits(UnitToSteps(x)) must be equal to 'x'.\n
         */ 
        //void setConversionFunctions(int (*UnitToSteps)(float), float (*StepsToUnits)(int));

        void setStepperMotorUnitsConverter(StepperMotorUnitsConverter* stepperMotorUnitsConverter);
        
        
        /**
         * @brief Start the motor 
         * @param
         * @return void
         * 
         * @details 
         * Function starts the motor. When 'Autostart' is set to FALSE it allows to move motor, but before it checks motor status.\n
         * When status is StepperMotorStatusTypes::M_OK or StepperMotorStatusTypes::M_MOTOR_IN_MOVE, target position set by setMotorPosition is passed to hardware and motor moves.\n
         * 
         */      
        void startMotor();
        
        /**
         * @brief Stop the motor 
         * @param
         * @return void
         * 
         * @details 
         * Function stop the motor in a way that real position from the motor is set as a target position.\n
         * It also interrupts all blocking functions as 'StepperMotorStatus moveToPosition(float newPosition)' or 'StepperMotorCalibrationStatus calibrateMotor()'.\n
         * Delays in communication can trigger effect that motor will move a few steps in current movement direction and later will move couple steps back, but this issue can be filtered by motor hysteresis effect.\n
         */  
        void stopMotor();

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
        void stopMotorEmergency();

        /**
         * @brief Perform calibration of the motor (blocking). 
         * @param
         * @return void
         * 
         * @details 
         * TO BE ADDED
         */ 
        StepperMotorCalibrationStatus calibrateMotor() throw();
        

        /**
         * @brief Return current calibration status.
         * @param
         * @return StepperMotorCalibrationStatus - current calibration status
         * 
         * @details 
         * Return current calibration status as StepperMotorCalibrationStatus object.\n
         * Currently following status are forseen:\n
         * 1) StepperMotorCalibrationStatusType::M_CALIBRATION_UNKNOWN - calibration status in unknown. This is default value after object creation.\n
         * 2) StepperMotorCalibrationStatusType::M_CALIBRATED - motor calibrated.\n
         * 3) StepperMotorCalibrationStatusType::M_CALIBRATED -motor not calibrated (ex. after emergency stop).\n
         * 4) StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED - motor calibration failed.\n
         * 5) StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS - motor is calibrating now.\n
         * 6) StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER - motor calibration was triggered by it was stopped by user (ex. execution of 'void stopMotor()' function).\n
         */
        StepperMotorCalibrationStatus getCalibrationStatus();

        /**
         * @brief Return current motor status.
         * @param
         * @return StepperMotorStatus - current motor status
         * 
         * @details 
         * Return current motor status as StepperMotorStatus object.\n
         * Every time it is called it checks status of the motor.
         * Currently following status are forseen:\n
         * 1) StepperMotorStatusTypes::M_OK - motor is OK and ready for new commands.\n
         * 2) StepperMotorStatusTypes::M_DISABLED - motor is disabled. No action can be done. to change this use 'void setEnable(bool enable)' method.\n
         * 3) StepperMotorStatusTypes::M_IN_MOVE -motor in in move.\n
         * 4) StepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON - hardware positive end switch has been triggered. Motor is on positive end.\n
         * 5) StepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON - hardware negative end switch has been triggered. Motor is on negative end.\n
         * 6) StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON - target position in greater than value set by 'void setMaxPositionLimit(float maxPos)' method.\n
         * 7) StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON - target position in smaller than value set by 'void setMinPositionLimit(float maxPos)' method.\n
         * 8) StepperMotorStatusTypes::M_ERROR - motor is in error state. For details about error check 'StepperMotorError getMotorError()' method.\n
         */
        StepperMotorStatus getMotorStatus() throw();
        
        StepperMotorError getMotorError() throw();
        
        
        // Speed setting
        void setMotorSpeed(float newSpeed);
        float getMotorSpeed();
        
        //autostart 
        bool getAutostart();
        void setAutostart(bool autostart);
        
        
        void setEnable(bool enable);
        bool getEnabled() const;
        //Software limits settings
        void setMaxPositionLimit(float maxPos);
        void setMinPositionLimit(float minPos);
        float getMaxPositionLimit();
        float getMinPositionLimit(); 
        
        
        //
        void setDebugLevel(unsigned int newLevel);
        unsigned int getDebugLevel();    
        void setDebugStream(std::ostream* debugStream);
        
        //end switches positions getters
        float getPositiveEndSwitchPosition() const;
        float getNegativeEndSwitchPosition() const;

        int recalculateUnitsToSteps(float units);
        float recalculateStepsToUnits(int steps);
        
    private: // methods

        void determineMotorStatusAndError() throw();
        float truncateMotorPosition(float newPosition);
    
    private: // fields
        std::string _motorDriverCardDeviceName;
        unsigned int _motorDriverId;
        std::string _motorDriverCardConfigFileName;
                
        //pointers to objects which controls the physical driver, we will use boost::shared_ptr
        MotorDriverCardConfig _motorDriverCardConfig;

        
        boost::shared_ptr<MotorDriverCard> _motorDriverCardPtr;
        //MotorDriverCard* _motorDriverCardPtr;
        
        //boost::shared_ptr<MotorControler> _motorControler;
        MotorControler* _motorControler;

        // position
        int _currentPostionsInSteps;
        float _currentPostionsInUnits;
        
        int _targetPositionInSteps;
        float _targetPositionInUnits;
        
        
        //position limts
        float _maxPositionLimit;
        float _minPositionLimit;
               
        //pointer to the units converter class
        StepperMotorUnitsConverter* _stepperMotorUnitsConverter;
        
        
        //status and error
        StepperMotorStatus _motorStatus;
        StepperMotorError  _motorError;
        StepperMotorCalibrationStatus _motorCalibrationStatus;
        
        //Autostart
        bool _autostartFlag;
        
        //Stop motor flag for blocking functions
        bool _stopMotorForBlocking;
        
        bool _stopMotorCalibration;
        
        //calibration
        int _calibNegativeEndSwitchInSteps;
        int _calibNegativeEndSwitchInUnits;
        
        int _calibPositiveEndSwitchInSteps;
        int _calibPositiveEndSwitchInUnits;
        
        unsigned int _debugLevel;
        std::ostream* _debugStream;
        
    };

} //namespace mtca4u
#endif	/* MTCA4U_STEPPER_MOTOR_H */

