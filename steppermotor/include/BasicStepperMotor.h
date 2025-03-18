// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "StateMachine.h"
#include "StepperMotor.h"

#include <boost/shared_ptr.hpp> // Boost kept for compatibility with mtca4u implementation and lower layers
#include <boost/thread.hpp>

#include <atomic>
#include <memory>
#include <string>

namespace ChimeraTK {
  class MotorDriverCard;
  class MotorControler;
} // namespace ChimeraTK

// Forward-declare fixture used in the test
class StepperMotorChimeraTKFixture;

namespace ChimeraTK::MotorDriver {

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
    explicit BasicStepperMotor(const StepperMotorParameters& parameters);

    /**
     * @brief  Destructor of the class object
     */
    ~BasicStepperMotor() override;

    /**
     * @ brief move the motor a delta from the current position
     * @param delta is given in unit
     */
    ExitStatus moveRelative(float delta) override;

    /**
     * @brief move the motor a delta from the current position
     * @param delta is given in steps
     */
    ExitStatus moveRelativeInSteps(int delta) override;

    /**
     * @brief Sets the target position in arbitrary units (according to the
     * scaling).
     *
     *        If the autostart flag is set to true, this will initiate movement,
     *        otherwise movement needs to be triggered by calling start().
     */
    ExitStatus setTargetPosition(float newPosition) override;

    /**
     * @brief Sets the target position in steps.
     *
     *        If the motor is idle and the autostart flag is set to true,
     *        this will initiate movement, otherwise movement needs to be
     *        triggered by calling start().
     *        If the motor is moving, the target position will
     *        be updated.
     */
    ExitStatus setTargetPositionInSteps(int newPositionInSteps) override;

    /**
     * @brief Initiates movement of the motor.
     *
     *  This command is called implicitly if autostart mode is enabled.
     *  @see setAutostart()
     */
    void start() override;

    /**
     * @brief interrupt the current action and return the motor to idle
     *
     */
    void stop() override;

    /**
     * @brief Immediately interrupt the current action, return the motor to idle
     * and disable it.
     *
     * Note: As an effect of stopping the motor as fast as possible, the real
     * motor position and the controller's step counter will lose synchronization.
     * Hence, calibration will be lost after calling this method.
     */
    void emergencyStop() override;

    /**
     * @brief Reset error state
     *
     * Depending on the event that caused the error, this will set the motor to
     * disabled\n (if power supply is disabled, e.g. after emergency stop) or idle
     * state (e.g. some \n action failed, but power supply is enabled).
     */
    void resetError() override;

    /**
     * @brief It transforms units in steps using the internal converter
     * @param units Value in unit to be converted in steps
     * @return Value in steps
     */
    int recalculateUnitsInSteps(float units) override;

    /**
     * @brief it transforms steps in units using the internal converter
     * @param steps Value in steps to be converted in units
     * @return Value in units
     */
    float recalculateStepsInUnits(int steps) override;

    /**
     * @brief enable/disable software position limits
     */
    ExitStatus setSoftwareLimitsEnabled(bool enabled) override;

    /**
     * @brief get software position limits enabled flag
     */
    bool getSoftwareLimitsEnabled() override;

    /**
     * @brief set maximum software limit in units
     * @param maxPos maximum limit
     */
    ExitStatus setMaxPositionLimit(float maxPos) override;

    /**
     * @brief set maximum software limit in steps
     * @param maxPos maximum limit
     */
    ExitStatus setMaxPositionLimitInSteps(int maxPos) override;

    /**
     * @brief set minimum software limit in units
     * @param minPos minimum limit
     */
    ExitStatus setMinPositionLimit(float minPos) override;

    /**
     * @brief set minimum software limit in steps
     * @param minPos maximum limit
     */
    ExitStatus setMinPositionLimitInSteps(int minPos) override;

    /**
     * @brief get maximal software position limit in units
     */
    float getMaxPositionLimit() override;

    /**
     * @brief get maximal software position limit in steps
     */
    int getMaxPositionLimitInSteps() override;

    /**
     * @brief get minimal software position limit in units
     */
    float getMinPositionLimit() override;

    /**
     * @brief get minimal software position limit in steps
     */
    int getMinPositionLimitInSteps() override;

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
    ExitStatus setActualPosition(float actualPositionInUnits) override;

    /**
     * @brief set actual position in steps of the motor respect to some reference
     * @param actualPositionInSteps In order to use the motor an absolute scale
     * must be defined.
     *
     * This can be done defining the position of the motor respect to an external
     * reference (actual position).
     */
    ExitStatus setActualPositionInSteps(int actualPositionInSteps) override;

    /**@brief translate the reference axis of the motor. This operation will
     * translate also the software limits
     * @param translationInSteps translation value in steps
     */

    ExitStatus translateAxisInSteps(int translationInSteps) override;

    /**@brief translate the reference axis of the motor. This operation will
     * translate also the software limits
     * @param translationInUnits translation value in unit
     */

    ExitStatus translateAxis(float translationInUnits) override;

    /**
     * @brief get current position of the motor in units
     */
    float getCurrentPosition() override;

    /**
     * @brief get current position of the motor in steps
     */
    int getCurrentPositionInSteps() override;

    /**
     * @brief Get the current position from encoder in user-defined units,
     *        according to the encoderUnitsConverter defined for the instance.
     */
    double getEncoderPosition() override;

    unsigned int getEncoderReadoutMode() override; ///< Get the decoder readout mode

    /**
     *  @brief Set the actual encoder position to a reference value.
     *
     *  Analogous to setActualPosition() which sets the motor driver's internal\n
     *  step counter to a reference value, this function can be used to define a
     * reference\n for the encoder output.
     */
    ExitStatus setActualEncoderPosition(double referencePosition) override;

    /**
     * Return target motor position in the arbitrary units.
     * @return float - target position of motor in arbitrary units.
     */
    float getTargetPosition() override;

    /**
     * Return target motor position in steps
     * @return int - target position of motor in steps
     */

    int getTargetPositionInSteps() override;

    // FIXME This can be constant after construction?
    /**
     * @brief set the steps-units converter. Per default each instance has a 1:1
     * converter
     */
    ExitStatus setStepperMotorUnitsConverter(
        std::shared_ptr<utility::MotorStepsConverter> stepperMotorUnitsConverter) override;

    // FIXME This can be constant after construction?
    /**
     * @brief set the steps-units converter to the default one
     */
    ExitStatus setStepperMotorUnitsConverterToDefault() override;

    /**
     * Check if the system is in idle and it returns true is it is so.
     * If in idle any action required by the user can be executed like move or
     * move relative.
     */
    bool isSystemIdle() override;

    /**
     * @brief actions are executed asynchronous, so using this function one can
     * block the program in case an action is being executed and waiting until
     * this will be terminated and the system is back to idle.
     */
    void waitForIdle() override;

    /**
     * @brief Returns the state of the stepper motor's state machine
     */
    std::string getState() override;

    /**
     * @brief Return error code for the motor. The error code is not a
     * bit-field.\n 0 - NO_ERROR\n 1 - ACTION_ERROR if any of the action started
     * by the user was not successful\n 2 - MOVE_INTERUPTED if the motion of the
     * motor was interrupted, e.g. a software/hardware limit was hit.\n 3 -
     * CALIBRATION_LOST if calibration is lost.\n
     */
    Error getError() override;

    /**
     * @brief function returns true if system is calibrated, false otherwise
     */
    bool isCalibrated() override;

    /**
     * @brief function returns time last calibration, 0 if no calibration was
     * performed about booting the hardware.
     */

    uint32_t getCalibrationTime() override;

    /**
     * @brief Enable/disable the motor.
     * @param enable If true, motor is enabled.
     */
    void setEnabled(bool enable) override;

    /**
     * @brief get motor enabled flag
     */
    bool getEnabled() override;

    /**
     * @brief Sets the autostart flag.
     *        Allows automatic start of movement on change of target position if
     * set to true.
     */
    void setAutostart(bool autostart) override;

    /**
     * @brief Returns the value of the autostart flag.
     *        If true, movement is initiated automatically on change of the target
     * position.
     */
    bool getAutostart() override;

    /**
     * @brief get maximal speed capability
     */
    double getMaxSpeedCapability() override;

    /**
     * @brief get safe current limit
     */
    double getSafeCurrentLimit() override;

    /**
     * @brief set user current limit for the motor
     * @param currentInAmps current limit in ampere
     */
    ExitStatus setUserCurrentLimit(double currentInAmps) override;

    /**
     * @brief returns the user current limit in ampere
     */
    double getUserCurrentLimit() override;

    /**
     * @brief set user speed limit in microsteps per second
     */
    ExitStatus setUserSpeedLimit(double speedInUstepsPerSec) override;

    /**
     * @brief return user speed limit in microsteps per secon.
     */
    double getUserSpeedLimit() override;

    /**
     * @brief Returns True if the motor is moving and false if at
     * standstill. This command is reliable as long as the motor is not
     * stalled.
     */
    bool isMoving() override;

    /**
     * @brief enabling full stepping movement.
     * The target position is rounded to the next full step value before it is
     * written to the register of the controller chip.
     *
     */
    void enableFullStepping(bool enable) override;

    /**
     * @brief returns true if motor is moving per full step, false otherwise
     */
    bool isFullStepping() override;

    bool hasHWReferenceSwitches() override;

    ExitStatus calibrate() override;

    ExitStatus determineTolerance() override;

    float getPositiveEndReference() override;

    int getPositiveEndReferenceInSteps() override;

    float getNegativeEndReference() override;

    int getNegativeEndReferenceInSteps() override;

    float getTolerancePositiveEndSwitch() override;

    float getToleranceNegativeEndSwitch() override;

    bool isPositiveReferenceActive() override;

    bool isNegativeReferenceActive() override;

    bool isPositiveEndSwitchEnabled() override;

    bool isNegativeEndSwitchEnabled() override;

    CalibrationMode getCalibrationMode() override;

    //    /friend class StateMachine;
    friend class ::StepperMotorChimeraTKFixture;

   protected:
    /**
     * StateMachine subclass specific to this implementation
     *
     * \dot
     * digraph {

    # To refactor nodes, place the cursor left to a node name
    initState -> disabled [label="initialEvent"]
    disabled -> idle [label = "enableEvent"];
    idle -> moving [label = "moveEvent"];
    idle -> disabled [label = "disableEvent"];
    moving -> idle [label = "stopEvent"];
    moving -> error [label = "errorEvent"];
    moving -> error [label = "emergencyStopEvent"];
    moving -> disabled [label = "disableEvent"];
    error -> idle [label = "resetToIdleEvent"];
    error -> disabled [label = "resetToDisabledEvent"];
    }
\enddot
     */
    class StateMachine : public utility::StateMachine {
      friend class BasicStepperMotor;

     public:
      explicit StateMachine(BasicStepperMotor& stepperMotor);
      ~StateMachine() override;

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
      State _moving{"moving"};
      State _idle{"idle"};
      State _disabled{"disabled"};
      State _error{"error"};
      BasicStepperMotor& _stepperMotor;
      boost::shared_ptr<ChimeraTK::MotorControler>& _motorControler;
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

    void initStateMachine();
    virtual bool limitsOK(int newPositionInSteps);
    virtual ExitStatus checkNewPosition(int newPositionInSteps);

    /// Checks if moving resulted in the requested target position
    virtual bool verifyMoveAction();

    /// Common actions for translateAxis for this and derived classes
    void translateAxisActions(int translationInSteps);

    boost::shared_ptr<ChimeraTK::MotorDriverCard> _motorDriverCard;
    boost::shared_ptr<ChimeraTK::MotorControler> _motorControler;

    std::shared_ptr<utility::MotorStepsConverter> _stepperMotorUnitsConverter;
    std::shared_ptr<utility::EncoderStepsConverter> _encoderUnitsConverter;

    int _encoderPositionOffset{0};
    std::atomic<int> _targetPositionInSteps{0};
    int _maxPositionLimitInSteps{std::numeric_limits<int>::max()};
    int _minPositionLimitInSteps{std::numeric_limits<int>::min()};
    bool _autostart{false};
    bool _softwareLimitsEnabled{false};

    mutable boost::mutex _mutex;
    std::shared_ptr<utility::StateMachine> _stateMachine;

    std::atomic<Error> _errorMode{Error::NO_ERROR};
    std::atomic<CalibrationMode> _calibrationMode{CalibrationMode::NONE};

  }; // class BasicStepperMotor
} // namespace ChimeraTK::MotorDriver
