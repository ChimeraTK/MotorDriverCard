/* 
 * File:   StepperMotor.h
 * Author: tkozak
 *
 * Created on April 8, 2014, 11:49 AM
 */


#ifndef MTCA4U_STEPPER_MOTOR_H
#define	MTCA4U_STEPPER_MOTOR_H

#include <string>
#include <atomic>
#include <thread>
#include <boost/shared_ptr.hpp> // TODO Move to std lib
#include <memory>
#include <boost/thread.hpp>

//MD22 library includes
#include "MotorDriverCardConfigXML.h"
#include "MotorDriverCard.h"
#include "MotorControler.h"
#include "MotorDriverException.h"

//Stepper Motor includes
#include "StepperMotorError.h"
#include "StepperMotorStatus.h"
#include "StepperMotorCalibrationStatus.h"
#include "StateMachine.h"
#include "Logger.h"

namespace ChimeraTK {
  using namespace mtca4u;
  /**
   * @class StepperMotorUnitsConverter
   * @details Abstract class which creates interface. It contains two abstract methods which should be implemented in derived class. \n
   *          Methods allows to recalculate steps to arbitrary units and arbitrary units into steps.
   */
  class StepperMotorUnitsConverter {
  public:
    virtual float stepsToUnits(int steps) = 0;
    virtual int unitsToSteps(float units) = 0;
    virtual ~StepperMotorUnitsConverter(){}
  };
  /**
   * @class StepperMotorUnitsConveterTrivia
   * @details Trivial implementation of StepperMotorUnitsConverter. 1:1 conversion between units and steps
   *          Used as default conversion if nothing else is specified
   */
  class StepperMotorUnitsConverterTrivia : public StepperMotorUnitsConverter{
  public:
    StepperMotorUnitsConverterTrivia(){

    };
    virtual float stepsToUnits(int steps){
      return static_cast<float> (steps);
    };
    virtual int unitsToSteps(float units){
      return static_cast<int> (units);
    }
  };
}

namespace mtca4u{

  using namespace ChimeraTK;
  /**
   * @class StepperMotorStatusAndError
   * @details Container class for status and error of StepperMotor. These two object describes the state of motor and create a pair.
   */
  class StepperMotorStatusAndError {
  public:
    StepperMotorStatus status;
    StepperMotorError error;

    StepperMotorStatusAndError(StepperMotorStatus statusParam = StepperMotorStatus(),
                               StepperMotorError errorParam  = StepperMotorError()) :
                        	 status(statusParam), error(errorParam){
    }
  };
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // StepperMotor class !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  class StepperMotor {

  public:

    /**
     * @brief  Constructor of the class object
     * @param  motorDriverCardDeviceName Name of the device in DMAP file
     * @param  moduleName Name of the module in the map file (there might be more than one MD22 per device/ FMC carrier).
     * @param  motorDriverId Each Motor Card Driver has two independent Motor Drivers (can drive two physical motors). ID defines which motor should be represented by this class instantiation
     * @param  motorDriverCardConfigFileName Name of configuration file
     * @return
     */
    StepperMotor(std::string const & motorDriverCardDeviceName, std::string const & moduleName, unsigned int motorDriverId, std::string motorDriverCardConfigFileName);

    /**
     * @brief  Destructor of the class object
     */
    virtual ~StepperMotor();

    /**
     * @brief Sending motor to new position (blocking).
     * @param  newPosition - new position for the motor
     * @return StepperMotorStatusAndError - status of the motor after movement routine finish
     *
     * @details
     * This method sends motor to new position. It is blocking blocking method. It will finish when motor movement will end with success or error.
     * Function can be interrupted by using 'void stopMotor()' method.\n
     * Function returns StepperMotorStatusAndError object which is status of the motor after movement routine finished. Should be StepperMotorStatusTypes::M_OK in case of success.
     */
    StepperMotorStatusAndError moveToPosition(float newPosition);

    /**
     * @brief Sending motor to new position (blocking)
     * @param newPositionInSteps - new position in steps for the motor
     * @return StepperMotorStatusAndError - status of the motor after movement routine finish
     * @details see method moveToPosition(float)
     */

    StepperMotorStatusAndError moveToPositionInSteps(int newPositionInSteps);

    /**
     * @brief  Set new position for the motor (no blocking)
     * @param  newPosition - new position for the motor
     * @return void
     *
     * @details
     * Motor behavior depends on the Autostart flag value. \n
     * When Autostart flag is set to TRUE motor will automatically start moving to desired position by calling 'void start()' method.\n
     * When Autostart flag is set to FALSE motor will NOT automatically start moving to desired position. Additionally, execution of start() method is necessary to start motor.\n
     * This is NO blocking method so user needs to monitor motor status.\n
     * Position is expressed in arbitrary units ex. mm or fs or deg or steps. Recalculation from arbitrary unit to steps is done by object of StepperMotorUnitsConverter.\n
     * StepperMotorUnitsConverter can be set by setStepperMotorUnitsConverter method. When not set conversion coefficient is 1 (steps = 1 x units). \n
     */
    void setTargetPosition(float newPosition); // old name: setMotorPosition

    /**
     * @brief Set new position for the motor in steps (no blocking)
     * @param newPositionInSteps - new position for the motor in steps
     * @return void
     *
     * @details see method setTargetPosition(float)
     */

    void setTargetPositionInSteps(int newPositionInSteps);

    /**
     * Return target motor position in the arbitrary units.
     * @return float - target position of motor in arbitrary units.
     */  
    float getTargetPosition();

    /**
     * Return target motor position in steps
     * @return int - target position of motor in steps
     */

    int getTargetPositionInSteps();

    /**
     * Return real motor position read directly from the hardware and expressed in the arbitrary units.
     * Calculation between steps and arbitrary units is done internally.\n
     * @return float - current, real position of motor in arbitrary units.
     */
    float getCurrentPosition(); //old name: getMotorPosition

    /**
     * Return real motor position read directly from the hardware and expressed in steps.
     * No calculation is involved.
     * @return int - current, real position in number of steps. The number of steps can be either negative or positive,
     * depending on the position with respect to the zero.
     */
    int getCurrentPositionInSteps();
    /**
     * @brief Set position passed as parameter as current position of the motor
     * @param  newPosition - new position for the motor expressed in arbitrary units

     * @details
     * Set given position as a current position of the motor.\n
     * Position is expressed in arbitrary units and internally recalculated into steps.\n
     * Can be used in example to zeroed current position.
     */
    virtual void setCurrentPositionAs(float newPosition); // old name: setCurrentMotorPositionAs

    /**
     * @brief Set position passed as parameter as current position of the motor
     * @param newPositionSteps  - new position for the motor expressed in steps
     *
     * @details see function setCurrentPositionAs(float newPosition)
     */

    virtual void setCurrentPositionInStepsAs(int newPositionSteps);

    /**
     * @brief Set a pointer to StepperMotorUnitsConverter object.
     * @param stepperMotorUnitsConverter - shared pointer to the converter object.
     *
     * @details
     *  Set a pointer to StepperMotorUnitsConverter object. The StepperMotorUnitsConverter is an interface (abstract class) which
     *  provides two methods which are converting steps to arbitrary units and vice-versa. The user need to create its own class which inherits from
     *  StepperMotorUnitsConverter and implements the: 'float stepsToUnits(int steps)' and 'int unitsToSteps(float units)' methods.
     */

    void setStepperMotorUnitsConverter(boost::shared_ptr<StepperMotorUnitsConverter> stepperMotorUnitsConverter);

    /**
     * @brief set the unit converter to default one, with a 1:1 conversion factor.
     */

    void setStepperMotorUnitsConverterToDefault();

    /**
     * @brief Start the motor
     *
     * @details
     * Function starts the motor. When 'Autostart' is set to FALSE it allows to move motor.\n
     *
     */
    void start();

    /**
     * @brief Stop the motor
     *
     * @details
     * Function stop the motor in a way that real position from the motor is set as a target position.\n
     * It also interrupts all blocking functions as 'StepperMotorStatusAndError moveToPosition(float newPosition)'. \n
     * Delays in communication can trigger effect that motor will move a few steps in current movement direction and later will move couple steps back, but this issue can be filtered by motor hysteresis effect.\n
     */
    virtual void stop();

    /**
     * @brief Returns True if the motor is moving and false if at
     * standstill. This command is reliable as long as the motor is not
     * stalled.
     */
    bool isMoving();

    /**
     * @brief Emergency stop the motor by disabling driver.
     *
     * @details
     * Function stops the motor by disabling driver. It means that it is done in the fastest possible way.\n
     * Function internally calls 'void stopMotor()' method so all blocking functions will be interrupted.\n
     */
    void emergencyStop(); // old name: stopMotorEmergency

    /**
     * @brief Perform calibration of the motor (blocking).
     *
     * @details
     * TO BE ADDED
     */
    virtual StepperMotorCalibrationStatus calibrateMotor();

    /**
     * @brief Return current calibration status.
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
     * @brief Return object which contains current status and error.
     * @return StepperMotorStatusAndError - object which contains of status and error code
     *
     * @details
     * Return current motor status and error as StepperMotorStatusAndError object.\n
     * It give possibility to check what was an error type when status is set to StepperMotorStatusTypes::M_ERROR.
     * Status and error create pair which describes current state of the object.
     * Every time it is called it checks status of the motor.
     *
     * Currently following status are foreseen:\n
     * 1) StepperMotorStatusTypes::M_OK - motor is OK and ready for new commands.\n
     * 2) StepperMotorStatusTypes::M_DISABLED - motor is disabled. No action can be done. to change this use 'void setEnabled(bool enable)' method.\n
     * 3) StepperMotorStatusTypes::M_IN_MOVE -motor in in move.\n
     * 4) StepperMotorStatusTypes::M_NOT_IN_POSITION - target position in different than current one and motor is not moving \n
     * 5) StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON - target position in greater than value set by 'void setMaxPositionLimit(float maxPos)' method.\n
     * 6) StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON - target position in smaller than value set by 'void setMinPositionLimit(float maxPos)' method.\n
     * 7) StepperMotorStatusTypes::M_ERROR - motor is in error state. For details about error check 'StepperMotorError getError()' method.\n
     *
     * Currently following errors are foreseen:\n
     * 1) StepperMotorErrorTypes::M_NO_ERROR - motor is OK and ready for new commands.\n
     * 2) StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX - error in configuration of software position limits\n
     * 3) StepperMotorErrorTypes::M_COMMUNICATION_LOST - errors and problems with communication with firmware\n
     * 4) StepperMotorErrorTypes::M_NO_REACION_ON_COMMAND - hardware is no reacting on command.\n
     * 5) StepperMotorErrorTypes::M_HARDWARE_NOT_CONNECTED - hardware is not connected.\n
     * */
    StepperMotorStatusAndError getStatusAndError();

    /**
     * @brief Sets the maximum speed the motor may operate at. The
     * method may not set the exact desired speed limit, but a value below
     * the desired limit, as determined by the operating parameters.
     *
     * @param microStepsPerSec The desired maximum motor speed during
     *                         operation; unit is microsteps per second
     *
     * @return Returns the motor speed that was actually set. This speed may not
     * be what the user requested, but a value closest to it (but not
     * exceeding the provided limit) as permitted by the motor controller
     * parameters
     */
    double setUserSpeedLimit(double microStepsPerSec);

    /**
     * @brief returns the current motor speed limit configured by the user
     *
     */
    double getUserSpeedLimit();

    /**
     * @brief Returns the maximum speed the motor is capable of achieving. Speed
     * is expressed in microsteps per second
     */

//    unsigned int getMicroStepCount(); //todo to be removed

    double getMaxSpeedCapability();

    /**
     * @brief Sets the current the motor driver ic provies to the motor.
     * Command may not set the exact value of current requested. In this
     * case the driver would use a current that will not exceed requested
     * value
     *
     * @param currentInAmps Desired driving current in Amps.
     *
     * @return Current used to drive the motor. This will be in the range
     *         [05625A, 1.8A] and will be closest to the user requested
     *         value (but not exceed it).
     */
    double setUserCurrentLimit(double currentInAmps);

    /**
     * @brief Returns the current used to drive the motor
     */
    double getUserCurrentLimit();

    /**
     * @brief Returns the maximum value of current that this motor can be
     * driven saftely
     */
    double getSafeCurrentLimit();

    /**
     * @brief Sets of Autostart flag
     * @param autostart - new value for Autostart flag
     *
     * @details
     * When Autostart flag is set to TRUE motor will automatically start moving to desired position by internally calling 'void start()' method.\n
     * When Autostart flag is set to FALSE motor will NOT automatically start moving to desired position. Additionally, execution of start() method is necessary to start motor.\n
     *
     */
    void setAutostart(bool autostart);

    /**
     * @brief Get the Autostart flag.
     * @return bool - Current autostart flag value
     *
     * @details
     * When Autostart flag is set to TRUE motor will automatically start moving to desired position by internally calling 'void start()' method.\n
     * When Autostart flag is set to FALSE motor will NOT automatically start moving to desired position. Additionally, execution of start() method is necessary to start motor.\n
     *
     */
    bool getAutostart();

    /**
     * @brief Enables the motor.
     * @param enable - New value for the Enabled flag
     *
     * @details
     * Motor can be disabled or enabled. To work with the motor user need to enable it. When motor disabled it stops moving immediately.
     */
    void setEnabled(bool enable);

    /**
     * @brief Get the enable status.
     * @return bool - Current Enabled flag value
     *
     * @details
     * Motor can be disabled or enabled. To work with the motor user need to enable it. When motor disabled it stops moving immediately.
     */
    bool getEnabled() const;

    /**
     * @brief Set soft limit on the maximum position of the motor expressed in arbitrary units
     * @param maxPos - Greatest position which motor will be able to reach
     *
     * @details
     * Motor can have not physical end switches which would limits range of possible position. For the reason software limits has been introduced.
     */
    void setMaxPositionLimit(float maxPos);

    /**
     * @brief Set soft limit on the maximum position of the motor expressed in steps
     * @param maxPosInSteps - Greatest position which motor will be able to reach
     */
    void setMaxPositionLimitInSteps(int maxPosInSteps);

    /**
     * @brief Set soft limit on the minimum position of the motor expressed in arbitrary units
     * @param minPos - Smallest position which motor will be able to reach
     *
     * @details
     * Motor can have not physical end switches which would limits range of possible position. For the reason software limits has been introduced.
     */
    void setMinPositionLimit(float minPos);

    /**
     * @brief Set soft limit on the minimum position of the motor expressed in steps
     * @param minPosInSteps - Smallest position which motor will be able to reach
     */
    void setMinPositionLimitInSteps(int minPosInSteps);

    /**
     * @brief Set soft limit enabled or disabled.
     * @param enabled - When true soft limits will be checked and validated, in false they will not
     *
     * @details
     *
     */
    void setSoftwareLimitsEnabled(bool enabled);

    /**
     * @brief Get current soft limit enabled flag.
     * @return bool - When true soft limits will be checked and validated, in false they will not
     *
     * @details
     */
    bool getSoftwareLimitsEnabled() const;

    /**
     * @brief Returns soft limit on the maximum position of the motor expressed in arbitrary units
     * @return float - Maximum position which motor is able to reach
     *
     * @details
     * Motor can have not physical end switches which would limits range of possible position. For the reason software limits has been introduced.
     */
    float getMaxPositionLimit();

    /**
     * @brief Returns soft limit on the maximum position of the motor expressed in steps
     * @return float - Maximum position which motor is able to reach
     */
    int getMaxPositionLimitInSteps();

    /**
     * @brief Returns soft limit on the minimum position of the motor expressed in arbitrary units
     * @return float - Minimum position which motor is able to reach
     *
     * @details
     * Motor can have not physical end switches which would limits range of possible position. For the reason software limits has been introduced.
     */
    float getMinPositionLimit();

    /**
     * @brief Returns soft limit on the minimum position of the motor expressed in steps
     * @return float - Minimum position which motor is able to reach
     */
    int getMinPositionLimitInSteps();

    /**
     * @brief Recalculate arbitrary units into steps
     * @param units Arbitrary units in which position is expressed
     * @return int - steps value which corresponds to given as parameter value of arbitrary units.
     *
     * @details
     *
     */
    int recalculateUnitsToSteps(float units);

    /**
     * @brief Recalculate arbitrary units into steps
     * @param steps Number of step to be converted
     * @return float - Calculated units which correspond to the number of steps
     *
     * @details
     *
     */
    float recalculateStepsToUnits(int steps);

    /**
     * @brief Set log level of the motor
     * @param newLevel - new level of log
     *
     * @details
     * Set the log level for the StepperMotor object.\n
     * Log levels are:\n
     * Logger::LogLevel NO_LOGGING\n
     * Logger::LogLevel ERROR;\n
     * Logger::LogLevel WARNING;\n
     * Logger::LogLevel INFO;\n
     * Logger::LogLevel DETAIL;\n
     * Logger::LogLevel FULL_DETAIL;\n
     */
    void setLogLevel(Logger::LogLevel newLevel);

    /**
     * @brief Get current log level of the motor
     * @return Logger::LogLevel - current log level
     *
     * @details
     */
    Logger::LogLevel getLogLevel();

    void enableFullStepping(bool enable = true);
    bool isFullStepping();

  protected: // methods

    StepperMotorStatusAndError determineMotorStatusAndError();
    int truncateMotorPosition(int newPositionInSteps);

  protected: // fields
    std::string _motorDriverCardDeviceName;
    unsigned int _motorDriverId;

    //pointers to objects which controls the physical driver, we will use boost::shared_ptr
    boost::shared_ptr<MotorDriverCard> _motorDriverCard;
    boost::shared_ptr<MotorControler> _motorControler;

    // current position
    //int _currentPostionsInSteps;
    //float _currentPostionsInUnits;
    // current target
    boost::shared_ptr<StepperMotorUnitsConverter> _stepperMotorUnitsConverter;
    int _targetPositionInSteps;
    //float _targetPositionInUnits;
    //position limits
    //float _maxPositionLimit;
    int   _maxPositionLimitInSteps;
    //float _minPositionLimit;
    int   _minPositionLimitInSteps;
    //pointer to the units converter class
    //Autostart
    bool _autostartFlag;
    //Stop motor flag for blocking functions
    bool _stopMotorForBlocking;
    //soft limits enabled flag
    bool _softwareLimitsEnabled;
    //status and error
    StepperMotorError  _motorError;
    StepperMotorCalibrationStatus _motorCalibrationStatus;
    StepperMotorStatus _motorStatus;
    //flag which indicate error in blocking function
    bool _blockingFunctionActive;
    Logger _logger;
    mutable boost::mutex _mutex;
  };
} //namespace mtca4u

namespace ChimeraTK{

  class StepperMotor{
  public:
    /**
     * @brief  Constructor of the class object
     * @param  motorDriverCardDeviceName Name of the device in DMAP file
     * @param  moduleName Name of the module in the map file (there might be more than one MD22 per device/ FMC carrier).
     * @param  motorDriverId Each Motor Card Driver has two independent Motor Drivers (can drive two physical motors). ID defines which motor should be represented by this class instantiation
     * @param  motorDriverCardConfigFileName Name of configuration file
     * @param  motorUnitsConverter A converter between motor steps and user unit. Based on the abstract class StepperMotorUnitsConverter. Defaults to a 1:1 converter between units and steps.
     * @param  encoderUnitsConverter A converter between encoder steps and user unit. Based on the abstract class StepperMotorUnitsConverter. Defaults to a 1:1 converter between units and steps.
     * @return
     */
    StepperMotor(std::string const & motorDriverCardDeviceName,
                 std::string const & moduleName,
                 unsigned int motorDriverId,
                 std::string motorDriverCardConfigFileName,
                 std::shared_ptr<StepperMotorUnitsConverter> motorUnitsConverter = std::make_shared<StepperMotorUnitsConverterTrivia>(),
                 std::shared_ptr<StepperMotorUnitsConverter> encoderUnitsConverter = std::make_shared<StepperMotorUnitsConverterTrivia>());

    /**
     * @brief  Destructor of the class object
     */
    virtual ~StepperMotor();

    /**
     * @brief in unit
     */
    virtual void moveToPosition(float newPosition);

    /**
     * @brief in steps
     */

    virtual void moveToPositionInSteps(int newPositionInSteps);

    /**
     * @ brief move the motor a delta from the current position
     * @param delta is given in unit
     */
    virtual void moveRelative(float delta);

    /**
     * @brief move the motor a delta from the current position
     * @param delta is given in steps
     */
    virtual void moveRelativeInSteps(int delta);

    /**
     * @brief Sets the target position in arbitrary units (according to the scaling).
     *        If the autostart flag is set to true, this will initiate movement, otherwise movement needs to be triggered
     *        by calling start().
     */
    virtual void setTargetPosition(float newPosition);

    /**
     * @brief Sets the target position in steps.
     *        If the autostart flag is set to true, this will initiate movement, otherwise movement needs to be triggered
     *        by calling start().
     */
    virtual void setTargetPositionInSteps(int newPositionInSteps);

    /**
     * @brief Initiates movement of the motor. This command is in effect, if
     *        the autostart flag is set to false.
     */
    virtual void start();

    /**
     * @brief interrupt the current action and return the motor to idle
     *
     */
    virtual void stop();

    /**
     * @brief interrupt the current action, return the motor to idle and disable it
     */
    virtual void emergencyStop();

    /**
     * @brief It transforms units in steps using the internal converter
     * @param units Value in unit to be converted in steps
     * @return Value in steps
     */
    virtual int recalculateUnitsInSteps(float units);

    /**
     * @brief it transforms steps in units using the internal converter
     * @param steps Value in steps to be coverted in units
     * @return Value in units
     */
    virtual float recalculateStepsInUnits(int steps);

    /**
     * @brief enable/disable software position limits
     */
    virtual void setSoftwareLimitsEnabled(bool enabled);

    /**
     * @brief get software position limits enabled flag
     */
    virtual bool getSoftwareLimitsEnabled();

    /**
     * @brief set maximum software limit in units
     * @param maxPos maximum limit
     */
    virtual void setMaxPositionLimit(float maxPos);

    /**
     * @brief set maximum software limit in steps
     * @param maxPos maximum limit
     */
    virtual void setMaxPositionLimitInSteps(int maxPos);

    /**
     * @brief set minimum software limit in units
     * @param minPos minimum limit
     */
    virtual void setMinPositionLimit(float minPos);

    /**
     * @brief set minimum software limit in steps
     * @param minPos maximum limit
     */
    virtual void setMinPositionLimitInSteps(int minPos);

    /**
     * @brief get maximal software position limit in units
     */
    virtual float getMaxPositionLimit();

    /**
     * @brief get maximal software position limit in steps
     */
    virtual int getMaxPositionLimitInSteps();

    /**
     * @brief get minimal software position limit in units
     */
    virtual float getMinPositionLimit();

    /**
     * @brief get minimal software position limit in steps
     */
    virtual int getMinPositionLimitInSteps();

    /**
     * @brief set actual position in units of the motor respect to some reference
     * @param actualPositionInUnits In order to use the motor an absolute scale must be defined.
     * This can be done defining the position of the motor respect to an external reference (actual position).
     * In addition to that, the conversion between steps and unit must provided through the method setStepperMotorUnitsConverter.
     */
    virtual void setActualPosition(float actualPositionInUnits);

    /**
     * @brief set actual position in steps of the motor respect to some reference
     * @param actualPositionInSteps In order to use the motor an absolute scale must be defined.
     * This can be done defining the position of the motor respect to an external reference (actual position).
     */
    virtual void setActualPositionInSteps(int actualPositionInSteps);

    /**@brief translate the reference axis of the motor. This operation will translate also the software limits
     * @param translationInSteps translation value in steps
     */

    virtual void translateAxisInSteps(int translationInSteps);

    /**@brief translate the reference axis of the motor. This operation will translate also the software limits
     * @param translationInUnits translation value in unit
     */

    virtual void translateAxis(float translationInUnits);

    /**
     * @brief get current position of the motor in units
     */
    virtual float getCurrentPosition();

    /**
     * @brief get current position of the motor in steps
     */
    virtual int getCurrentPositionInSteps();

    /**
     * @brief Get the current position from encoder in user-defined units,
     *        according to the encoderUnitsConverter defined for the instance.
     */
    virtual double getEncoderPosition();

    /**
     * Return target motor position in the arbitrary units.
     * @return float - target position of motor in arbitrary units.
     */
    float getTargetPosition();

    /**
     * Return target motor position in steps
     * @return int - target position of motor in steps
     */

    int getTargetPositionInSteps();

    // FIXME This can be constant after construction?
    /**
     * @brief set the steps-units converter. Per default each instance has a 1:1 converter
     */
    virtual void setStepperMotorUnitsConverter(std::shared_ptr<mtca4u::StepperMotorUnitsConverter> stepperMotorUnitsConverter);

    // FIXME This can be constant after construction?
    /**
     * @brief set the steps-units converter to the default one
     */
    virtual void setStepperMotorUnitsConverterToDefault();

    /**
     * Check if the system is in idle and it returns true is it is so.
     * If in idle any action required by the user can be executed like move or move relative.
     */
    virtual bool isSystemIdle();

    /**
     * @brief actions are executed asynchronous, so using this function one can block the program in case an action is being executed
     * and waiting until this will be terminated and the system is back to idle.
     */
    virtual void waitForIdle();


    /**
     * @brief Returns the state of the stepper motor's state machine
     */
    virtual std::string getState();

    /**
     * @brief return error code for the motor. The error code is not a bit-field
     * 0 - NO_ERROR
     * 1 - ACTION_ERROR if any of the action started by the user was not successful
     * 2 - MOVE_INTERUPTED if the motion of the motor was interrupted, e.g. a software/hardware limit was hit.
     * 3 - CALIBRATION_LOST if calibration is lost.
     */
    virtual StepperMotorError getError();

    /**
     * @brief function returns true if system is calibrated, false otherwise
     */
    virtual bool isCalibrated();

    /**
     * @brief function returns time last calibration, 0 if no calibration was performed about booting the hardware.
     */

    virtual uint32_t getCalibrationTime();

    /**
     * @brief enable/disable the motor
     * @param enable if true motor is enabled
     */
    virtual void setEnabled(bool enable);

    /**
     * @brief get motor enalbed flag
     */
    virtual bool getEnabled();

    /**
     * @brief set logger level
     */
    virtual void setLogLevel(ChimeraTK::Logger::LogLevel newLevel);


    /**
     * @brief Sets the autostart flag. Allows automatic start of movement
     *        on change of target position if set to true
     */
    virtual void setAutostart(bool autostart);

    /**
     * @brief Returns the value of the autostart flag. If true, movement is
     *        initiated automatically on change of the target position
     */
    virtual bool getAutostart();

    /**
     * @brief get logger level
     */
    virtual ChimeraTK::Logger::LogLevel getLogLevel();

    /**
     * @brief get maximal speed capability
     */
    virtual double getMaxSpeedCapability();

    /**
     * @brief get safe current limit
     */
    virtual double getSafeCurrentLimit();

    /**
     * @brief set user current limit for the motor
     * @param currentInAmps current limit in ampere
     */
    virtual double setUserCurrentLimit(double currentInAmps);

    /**
     * @brief returns the user current limit in ampere
     */
    virtual double getUserCurrentLimit();

    /**
     * @brief set user speed limit
     */
    virtual double setUserSpeedLimit(double newSpeed);//todo newSpeed unit!?!?

    /**
     * @brief return user speed limit
     */
    virtual double getUserSpeedLimit();//todo newSpeed unit!?!?

    /**
     * @brief enabling full stepping movement.
     * The target position is rounded to the next full step value before it is written to the register of the controller chip.
     *
     */
    void enableFullStepping(bool enable = true);

    /**
     * @brief returns true if motor is moving per full step, false otherwise
     */
    bool isFullStepping();

    friend class StepperMotorStateMachine;
    friend class StepperMotorChimeraTKTest;
    //boost::shared_ptr<mtca4u::MotorControler> getController(){return _motorControler;}

  protected: // fields
    StepperMotor();
    std::string _motorDriverCardDeviceName;
    unsigned int _motorDriverId;
    boost::shared_ptr<mtca4u::MotorDriverCard> _motorDriverCard;
    boost::shared_ptr<mtca4u::MotorControler> _motorControler;
    std::shared_ptr<ChimeraTK::StepperMotorUnitsConverter> _stepperMotorUnitsConverter;
    std::shared_ptr<ChimeraTK::StepperMotorUnitsConverter> _encoderUnitsConverter;
    std::atomic<int> _targetPositionInSteps;
    int  _maxPositionLimitInSteps;
    int  _minPositionLimitInSteps;
    bool _autostart;
    bool _softwareLimitsEnabled;
    volatile std::atomic<bool> _runStateMachine;
    Logger _logger;
    mutable boost::mutex _mutex;
    boost::mutex _converterMutex;
    std::thread _stateMachineThread;
    std::shared_ptr<StateMachine> _stateMachine;
    //std::atomic<bool> _calibrated;
    virtual bool stateMachineInIdleAndNoEvent();
    void stateMachineThreadFunction();
    void stateMachinePerformTransition();
    void resetPositionMotorController(int newPositionInStep);
    virtual void createStateMachine();
    virtual bool limitsOK(int newPositionInSteps);
    bool checkIfOverflow(int termA, int termB);
    void checkConditionsSetTargetPosAndEmitMoveEvent(int newPositionInSteps);
    virtual void resetMotorControlerAndCheckOverFlowSoftLimits(int translationInSteps);
  };
}// namespace ChimeraTK
#endif	/* MTCA4U_STEPPER_MOTOR_H */
