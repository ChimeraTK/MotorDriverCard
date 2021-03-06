/**
 * File:   BasicStepperMotor.h
 * Author: ckampm
 *
 * This file declares a basic implementation of
 * the ChimeraTK::StepperMotor
 *
 * Created on April 8, 2014, 11:49 AM
 */

#ifndef CHIMERATK_BASIC_STEPPER_MOTOR_H
#define CHIMERATK_BASIC_STEPPER_MOTOR_H

#include "StateMachine.h"
#include "StepperMotor.h"

#include <atomic>
#include <boost/shared_ptr.hpp> // Boost kept for compatibility with mtca4u implementation and lower layers
#include <boost/thread.hpp>
#include <memory>
#include <string>

namespace mtca4u {
  class MotorDriverCard;
  class MotorControler;
} // namespace mtca4u

// Forward-declare fixture used in the test
class StepperMotorChimeraTKFixture;

namespace ChimeraTK { namespace MotorDriver {

  /**
   *  @class BasicStepperMotor
   *  @brief This class implements the basic implementation stepper motor.
   */
  class BasicStepperMotor : public StepperMotor {
   public:
    /**
     * @brief  Constructor of the class object
     * @param  parameters Configuration parameters of type StepperMotorParameters
     */
    BasicStepperMotor(const StepperMotorParameters& parameters);

    /**
     * @brief  Destructor of the class object
     */
    virtual ~BasicStepperMotor();

    /**
     * @ brief move the motor a delta from the current position
     * @param delta is given in unit
     */
    virtual ExitStatus moveRelative(float delta);

    /**
     * @brief move the motor a delta from the current position
     * @param delta is given in steps
     */
    virtual ExitStatus moveRelativeInSteps(int delta);

    /**
     * @brief Sets the target position in arbitrary units (according to the
     * scaling).
     *
     *        If the autostart flag is set to true, this will initiate movement,
     *        otherwise movement needs to be triggered by calling start().
     */
    virtual ExitStatus setTargetPosition(float newPosition);

    /**
     * @brief Sets the target position in steps.
     *
     *        If the motor is idle and the autostart flag is set to true,
     *        this will initiate movement, otherwise movement needs to be
     *        triggered by calling start().
     *        If the motor is moving, the target position will
     *        be updated.
     */
    virtual ExitStatus setTargetPositionInSteps(int newPositionInSteps);

    /**
     * @brief Initiates movement of the motor.
     *
     *  This command is called implicitly if autostart mode is enabled.
     *  @see setAutostart()
     */
    virtual void start();

    /**
     * @brief interrupt the current action and return the motor to idle
     *
     */
    virtual void stop();

    /**
     * @brief Immediately interrupt the current action, return the motor to idle
     * and disable it.
     *
     * Note: As an effect of stopping the motor as fast as possible, the real
     * motor position and the controller's step counter will lose synchronization.
     * Hence, calibration will be lost after calling this method.
     */
    virtual void emergencyStop();

    /**
     * @brief Reset error state
     *
     * Depending on the event that caused the error, this will set the motor to
     * disabled\n (if power supply is disabled, e.g. after emergency stop) or idle
     * state (e.g. some \n action failed, but power supply is enabled).
     */
    virtual void resetError();

    /**
     * @brief It transforms units in steps using the internal converter
     * @param units Value in unit to be converted in steps
     * @return Value in steps
     */
    virtual int recalculateUnitsInSteps(float units);

    /**
     * @brief it transforms steps in units using the internal converter
     * @param steps Value in steps to be converted in units
     * @return Value in units
     */
    virtual float recalculateStepsInUnits(int steps);

    /**
     * @brief enable/disable software position limits
     */
    virtual ExitStatus setSoftwareLimitsEnabled(bool enabled);

    /**
     * @brief get software position limits enabled flag
     */
    virtual bool getSoftwareLimitsEnabled();

    /**
     * @brief set maximum software limit in units
     * @param maxPos maximum limit
     */
    virtual ExitStatus setMaxPositionLimit(float maxPos);

    /**
     * @brief set maximum software limit in steps
     * @param maxPos maximum limit
     */
    virtual ExitStatus setMaxPositionLimitInSteps(int maxPos);

    /**
     * @brief set minimum software limit in units
     * @param minPos minimum limit
     */
    virtual ExitStatus setMinPositionLimit(float minPos);

    /**
     * @brief set minimum software limit in steps
     * @param minPos maximum limit
     */
    virtual ExitStatus setMinPositionLimitInSteps(int minPos);

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
     * @param actualPositionInUnits In order to use the motor an absolute scale
     * must be defined.
     *
     * This can be done defining the position of the motor respect to an external
     * reference (actual position).\n In addition to that, the conversion between
     * steps and unit must provided through the method
     * setStepperMotorUnitsConverter.
     */
    virtual ExitStatus setActualPosition(float actualPositionInUnits);

    /**
     * @brief set actual position in steps of the motor respect to some reference
     * @param actualPositionInSteps In order to use the motor an absolute scale
     * must be defined.
     *
     * This can be done defining the position of the motor respect to an external
     * reference (actual position).
     */
    virtual ExitStatus setActualPositionInSteps(int actualPositionInSteps);

    /**@brief translate the reference axis of the motor. This operation will
     * translate also the software limits
     * @param translationInSteps translation value in steps
     */

    virtual ExitStatus translateAxisInSteps(int translationInSteps);

    /**@brief translate the reference axis of the motor. This operation will
     * translate also the software limits
     * @param translationInUnits translation value in unit
     */

    virtual ExitStatus translateAxis(float translationInUnits);

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
     *  @brief Set the actual encoder position to a reference value.
     *
     *  Analogous to setActualPosition() which sets the motor driver's internal\n
     *  step counter to a reference value, this function can be used to define a
     * reference\n for the encoder output.
     */
    virtual ExitStatus setActualEncoderPosition(double referencePosition);

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
     * @brief set the steps-units converter. Per default each instance has a 1:1
     * converter
     */
    virtual ExitStatus setStepperMotorUnitsConverter(
        std::shared_ptr<utility::MotorStepsConverter> stepperMotorUnitsConverter);

    // FIXME This can be constant after construction?
    /**
     * @brief set the steps-units converter to the default one
     */
    virtual ExitStatus setStepperMotorUnitsConverterToDefault();

    /**
     * Check if the system is in idle and it returns true is it is so.
     * If in idle any action required by the user can be executed like move or
     * move relative.
     */
    virtual bool isSystemIdle();

    /**
     * @brief actions are executed asynchronous, so using this function one can
     * block the program in case an action is being executed and waiting until
     * this will be terminated and the system is back to idle.
     */
    virtual void waitForIdle();

    /**
     * @brief Returns the state of the stepper motor's state machine
     */
    virtual std::string getState();

    /**
     * @brief Return error code for the motor. The error code is not a
     * bit-field.\n 0 - NO_ERROR\n 1 - ACTION_ERROR if any of the action started
     * by the user was not successful\n 2 - MOVE_INTERUPTED if the motion of the
     * motor was interrupted, e.g. a software/hardware limit was hit.\n 3 -
     * CALIBRATION_LOST if calibration is lost.\n
     */
    virtual Error getError();

    /**
     * @brief function returns true if system is calibrated, false otherwise
     */
    virtual bool isCalibrated();

    /**
     * @brief function returns time last calibration, 0 if no calibration was
     * performed about booting the hardware.
     */

    virtual uint32_t getCalibrationTime();

    /**
     * @brief Enable/disable the motor.
     * @param enable If true, motor is enabled.
     */
    virtual void setEnabled(bool enable);

    /**
     * @brief get motor enabled flag
     */
    virtual bool getEnabled();

    /**
     * @brief Sets the autostart flag.
     *        Allows automatic start of movement on change of target position if
     * set to true.
     */
    virtual void setAutostart(bool autostart);

    /**
     * @brief Returns the value of the autostart flag.
     *        If true, movement is initiated automatically on change of the target
     * position.
     */
    virtual bool getAutostart();

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
    virtual ExitStatus setUserCurrentLimit(double currentInAmps);

    /**
     * @brief returns the user current limit in ampere
     */
    virtual double getUserCurrentLimit();

    /**
     * @brief set user speed limit in microsteps per second
     */
    virtual ExitStatus setUserSpeedLimit(double speedInUstepsPerSec);

    /**
     * @brief return user speed limit in microsteps per secon.
     */
    virtual double getUserSpeedLimit();

    /**
     * @brief Returns True if the motor is moving and false if at
     * standstill. This command is reliable as long as the motor is not
     * stalled.
     */
    bool isMoving();

    /**
     * @brief enabling full stepping movement.
     * The target position is rounded to the next full step value before it is
     * written to the register of the controller chip.
     *
     */
    void enableFullStepping(bool enable = true);

    /**
     * @brief returns true if motor is moving per full step, false otherwise
     */
    bool isFullStepping();

    virtual bool hasHWReferenceSwitches();

    virtual ExitStatus calibrate();

    virtual ExitStatus determineTolerance();

    virtual float getPositiveEndReference();

    virtual int getPositiveEndReferenceInSteps();

    virtual float getNegativeEndReference();

    virtual int getNegativeEndReferenceInSteps();

    virtual float getTolerancePositiveEndSwitch();

    virtual float getToleranceNegativeEndSwitch();

    virtual bool isPositiveReferenceActive();

    virtual bool isNegativeReferenceActive();

    virtual bool isPositiveEndSwitchEnabled();

    virtual bool isNegativeEndSwitchEnabled();

    virtual CalibrationMode getCalibrationMode();

    //    /friend class StateMachine;
    friend class ::StepperMotorChimeraTKFixture;

   protected:
    /**
     * StateMachine subclass specific to this implementation
     */
    class StateMachine : public utility::StateMachine {
      friend class BasicStepperMotor;

     public:
      StateMachine(BasicStepperMotor& stepperMotor);
      virtual ~StateMachine();
      static const Event initialEvent;
      static const Event moveEvent;
      static const Event stopEvent;
      static const Event errorEvent;
      static const Event emergencyStopEvent;
      static const Event actionCompleteEvent;
      static const Event enableEvent;
      static const Event disableEvent;
      static const Event resetToIdleEvent;
      static const Event resetToDisableEvent;

     protected:
      State _moving;
      State _idle;
      State _disabled;
      State _error;
      BasicStepperMotor& _stepperMotor;
      boost::shared_ptr<mtca4u::MotorControler>& _motorControler;
      void getActionCompleteEvent();
      void waitForStandstill();
      void actionIdleToMove();
      void actionMovetoStop();
      void actionMoveToFullStep();
      void actionEnable();
      void actionDisable();
      void actionEmergencyStop();
      void actionResetError();
    }; // class StateMachine

    BasicStepperMotor();

    /**
     * Returns true if the motor is not active,
     * i.e. in disabled or idle state
     */
    virtual bool motorActive();

    /// Common actions for setActualPosition for this and derived classes
    void setActualPositionActions(int actualPositionInSteps);

    /// Resets target and actual postions of the MotorControler
    void resetMotorControllerPositions(int newPositionInStep);

    /// Translate positions limits, if in numerical range
    void translateLimits(int translationInSteps);

    virtual void initStateMachine();
    virtual bool limitsOK(int newPositionInSteps);
    bool checkIfOverflow(int termA, int termB);
    virtual ExitStatus checkNewPosition(int newPositionInSteps);

    /// Checks if moving resulted in the requested target position
    virtual bool verifyMoveAction();

    /// Common actions for translateAxis for this and derived classes
    void translateAxisActions(int translationInSteps);

    boost::shared_ptr<mtca4u::MotorDriverCard> _motorDriverCard;
    boost::shared_ptr<mtca4u::MotorControler> _motorControler;

    std::shared_ptr<utility::MotorStepsConverter> _stepperMotorUnitsConverter;
    std::shared_ptr<utility::EncoderStepsConverter> _encoderUnitsConverter;

    int _encoderPositionOffset;
    std::atomic<int> _targetPositionInSteps;
    int _maxPositionLimitInSteps;
    int _minPositionLimitInSteps;
    bool _autostart;
    bool _softwareLimitsEnabled;

    mutable boost::mutex _mutex;
    std::shared_ptr<utility::StateMachine> _stateMachine;

    std::atomic<Error> _errorMode;
    std::atomic<CalibrationMode> _calibrationMode;

  }; // class BasicStepperMotor
}}   // namespace ChimeraTK::MotorDriver

#endif /* CHIMERATK_BASIC_STEPPER_MOTOR_H */
