/**
 * File:   StepperMotor.h
 * Author: ckampm
 *
 * Provides StepperMotor base class and
 * a Factory to create instances.
 *
 */

#ifndef CHIMERATK_STEPPER_MOTOR_H
#define CHIMERATK_STEPPER_MOTOR_H

#include "StepperMotorUtil.h"

#include <map>
#include <memory>
#include <mutex>
#include <string>

// Forward-declare fixture used in the test
class StepperMotorChimeraTKFixture;

namespace ChimeraTK { namespace MotorDriver {

  /**
   * @brief Contains parameters for initialization of a StepperMotor object
   */
  struct StepperMotorParameters {
    StepperMotorType motorType{StepperMotorType::BASIC};
    /// Name of the device in DMAP file
    std::string deviceName{""};
    /// Name of the module in the map file (there might be more than one MD22 per
    /// device/ FMC carrier).
    std::string moduleName{""};
    /// Each Motor Card Driver has two independent Motor Drivers (can drive two
    /// physical motors). ID defines which motor should be represented by this
    /// class instantiation
    unsigned int driverId{0U};
    /// Name of configuration file
    std::string configFileName{""};
    /// A converter between motor steps and user unit. Based on the abstract class
    /// StepperMotorUnitsConverter. Defaults to a 1:1 converter between units and
    /// steps.
    std::shared_ptr<utility::MotorStepsConverter> motorUnitsConverter{
        std::make_shared<utility::MotorStepsConverterTrivia>()};
    /// A converter between encoder steps and user unit. Based on the abstract
    /// class EncoderUnitsConverter. Defaults to a 1:1 converter between units and
    /// steps.
    std::shared_ptr<utility::EncoderStepsConverter> encoderUnitsConverter{
        std::make_shared<utility::EncoderStepsConverterTrivia>()};
  };

  /**
   *  @class StepperMotor
   *  @brief This class provides the user interface for a basic stepper motor.
   */
  class StepperMotor {
   public:
    /**
     * @brief  Destructor of the class object
     */
    virtual ~StepperMotor() {}

    /**
     * @ brief move the motor a delta from the current position
     * @param delta is given in unit
     */
    [[nodiscard]] virtual ExitStatus moveRelative(float delta) = 0;

    /**
     * @brief move the motor a delta from the current position
     * @param delta is given in steps
     */
    [[nodiscard]] virtual ExitStatus moveRelativeInSteps(int delta) = 0;

    /**
     * @brief Sets the target position in arbitrary units (according to the
     * scaling).
     *
     *        If the autostart flag is set to true, this will initiate movement,
     *        otherwise movement needs to be triggered by calling start().
     */
    [[nodiscard]] virtual ExitStatus setTargetPosition(float newPosition) = 0;

    /**
     * @brief Sets the target position in steps.
     *
     *        If the autostart flag is set to true, this will initiate movement,
     *        otherwise movement needs to be triggered by calling start().
     */
    [[nodiscard]] virtual ExitStatus setTargetPositionInSteps(int newPositionInSteps) = 0;

    /**
     * @brief Initiates movement of the motor.
     *
     *  This command is called implicitly if autostart mode is enabled.
     *  @see setAutostart()
     */
    virtual void start() = 0;

    /**
     * @brief interrupt the current action and return the motor to idle
     *
     */
    virtual void stop() = 0;

    /**
     * @brief Immediately interrupt the current action, return the motor to idle
     * and disable it.
     *
     * Note: As an effect of stopping the motor as fast as possible, the real
     * motor position and the controller's step counter will lose synchronization.
     * Hence, calibration will be lost after calling this method.
     */
    virtual void emergencyStop() = 0;

    /**
     * @brief Reset error state
     *
     * Depending on the event that caused the error, this will set the motor to
     * disabled\n (if power supply is disabled, e.g. after emergency stop) or idle
     * state (e.g. some \n action failed, but power supply is enabled).
     */
    virtual void resetError() = 0;

    /**
     * @brief It transforms units in steps using the internal converter
     * @param units Value in unit to be converted in steps
     * @return Value in steps
     */
    [[nodiscard]] virtual int recalculateUnitsInSteps(float units) = 0;

    /**
     * @brief it transforms steps in units using the internal converter
     * @param steps Value in steps to be converted in units
     * @return Value in units
     */
    [[nodiscard]] virtual float recalculateStepsInUnits(int steps) = 0;

    /**
     * @brief enable/disable software position limits
     */
    [[nodiscard]] virtual ExitStatus setSoftwareLimitsEnabled(bool enabled) = 0;

    /**
     * @brief get software position limits enabled flag
     */
    [[nodiscard]] virtual bool getSoftwareLimitsEnabled() = 0;

    /**
     * @brief set maximum software limit in units
     * @param maxPos maximum limit
     */
    [[nodiscard]] virtual ExitStatus setMaxPositionLimit(float maxPos) = 0;

    /**
     * @brief set maximum software limit in steps
     * @param maxPos maximum limit
     */
    [[nodiscard]] virtual ExitStatus setMaxPositionLimitInSteps(int maxPos) = 0;

    /**
     * @brief set minimum software limit in units
     * @param minPos minimum limit
     */
    [[nodiscard]] virtual ExitStatus setMinPositionLimit(float minPos) = 0;

    /**
     * @brief set minimum software limit in steps
     * @param minPos maximum limit
     */
    [[nodiscard]] virtual ExitStatus setMinPositionLimitInSteps(int minPos) = 0;

    /**
     * @brief get maximal software position limit in units
     */
    [[nodiscard]] virtual float getMaxPositionLimit() = 0;

    /**
     * @brief get maximal software position limit in steps
     */
    [[nodiscard]] virtual int getMaxPositionLimitInSteps() = 0;

    /**
     * @brief get minimal software position limit in units
     */
    [[nodiscard]] virtual float getMinPositionLimit() = 0;

    /**
     * @brief get minimal software position limit in steps
     */
    [[nodiscard]] virtual int getMinPositionLimitInSteps() = 0;

    /**
     * @brief set actual position in units of the motor respect to some reference
     * @param actualPositionInUnits In order to use the motor an absolute scale
     * must be defined.
     *
     * This can be done defining the position of the motor respect to an external
     * reference (actual position).\n In addition to that, the conversion between
     * steps and unit must provided through the method
     * setStepperMotorUnitsConverter.
     */
    [[nodiscard]] virtual ExitStatus setActualPosition(float actualPositionInUnits) = 0;

    /**
     * @brief set actual position in steps of the motor respect to some reference
     * @param actualPositionInSteps In order to use the motor an absolute scale
     * must be defined.
     *
     * This can be done defining the position of the motor respect to an external
     * reference (actual position).
     */
    [[nodiscard]] virtual ExitStatus setActualPositionInSteps(int actualPositionInSteps) = 0;

    /**
     * @brief translate the reference axis of the motor. This operation will
     * translate also the software limits
     * @param translationInSteps translation value in steps
     */
    [[nodiscard]] virtual ExitStatus translateAxisInSteps(int translationInSteps) = 0;

    /**
     * @brief translate the reference axis of the motor. This operation will
     * translate also the software limits
     * @param translationInUnits translation value in unit
     */
    [[nodiscard]] virtual ExitStatus translateAxis(float translationInUnits) = 0;

    /**
     * @brief get current position of the motor in units
     */
    [[nodiscard]] virtual float getCurrentPosition() = 0;

    /**
     * @brief get current position of the motor in steps
     */
    [[nodiscard]] virtual int getCurrentPositionInSteps() = 0;

    /**
     * @brief Get the current position from encoder in user-defined units,
     *        according to the encoderUnitsConverter defined for the instance.
     */
    [[nodiscard]] virtual double getEncoderPosition() = 0;

    /**
     *  @brief Set the actual encoder position to a reference value.
     *
     *  Analogous to setActualPosition() which sets the motor driver's internal\n
     *  step counter to a reference value, this function can be used to define a
     * reference\n for the encoder output.
     */
    [[nodiscard]] virtual ExitStatus setActualEncoderPosition(double referencePosition) = 0;

    [[nodiscard]] virtual unsigned int getEncoderReadoutMode() = 0; ///< Get the decoder readout mode

    /**
     * Return target motor position in the arbitrary units.
     * @return float - target position of motor in arbitrary units.
     */
    [[nodiscard]] virtual float getTargetPosition() = 0;

    /**
     * Return target motor position in steps
     * @return int - target position of motor in steps
     */

    [[nodiscard]] virtual int getTargetPositionInSteps() = 0;

    // FIXME This can be constant after construction?
    /**
     * @brief set the steps-units converter. Per default each instance has a 1:1
     * converter
     */
    [[nodiscard]] virtual ExitStatus setStepperMotorUnitsConverter(
        std::shared_ptr<utility::MotorStepsConverter> stepperMotorUnitsConverter) = 0;

    // FIXME This can be constant after construction?
    /**
     * @brief set the steps-units converter to the default one
     */
    [[nodiscard]] virtual ExitStatus setStepperMotorUnitsConverterToDefault() = 0;

    /**
     * Check if the system is in idle and it returns true is it is so.
     * If in idle any action required by the user can be executed like move or
     * move relative.
     */
    [[nodiscard]] virtual bool isSystemIdle() = 0;

    /**
     * @brief actions are executed asynchronous, so using this function one can
     * block the program in case an action is being executed and waiting until
     * this will be terminated and the system is back to idle.
     */
    virtual void waitForIdle() = 0;

    /**
     * @brief Returns the state of the stepper motor's state machine
     */
    [[nodiscard]] virtual std::string getState() = 0;

    /**
     * @brief Return error code for the motor. The error code is not a
     * bit-field.\n 0 - NO_ERROR\n 1 - ACTION_ERROR if any of the action started
     * by the user was not successful\n 2 - MOVE_INTERUPTED if the motion of the
     * motor was interrupted, e.g. a software/hardware limit was hit.\n 3 -
     * CALIBRATION_LOST if calibration is lost.\n
     */
    [[nodiscard]] virtual Error getError() = 0;

    /**
     * @brief function returns true if system is calibrated, false otherwise
     */
    [[nodiscard]] virtual bool isCalibrated() = 0;

    /**
     * @brief function returns time last calibration, 0 if no calibration was
     * performed about booting the hardware.
     */

    [[nodiscard]] virtual uint32_t getCalibrationTime() = 0;

    /**
     * @brief Enable/disable the motor.
     * @param enable If true, motor is enabled.
     */
    virtual void setEnabled(bool enable) = 0;

    /**
     * @brief get motor enabled flag
     */
    [[nodiscard]] virtual bool getEnabled() = 0;

    /**
     * @brief Sets the autostart flag.
     *        Allows automatic start of movement on change of target position if
     * set to true.
     */
    virtual void setAutostart(bool autostart) = 0;

    /**
     * @brief Returns the value of the autostart flag.
     *        If true, movement is initiated automatically on change of the target
     * position.
     */
    [[nodiscard]] virtual bool getAutostart() = 0;

    /**
     * @brief get maximal speed capability
     */
    [[nodiscard]] virtual double getMaxSpeedCapability() = 0;

    /**
     * @brief get safe current limit
     */
    [[nodiscard]] virtual double getSafeCurrentLimit() = 0;

    /**
     * @brief set user current limit for the motor
     * @param currentInAmps current limit in ampere
     */
    [[nodiscard]] virtual ExitStatus setUserCurrentLimit(double currentInAmps) = 0;

    /**
     * @brief returns the user current limit in ampere
     */
    [[nodiscard]] virtual double getUserCurrentLimit() = 0;

    /**
     * @brief Set user speed limit in microsteps per second
     */
    [[nodiscard]] virtual ExitStatus setUserSpeedLimit(double speedInUstepsPerSec) = 0;

    /**
     * @brief Return user speed limit in microsteps per second
     */
    [[nodiscard]] virtual double getUserSpeedLimit() = 0;

    /**
     * @brief Returns True if the motor is moving and false if at
     * standstill. This command is reliable as long as the motor is not
     * stalled.
     */
    [[nodiscard]] virtual bool isMoving() = 0;

    /**
     * @brief enabling full stepping movement.
     * The target position is rounded to the next full step value before it is
     * written to the register of the controller chip.
     *
     */
    virtual void enableFullStepping(bool enable = true) = 0;

    /**
     * @brief returns true if motor is moving per full step, false otherwise
     */
    [[nodiscard]] virtual bool isFullStepping() = 0;

    /**
     * @brief Returns true if the object supports HW reference switches
     *
     * This function should be called before accessing functions specific to HW
     * reference switches which throw if no support is provided by the concrete
     * object.
     */
    [[nodiscard]] virtual bool hasHWReferenceSwitches() = 0;

    [[nodiscard]] virtual ExitStatus calibrate() = 0;

    [[nodiscard]] virtual ExitStatus determineTolerance() = 0;

    [[nodiscard]] virtual float getPositiveEndReference() = 0;

    [[nodiscard]] virtual int getPositiveEndReferenceInSteps() = 0;

    [[nodiscard]] virtual float getNegativeEndReference() = 0;

    [[nodiscard]] virtual int getNegativeEndReferenceInSteps() = 0;

    [[nodiscard]] virtual float getTolerancePositiveEndSwitch() = 0;

    [[nodiscard]] virtual float getToleranceNegativeEndSwitch() = 0;

    [[nodiscard]] virtual bool isPositiveReferenceActive() = 0;

    [[nodiscard]] virtual bool isNegativeReferenceActive() = 0;

    [[nodiscard]] virtual bool isPositiveEndSwitchEnabled() = 0;

    [[nodiscard]] virtual bool isNegativeEndSwitchEnabled() = 0;

    [[nodiscard]] virtual CalibrationMode getCalibrationMode() = 0;

  }; // class StepperMotor

  /**
   * @brief The StepperMotorFactory is used to create StepperMotor instances.
   *
   * This assures that for each pysical device, only one instance is created
   * and it manages creation of the different implementations.
   */
  class StepperMotorFactory {
   public:
    StepperMotorFactory(const StepperMotorFactory&) = delete;
    StepperMotorFactory(const StepperMotorFactory&&) = delete;
    StepperMotorFactory& operator=(const StepperMotorFactory&) = delete;
    StepperMotorFactory& operator=(const StepperMotorFactory&&) = delete;

    /// Returns a reference to the StepperMotorFactory
    static StepperMotorFactory& instance();

    /// Creates an StepperMotor instance according to the specifed type and
    /// configuration parameters
    std::shared_ptr<StepperMotor> create(const StepperMotorParameters& parameters);

   private:
    /// This is a singleton and can only be retrieved by the instance() method
    StepperMotorFactory();

    /// Allow thread-safe usage of create() function
    std::mutex _factoryMutex;

    /**
     * Used to identify created motors
     * Must take into account device and module names and the id
     * of a motor.
     */
    using MotorIdentifier = std::tuple<std::string, std::string, unsigned>;
    /// A map holding all existing instances
    std::map<MotorIdentifier, std::weak_ptr<StepperMotor>> _existingMotors;
  }; // class StepperMotorFactory

}}     // namespace ChimeraTK::MotorDriver
#endif /* CHIMERATK_STEPPER_MOTOR_H */
