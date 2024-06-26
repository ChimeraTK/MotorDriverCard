/**
 * File:   StepperMotor.h
 * Author: tkozak
 *
 * Created on April 8, 2014, 11:49 AM
 */

#ifndef MTCA4U_STEPPER_MOTOR_H
#define MTCA4U_STEPPER_MOTOR_H

#include <boost/shared_ptr.hpp> // Boost kept for compatibility with mtca4u implementation and lower layers
#include <boost/thread.hpp>

#include <atomic>
#include <memory>
#include <string>

// MD22 library includes
#include "MotorControler.h"
#include "MotorDriverCard.h"
#include "MotorDriverCardConfigXML.h"

// Stepper Motor includes
#include "Logger.h"
#include "StepperMotorCalibrationStatus.h"
#include "StepperMotorError.h"
#include "StepperMotorStatus.h"
#include "StepperMotorUtil.h"

namespace mtca4u {

  using namespace ChimeraTK;
  using StepperMotorUnitsConverter = ChimeraTK::MotorDriver::utility::MotorStepsConverter;
  using StepperMotorUnitsConverterTrivia = ChimeraTK::MotorDriver::utility::MotorStepsConverterTrivia;

  /**
   * @class StepperMotorStatusAndError
   * @details Container class for status and error of StepperMotor. These two
   * object describes the state of motor and create a pair.
   */
  class StepperMotorStatusAndError {
   public:
    StepperMotorStatus status;
    StepperMotorError error;

    StepperMotorStatusAndError(
        StepperMotorStatus statusParam = StepperMotorStatus(), StepperMotorError errorParam = StepperMotorError())
    : status(statusParam), error(errorParam) {}
  };
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // StepperMotor class !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  class StepperMotor {
   public:
    /**
     * @brief  Constructor of the class object
     * @param  motorDriverCardDeviceName Name of the device in DMAP file
     * @param  moduleName Name of the module in the map file (there might be more
     * than one MD22 per device/ FMC carrier).
     * @param  motorDriverId Each Motor Card Driver has two independent Motor
     * Drivers (can drive two physical motors). ID defines which motor should be
     * represented by this class instantiation
     * @param  motorDriverCardConfigFileName Name of configuration file
     * @return
     */
    StepperMotor(std::string const& motorDriverCardDeviceName, std::string const& moduleName,
        unsigned int motorDriverId, std::string motorDriverCardConfigFileName);

    /**
     * @brief  Destructor of the class object
     */
    virtual ~StepperMotor();

    /**
     * @brief Sending motor to new position (blocking).
     * @param  newPosition - new position for the motor
     * @return StepperMotorStatusAndError - status of the motor after movement
     * routine finish
     *
     * @details
     * This method sends motor to new position. It is blocking blocking method. It
     * will finish when motor movement will end with success or error. Function
     * can be interrupted by using 'void stopMotor()' method.\n Function returns
     * StepperMotorStatusAndError object which is status of the motor after
     * movement routine finished. Should be StepperMotorStatusTypes::M_OK in case
     * of success.
     */
    StepperMotorStatusAndError moveToPosition(float newPosition);

    /**
     * @brief Sending motor to new position (blocking)
     * @param newPositionInSteps - new position in steps for the motor
     * @return StepperMotorStatusAndError - status of the motor after movement
     * routine finish
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
     * When Autostart flag is set to TRUE motor will automatically start moving to
     * desired position by calling 'void start()' method.\n When Autostart flag is
     * set to FALSE motor will NOT automatically start moving to desired position.
     * Additionally, execution of start() method is necessary to start motor.\n
     * This is NO blocking method so user needs to monitor motor status.\n
     * Position is expressed in arbitrary units ex. mm or fs or deg or steps.
     * Recalculation from arbitrary unit to steps is done by object of
     * StepperMotorUnitsConverter.\n StepperMotorUnitsConverter can be set by
     * setStepperMotorUnitsConverter method. When not set conversion coefficient
     * is 1 (steps = 1 x units). \n
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
     * Return real motor position read directly from the hardware and expressed in
     * the arbitrary units. Calculation between steps and arbitrary units is done
     * internally.\n
     * @return float - current, real position of motor in arbitrary units.
     */
    float getCurrentPosition(); // old name: getMotorPosition

    /**
     * Return real motor position read directly from the hardware and expressed in
     * steps. No calculation is involved.
     * @return int - current, real position in number of steps. The number of
     * steps can be either negative or positive, depending on the position with
     * respect to the zero.
     */
    int getCurrentPositionInSteps();
    /**
     * @brief Set position passed as parameter as current position of the motor
     * @param  newPosition - new position for the motor expressed in arbitrary
     units

     * @details
     * Set given position as a current position of the motor.\n
     * Position is expressed in arbitrary units and internally recalculated into
     steps.\n
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
     *  Set a pointer to StepperMotorUnitsConverter object. The
     * StepperMotorUnitsConverter is an interface (abstract class) which provides
     * two methods which are converting steps to arbitrary units and vice-versa.
     * The user need to create its own class which inherits from
     *  StepperMotorUnitsConverter and implements the: 'float stepsToUnits(int
     * steps)' and 'int unitsToSteps(float units)' methods.
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
     * Function starts the motor. When 'Autostart' is set to FALSE it allows to
     * move motor.\n
     *
     */
    void start();

    /**
     * @brief Stop the motor
     *
     * @details
     * Function stop the motor in a way that real position from the motor is set
     * as a target position.\n It also interrupts all blocking functions as
     * 'StepperMotorStatusAndError moveToPosition(float newPosition)'. \n Delays
     * in communication can trigger effect that motor will move a few steps in
     * current movement direction and later will move couple steps back, but this
     * issue can be filtered by motor hysteresis effect.\n
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
     * Function stops the motor by disabling driver. It means that it is done in
     * the fastest possible way.\n Function internally calls 'void stopMotor()'
     * method so all blocking functions will be interrupted.\n
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
     * Return current calibration status as StepperMotorCalibrationStatus
     * object.\n Currently following status are forseen:\n 1)
     * StepperMotorCalibrationStatusType::M_CALIBRATION_UNKNOWN - calibration
     * status in unknown. This is default value after object creation.\n 2)
     * StepperMotorCalibrationStatusType::M_CALIBRATED - motor calibrated.\n 3)
     * StepperMotorCalibrationStatusType::M_CALIBRATED -motor not calibrated (ex.
     * after emergency stop).\n 4)
     * StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED - motor calibration
     * failed.\n 5) StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS -
     * motor is calibrating now.\n 6)
     * StepperMotorCalibrationStatusType::M_CALIBRATION_STOPED_BY_USER - motor
     * calibration was triggered by it was stopped by user (ex. execution of 'void
     * stopMotor()' function).\n 7)
     * StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE - motor
     * calibration is not possible because motor doesn't have physical end
     * switches.\n
     */
    StepperMotorCalibrationStatus getCalibrationStatus();

    /**
     * @brief Return object which contains current status and error.
     * @return StepperMotorStatusAndError - object which contains of status and
     * error code
     *
     * @details
     * Return current motor status and error as StepperMotorStatusAndError
     * object.\n It give possibility to check what was an error type when status
     * is set to StepperMotorStatusTypes::M_ERROR. Status and error create pair
     * which describes current state of the object. Every time it is called it
     * checks status of the motor.
     *
     * Currently following status are foreseen:\n
     * 1) StepperMotorStatusTypes::M_OK - motor is OK and ready for new
     * commands.\n 2) StepperMotorStatusTypes::M_DISABLED - motor is disabled. No
     * action can be done. to change this use 'void setEnabled(bool enable)'
     * method.\n 3) StepperMotorStatusTypes::M_IN_MOVE -motor in in move.\n 4)
     * StepperMotorStatusTypes::M_NOT_IN_POSITION - target position in different
     * than current one and motor is not moving \n 5)
     * StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON - target position
     * in greater than value set by 'void setMaxPositionLimit(float maxPos)'
     * method.\n 6) StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON -
     * target position in smaller than value set by 'void
     * setMinPositionLimit(float maxPos)' method.\n 7)
     * StepperMotorStatusTypes::M_ERROR - motor is in error state. For details
     * about error check 'StepperMotorError getError()' method.\n
     *
     * Currently following errors are foreseen:\n
     * 1) StepperMotorErrorTypes::M_NO_ERROR - motor is OK and ready for new
     * commands.\n 2)
     * StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX - error
     * in configuration of software position limits\n 3)
     * StepperMotorErrorTypes::M_COMMUNICATION_LOST - errors and problems with
     * communication with firmware\n 4)
     * StepperMotorErrorTypes::M_NO_REACION_ON_COMMAND - hardware is no reacting
     * on command.\n 5) StepperMotorErrorTypes::M_HARDWARE_NOT_CONNECTED -
     * hardware is not connected.\n
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
     *         [0.5625A, 1.8A] and will be closest to the user requested
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
     * When Autostart flag is set to TRUE motor will automatically start moving to
     * desired position by internally calling 'void start()' method.\n When
     * Autostart flag is set to FALSE motor will NOT automatically start moving to
     * desired position. Additionally, execution of start() method is necessary to
     * start motor.\n
     *
     */
    void setAutostart(bool autostart);

    /**
     * @brief Get the Autostart flag.
     * @return bool - Current autostart flag value
     *
     * @details
     * When Autostart flag is set to TRUE motor will automatically start moving to
     * desired position by internally calling 'void start()' method.\n When
     * Autostart flag is set to FALSE motor will NOT automatically start moving to
     * desired position. Additionally, execution of start() method is necessary to
     * start motor.\n
     *
     */
    bool getAutostart();

    /**
     * @brief Enables the motor.
     * @param enable - New value for the Enabled flag
     *
     * @details
     * Motor can be disabled or enabled. To work with the motor user need to
     * enable it. When motor disabled it stops moving immediately.
     */
    void setEnabled(bool enable);

    /**
     * @brief Get the enable status.
     * @return bool - Current Enabled flag value
     *
     * @details
     * Motor can be disabled or enabled. To work with the motor user need to
     * enable it. When motor disabled it stops moving immediately.
     */
    bool getEnabled() const;

    /**
     * @brief Set soft limit on the maximum position of the motor expressed in
     * arbitrary units
     * @param maxPos - Greatest position which motor will be able to reach
     *
     * @details
     * Motor can have not physical end switches which would limits range of
     * possible position. For the reason software limits has been introduced.
     */
    void setMaxPositionLimit(float maxPos);

    /**
     * @brief Set soft limit on the maximum position of the motor expressed in
     * steps
     * @param maxPosInSteps - Greatest position which motor will be able to reach
     */
    void setMaxPositionLimitInSteps(int maxPosInSteps);

    /**
     * @brief Set soft limit on the minimum position of the motor expressed in
     * arbitrary units
     * @param minPos - Smallest position which motor will be able to reach
     *
     * @details
     * Motor can have not physical end switches which would limits range of
     * possible position. For the reason software limits has been introduced.
     */
    void setMinPositionLimit(float minPos);

    /**
     * @brief Set soft limit on the minimum position of the motor expressed in
     * steps
     * @param minPosInSteps - Smallest position which motor will be able to reach
     */
    void setMinPositionLimitInSteps(int minPosInSteps);

    /**
     * @brief Set soft limit enabled or disabled.
     * @param enabled - When true soft limits will be checked and validated, in
     * false they will not
     *
     * @details
     *
     */
    void setSoftwareLimitsEnabled(bool enabled);

    /**
     * @brief Get current soft limit enabled flag.
     * @return bool - When true soft limits will be checked and validated, in
     * false they will not
     *
     * @details
     */
    bool getSoftwareLimitsEnabled() const;

    /**
     * @brief Returns soft limit on the maximum position of the motor expressed in
     * arbitrary units
     * @return float - Maximum position which motor is able to reach
     *
     * @details
     * Motor can have not physical end switches which would limits range of
     * possible position. For the reason software limits has been introduced.
     */
    float getMaxPositionLimit();

    /**
     * @brief Returns soft limit on the maximum position of the motor expressed in
     * steps
     * @return float - Maximum position which motor is able to reach
     */
    int getMaxPositionLimitInSteps();

    /**
     * @brief Returns soft limit on the minimum position of the motor expressed in
     * arbitrary units
     * @return float - Minimum position which motor is able to reach
     *
     * @details
     * Motor can have not physical end switches which would limits range of
     * possible position. For the reason software limits has been introduced.
     */
    float getMinPositionLimit();

    /**
     * @brief Returns soft limit on the minimum position of the motor expressed in
     * steps
     * @return float - Minimum position which motor is able to reach
     */
    int getMinPositionLimitInSteps();

    /**
     * @brief Recalculate arbitrary units into steps
     * @param units Arbitrary units in which position is expressed
     * @return int - steps value which corresponds to given as parameter value of
     * arbitrary units.
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
    void setLogLevel(MotorDriverCardDetail::Logger::LogLevel newLevel);

    /**
     * @brief Get current log level of the motor
     * @return Logger::LogLevel - current log level
     *
     * @details
     */
    MotorDriverCardDetail::Logger::LogLevel getLogLevel();

    void enableFullStepping(bool enable = true);
    bool isFullStepping();

   protected: // methods
    StepperMotorStatusAndError determineMotorStatusAndError();
    int truncateMotorPosition(int newPositionInSteps);

   protected: // fields
    std::string _motorDriverCardDeviceName;
    unsigned int _motorDriverId;

    // pointers to objects which controls the physical driver, we will use
    // boost::shared_ptr
    boost::shared_ptr<MotorDriverCard> _motorDriverCard;
    boost::shared_ptr<MotorControler> _motorControler;

    // current position
    // int _currentPostionsInSteps;
    // float _currentPostionsInUnits;
    // current target
    boost::shared_ptr<StepperMotorUnitsConverter> _stepperMotorUnitsConverter;
    int _targetPositionInSteps;
    // float _targetPositionInUnits;
    // position limits
    // float _maxPositionLimit;
    int _maxPositionLimitInSteps;
    // float _minPositionLimit;
    int _minPositionLimitInSteps;
    // pointer to the units converter class
    // Autostart
    bool _autostartFlag;
    // Stop motor flag for blocking functions
    bool _stopMotorForBlocking;
    // soft limits enabled flag
    bool _softwareLimitsEnabled;
    // status and error
    StepperMotorError _motorError;
    StepperMotorCalibrationStatus _motorCalibrationStatus;
    StepperMotorStatus _motorStatus;
    // flag which indicate error in blocking function
    bool _blockingFunctionActive;
    MotorDriverCardDetail::Logger _logger;
    mutable boost::mutex _mutex;
  };
} // namespace mtca4u
#endif /* MTCA4U_STEPPER_MOTOR_H */
