// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "BasicStepperMotor.h"

class StepperMotorWithReferenceTestFixture;

namespace ChimeraTK::MotorDriver {

  /// Helper class for endswitch polarity
  enum class Sign { NEGATIVE = -1, POSITIVE = 1 };

  struct StepperMotorParameters;
  enum class ExitStatus;
  enum class Error;

  /**
   *  @brief This class provides the user interface for a linear stepper motor
   * stage with end switches.
   */
  class LinearStepperMotor : public BasicStepperMotor {
   public:
    /**
     * @brief  Constructor of the class object
     * @param  parameters Configuration parameters of type StepperMotorParameters
     */
    explicit LinearStepperMotor(const StepperMotorParameters&);

    /**
     * @brief  Destructor of the class object
     */
    ~LinearStepperMotor() override;

    /**
     *  @brief Simple calibration of the linear stage by defining the actual
     * position in steps.
     *
     *  This function only sets the actual position to a defined value. The end
     * switch positions will not be defined.\n This is useful in applications
     * where the stage can not move through the full range between the end
     * switches.\n This function will result in the calibration mode being
     * CalibrationMode::SIMPLE.\n
     */
    ExitStatus setActualPositionInSteps(int actualPositionInSteps) override;

    /**
     * @brief Translate axis of the linear stage by offset translationInSteps
     *
     * The actual position and calibrated values of the end switch positions are
     * shifted by the given offset. The resulting new position will be truncated
     * if the calculated value exceeds the numeric limits of an int.
     */
    ExitStatus translateAxisInSteps(int translationInSteps) override;

    bool hasHWReferenceSwitches() override;

    /**
     *  @brief Calibration of the linear stage.
     *
     *  This initiates a drive through the full range between both end switches
     * and determines their position.\n The negative end switch will be defined as
     * zero. The resulting coordinates can later be translated by the
     *  translateAxis() routines.\n
     *
     *  On success, this function will result in the calibration mode being
     * CalibrationMode::FULL.\n
     */
    ExitStatus calibrate() override;

    /**
     *  @brief Determines the standard deviation of the end switch position.
     *
     *  The standard deviation is determined by running into each end switch
     * multiple times.\n If this has been called on system setup, getError() will
     * return Error::CALIBRATION_LOST\n error when an end switch is activated and
     * the actual position is not within a 3 sigma band around the\n calibrated
     * end switch position.\n
     */
    ExitStatus determineTolerance() override;

    Error getError() override;

    float getPositiveEndReference() override;

    int getPositiveEndReferenceInSteps() override;

    float getNegativeEndReference() override;

    int getNegativeEndReferenceInSteps() override;

    float getTolerancePositiveEndSwitch() override;

    float getToleranceNegativeEndSwitch() override;

    /**
     *  Determines if the end switch has been hit.
     */
    bool isPositiveReferenceActive() override;

    /**
     *  Determines if the end switch has been hit.
     */
    bool isNegativeReferenceActive() override;

    /**
     *  Determines if the end switch is enabled (and will be activated when hit).
     */
    bool isPositiveEndSwitchEnabled() override;

    /**
     *  Determines if the end switch is enabled (and will be activated when hit).
     */
    bool isNegativeEndSwitchEnabled() override;

    /**
     *  @brief Returns the current calibration mode
     */
    CalibrationMode getCalibrationMode() override;

    // friend class utility::StepperMotorWithReferenceStateMachine;
    friend class ::StepperMotorWithReferenceTestFixture;

   protected:
    /**
     * StateMachine subclass specific to this implementation of the StepperMotor
     */
    class StateMachine : public BasicStepperMotor::StateMachine {
      friend class LinearStepperMotor;

     public:
      explicit StateMachine(LinearStepperMotor& stepperMotorWithReference);
      ~StateMachine() override = default;

      static const Event calibEvent;
      static const Event calcToleranceEvent;

      friend class ::StepperMotorWithReferenceTestFixture;

     protected:
      State _calibrating;
      State _calculatingTolerance;
      LinearStepperMotor& _motor;
      std::atomic<bool> _stopAction;
      std::atomic<bool> _moveInterrupted;

      void actionStop();
      void actionStartCalib();
      void actionEndCallback();
      void actionStartCalcTolercance();
      void calibrationThreadFunction();
      void toleranceCalcThreadFunction();
      void moveToEndSwitch(Sign sign);
      void findEndSwitch(Sign sign);
      double getToleranceEndSwitch(Sign sign);
      int getPositionEndSwitch(Sign sign);
    };

    bool motorActive() override;
    bool limitsOK(int newPositionInSteps) override;
    ExitStatus checkNewPosition(int newPositionInSteps) override;
    bool verifyMoveAction() override;

    /// Loads end switch calibration from the HW and sets the calibration mode
    /// accordingly.
    void loadEndSwitchCalibration();

    /// True if end switch is activated, checks for error
    bool isEndSwitchActive(Sign sign);

    std::atomic<bool> _positiveEndSwitchEnabled{false};
    std::atomic<bool> _negativeEndSwitchEnabled{false};
    std::atomic<bool> _calibrationFailed{false};
    std::atomic<bool> _toleranceCalcFailed{false};
    std::atomic<bool> _toleranceCalculated{false};
    std::atomic<int> _calibNegativeEndSwitchInSteps{-std::numeric_limits<int>::max()};
    std::atomic<int> _calibPositiveEndSwitchInSteps{std::numeric_limits<int>::max()};
    std::atomic<float> _tolerancePositiveEndSwitch{0};
    std::atomic<float> _toleranceNegativeEndSwitch{0};
  };
} // namespace ChimeraTK::MotorDriver
