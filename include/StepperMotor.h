/* 
 * File:   StepperMotor.h
 * Author: tkozak
 *
 * Created on April 8, 2014, 11:49 AM
 */
#include <string>
#include "MotorDriverCardImpl.h"
#include "MotorControler.h"
#include <boost/shared_ptr.hpp>
#include "MotorDriverCardConfigXML.h"

#ifndef MTCA4U_STEPPER_MOTOR_H
#define	MTCA4U_STEPPER_MOTOR_H
namespace mtca4u {

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // StepperMotorError structure !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     class StepperMotorError {
     
     public:
        int id;
        std::string name;
        
        //default constructor
        StepperMotorError() : id(0), name("No error") {
        };
        
        //constructor with params
        StepperMotorError(int itemId, std::string itemName) : id(itemId), name(itemName) {
        };
        
        //copy constructor
        StepperMotorError(const StepperMotorError &error) : id(error.id), name(error.name){            
        }
        
        bool operator==(StepperMotorError const& right) const {
            if (id != right.id) {
                return false;
            }

            return true;
        };
    };
    
    class StepperMotorErrorTypes {
    
    public:
        static struct StepperMotorError MOTOR_NO_ERROR;
        static struct StepperMotorError MOTOR_BOTH_ENDSWICHTED_ON;
        static struct StepperMotorError MOTOR_COMMUNICATION_LOST;
    };


    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // StepperMotorStatusItem structure !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    class StepperMotorStatus {
    
    public:
        int id;
        std::string name;

        StepperMotorStatus() : id(0), name("OK") {
        };
        
        StepperMotorStatus(int itemId, std::string itemName) : id(itemId), name(itemName) {
        };

        //copy constructor
        StepperMotorStatus(const StepperMotorStatus &status) : id(status.id), name(status.name){            
        }
        
        bool operator==(StepperMotorStatus const& right) const {
            if (id != right.id) {
                return false;
            }

            return true;
        };

        bool operator!=(StepperMotorStatus const& right) const {
            if (id == right.id) {
                return false;
            }

            return true;
        };
        
        friend std::ostream &operator<<(std::ostream &out, const StepperMotorStatus &status) {
            out << status.name <<"("<<status.id<<")";
            return out;
        }
    };
    
    class StepperMotorStatusTypes {
    public:
        static StepperMotorStatus MOTOR_OK;
        static StepperMotorStatus MOTOR_IN_MOVE;
        static StepperMotorStatus MOTOR_PLUS_END_SWITCHED_ON;
        static StepperMotorStatus MOTOR_MINUS_END_SWITCHED_ON;
        static StepperMotorStatus MOTOR_SOFT_PLUS_END_SWITCHED_ON;
        static StepperMotorStatus MOTOR_SOFT_MINUS_END_SWITCHED_ON;
        static StepperMotorStatus MOTOR_IN_ERROR;
        static StepperMotorStatus MOTOR_DISABLED;
        static StepperMotorStatus MOTOR_CONFIGURATION_ERROR;

    };
    // END OF StepperMotorStatusItem structure !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    
    
     class StepperMotorCalibrationStatusType {
     public:
        static int MOTOR_CALIBRATED;
        static int MOTOR_NOT_CALIBRATED;
        static int MOTOR_CALIBRATION_UNKNOWN;
        static int MOTOR_CALIBRATION_FAILED;
        static int MOTOR_CALIBRATION_IN_PROGRESS;
    };   
    
   
    
    
     // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // StepperMotor class !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   
    
    
    class StepperMotor {
    
    public:
        /*! Constructor of the class object
         * \param motorDriverCardDeviceName - name of the device in MAP file which have Motor Card Driver
         * \param motorDriverId - each Motor Card Driver has two independent Motor Drivers (can drive two physical motors). ID define which motor should be represented by this class instantiation  
         * \param motorDriverCardConfigFileName - name of configuration file 
         * \return
         */
        
        StepperMotor(std::string motorDriverCardDeviceName, unsigned int motorDriverId, std::string motorDriverCardConfigFileName);
        /*! Destructor of the class
         */
        ~StepperMotor();
    

        /*! Set new position for the motor. 
         * Motor behavior depends on the Autostart flag value. 
         * When Autostart flag is set to TRUE motor will automatically start moving to desired position.
         * When Autostart flag is set to FALSE motor will NOT automatically start moving to desired position. Additionally, execution of startMotor() method is necessary to start motor.
         * Position is expressed in arbitrary units ex. mm or fs or deg or steps. Recalculation from arbitrary unit to steps is done internally according to the 'UnitsToSteps Conversion function '.
         * 'UnitsToSteps Conversion function ' is set by setConverionFunctions method. By default it is direct conversion: steps = 1 x units
         * \param newPosition - new position for the motor 
         * \return void
         */
        void setMotorPosition(float newPosition);
        
         /*! Return real motor position read directly from the hardware expressed in the arbitrary units.
          *  Calculation between steps and arbitrary units is done internally according to 'StepsToUnits Conversion function '.
          * \param 
          * \return float - current, real position of motor in arbitrary units.
          */       
        float getMotorPosition();

        /*! Set functions which allows convert arbitrary units to steps and steps into arbitrary units.
         *  By default this functions are set as: "steps = 1 * units" and "unit = 1 * steps"
         *  
         * \param UnitToSteps - pointer to the function which allow to convert number of units to number of steps. Function take 'float' type as parameter and return 'int' type
         * \param StepsToUnits - pointer to the function which allow to convert number of steps to number of units. Function take 'int' type as parameter and return 'float' type
         * \return
         */       
        void setConversionFunctions(unsigned int (*UnitToSteps)(float), float (*StepsToUnits)(int));
        
        // Motors command        
        void startMotor();
        void stopMotor();
        void emergencyStopMotor();
        void calibrateMotor();
        
        
        // Speed setting
        void setMotorSpeed(float newSpeed);
        float getMotorSpeed();
        
        //Software limits settings
        void setMaxPositionLimit(float maxPos);
        void setMinPositionLimit(float minPos);
        float getMaxPositionLimit();
        float getMinPositionLimit();      
        
        //motor statuses
        StepperMotorStatus getMotorStatus();
        StepperMotorError getMotorError();
        
        //autostart 
        bool getAutostart();
        void setAutostart(bool autostart);
        
        
        void setEnable(bool enable);
        
    private: // methods
        int recalculateUnitsToSteps(float units);
        float recalculateStepsToUnits(int steps);
        void determineMotorStatusAndError();
    
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
        unsigned int _currentPostionsInSteps;
        float _currentPostionsInUnits;
        
        unsigned int _setPositionInSteps;
        float _setPositionInUnits;
        
        
        //position limts
        float _maxPositionLimit;
        float _minPositionLimit;
        
        //pointers to conversion functions
        unsigned int   (*unitsToSteps) (float);
        float (*stepsToUnits) (int);
        
        //status and error
        StepperMotorStatus _motorStatus;
        StepperMotorError  _motorError;
        int _motorCalibrationStatus;
        
        //Autostart
        bool _autostartFlag;
        
        


 
    };

} //namespace mtca4u
#endif	/* MTCA4U_STEPPER_MOTOR_H */

