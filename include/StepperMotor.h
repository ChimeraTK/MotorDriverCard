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
#include <boost/thread.hpp>

//MD22 library includes
#include "MotorDriverCardConfigXML.h"
#include "XMLException.h"
#include "MotorDriverCardImpl.h"
#include "MotorControler.h"
#include "MotorDriverException.h"

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
        virtual StepperMotorStatus moveToPosition(float newPosition) throw();     

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
        virtual void setCurrentMotorPositionAs(float newPosition);

        /**
         * @brief Set a pointer to StepperMotorUnitsConverter object.
         * @param UnitToSteps - pointer to the function which allow to convert number of units to number of steps. Function take 'float' type as parameter and return 'int' type.
         * @param StepsToUnits - pointer to the function which allow to convert number of steps to number of units. Function take 'int' type as parameter and return 'float' type
         * 
         * @return void
         * 
         * @details 
         *  Set a pointer to StepperMotorUnitsConverter object. The StepperMotorUnitsConverter is an interface (abstract class) which
         *  provides two methods which are converting steps to arbitrary units and vice-versa. The user need to create its own class which inherits from
         *  StepperMotorUnitsConverter and implements the: 'float stepsToUnits(int steps)' and 'int unitsToSteps(float units)' methods.
         */ 
        void setStepperMotorUnitsConverter(StepperMotorUnitsConverter* stepperMotorUnitsConverter);
        
        
        /**
         * @brief Start the motor 
         * @param
         * @return void
         * 
         * @details 
         * Function starts the motor. When 'Autostart' is set to FALSE it allows to move motor.\n
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
        virtual StepperMotorCalibrationStatus calibrateMotor() throw();
        

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
         * 7) StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE - motor calibration is not possible because motor doesn't have physical end switches.\n
         */
        StepperMotorCalibrationStatus getCalibrationStatus();

        /**
         * @brief Return current motor status.
         * @param
         * @return StepperMotorStatus - current motor status object which contains of status code and string which describes it in human readable way
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
        

        /**
         * @brief Return current motor status.
         * @param
         * @return StepperMotorError - current motor error object which contains of error code and string which describes it in human readable way
         * 
         * @details 
         * Return current motor status as StepperMotorStatus object.\n
         * Every time it is called it checks status of the motor.
         * Currently following status are forseen:\n
         * 1) StepperMotorErrorTypes::M_NO_ERROR - motor is OK and ready for new commands.\n
         * 2) StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX - error in configuration of software position limits\n
         * 3) StepperMotorErrorTypes::M_BOTCH_END_SWITCH_ON -both end switch active. Hardware is missing or in error state.\n
         * 4) StepperMotorErrorTypes::M_COMMUNICATION_LOST - errors and problems with communication with firmware\n
         * 5) StepperMotorErrorTypes::M_NO_REACION_ON_COMMAND - hardware is no reacting on command.\n
         * */

        StepperMotorError getMotorError() throw ();
        
        
        // Speed setting
        /**
         * @brief Set the new speed of the motor - NOT IMPLEMETNED YET 
         * @param
         * @return void
         * 
         * @details 
         * TO BE ADDED
         */
        void setMotorSpeed(float newSpeed);
        
        
        /**
         * @brief Get current speed of motor - NOT IMPLEMETNED YET 
         * @param
         * @return void
         * 
         * @details 
         * TO BE ADDED
         */
        float getMotorSpeed();

        //autostart


        /**
         * @brief Sets of Autostart flag
         * @param bool autostart - new value for Autostart flag
         * @return void
         * 
         * @details 
         * When Autostart flag is set to TRUE motor will automatically start moving to desired position by internally calling 'void startMotor()' method.\n
         * When Autostart flag is set to FALSE motor will NOT automatically start moving to desired position. Additionally, execution of startMotor() method is necessary to start motor.\n
         * 
         */
        void setAutostart(bool autostart);

        /**
         * @brief Get the Autostart flag.
         * @param
         * @return bool - Current autostart flag value
         * 
         * @details 
         * When Autostart flag is set to TRUE motor will automatically start moving to desired position by internally calling 'void startMotor()' method.\n
         * When Autostart flag is set to FALSE motor will NOT automatically start moving to desired position. Additionally, execution of startMotor() method is necessary to start motor.\n
         * 
         */
        bool getAutostart();
        
        
        /**
         * @brief Set the Enable flag.
         * @param bool enable - New value for the Enabled flag
         * @return
         * 
         * @details 
         * Motor can be disabled or enabled. To work with the motor user need to enable it. When motor disabled it stops moving immediately. 
         */
        void setEnable(bool enable);

        /**
         * @brief Get the Enable flag.
         * @param
         * @return bool - Current Enabled flag value
         * 
         * @details 
         * Motor can be disabled or enabled. To work with the motor user need to enable it. When motor disabled it stops moving immediately. 
         */
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
        
        int recalculateUnitsToSteps(float units);
        float recalculateStepsToUnits(int steps);
        
        //
        void setSoftwareLimitsEnabled(bool newVal);
        bool getSoftwareLimitsEnabled() const;
        
    protected: // methods

        virtual void determineMotorStatusAndError() throw();
        float truncateMotorPosition(float newPosition);
    
    protected: // fields
        std::string _motorDriverCardDeviceName;
        unsigned int _motorDriverId;
                
        //pointers to objects which controls the physical driver, we will use boost::shared_ptr
	boost::shared_ptr<MotorDriverCard> _motorDriverCard;
	boost::shared_ptr<MotorControler> _motorControler;

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
                
        unsigned int _debugLevel;
        std::ostream* _debugStream;
        
        bool _softwareLimitsEnabled;
        
        
        static boost::mutex mutex;
        
    };

} //namespace mtca4u
#endif	/* MTCA4U_STEPPER_MOTOR_H */

