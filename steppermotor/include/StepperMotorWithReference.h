/*
 * StepperMotorWithReference.h
 *
 *  Created on: Jan 17, 2017
 *      Author: vitimic
 */

#ifndef CHIMERATK_LINEAR_STEPPER_MOTOR_H
#define CHIMERATK_LINEAR_STEPPER_MOTOR_H

#include "BasicStepperMotor.h"

#include <memory>

class StepperMotorWithReferenceTestFixture;

namespace ChimeraTK{

  /// Helper class for endswitch polarity
  enum class Sign{NEGATIVE = -1, POSITIVE = 1};

  struct StepperMotorParameters;
  enum class StepperMotorRet;
  enum class StepperMotorError;

  /**
   *  @brief This class provides the user interface for a linear stepper motor stage with end switches.
   */
  class StepperMotorWithReference : public BasicStepperMotor{
  public:
    /**
     * @brief  Constructor of the class object
     * @param  parameters Configuration parameters of type StepperMotorParameters
     */
    StepperMotorWithReference(const StepperMotorParameters &);

    /**
     * @brief  Destructor of the class object
     */
    virtual ~StepperMotorWithReference();

    /**
     *  @brief Simple calibration of the linear stage by defining the actual position in steps.
     *
     *  This function only sets the actual position to a defined value. The end switch positions will not be defined.\n
     *  This is useful in applications where the stage can not move through the full range between the end switches.\n
     *  This function will result in the calibration mode being StepperMotorCalibrationMode::SIMPLE.\n
     */
    virtual StepperMotorRet setActualPositionInSteps(int actualPositionInSteps);


    /**
     * @brief Translate axis of the linear stage by offset translationInSteps
     *
     * The actual position and calibrated values of the end switch positions are shifted by
     * the given offset. The resulting new position will be truncated if the calculated value
     * exceeds the numeric limits of an int.
     */
    virtual StepperMotorRet translateAxisInSteps(int translationInSteps);


    virtual bool hasHWReferenceSwitches();

    /**
     *  @brief Calibration of the linear stage.
     *
     *  This initiates a drive through the full range between both end switches and determines their position.\n
     *  The negative end switch will be defined as zero. The resulting coordinates can later be translated by the
     *  translateAxis() routines.\n
     *
     *  On success, this function will result in the calibration mode being StepperMotorCalibrationMode::FULL.\n
     */
    virtual StepperMotorRet calibrate();

    /**
     *  @brief Determines the standard deviation of the end switch position.
     *
     *  The standard deviation is determined by running into each end switch multiple times.\n
     *  If this has been called on system setup, getError() will return StepperMotorError::CALIBRATION_LOST\n
     *  error when an end switch is activated and the actual position is not within a 3 sigma band around the\n
     *  calibrated end switch position.\n
     */
    virtual StepperMotorRet determineTolerance();

    virtual StepperMotorError getError();

    virtual float getPositiveEndReference();

    virtual int getPositiveEndReferenceInSteps();

    virtual float getNegativeEndReference();

    virtual int getNegativeEndReferenceInSteps();

    virtual float getTolerancePositiveEndSwitch();

    virtual float getToleranceNegativeEndSwitch();

    /**
     *  Determines if the end switch has been hit.
     */
    virtual bool isPositiveReferenceActive();

    /**
     *  Determines if the end switch has been hit.
     */
    virtual bool isNegativeReferenceActive();

    /**
     *  Determines if the end switch is enabled (and will be activated when hit).
     */
    virtual bool isPositiveEndSwitchEnabled();

    /**
     *  Determines if the end switch is enabled (and will be activated when hit).
     */
    virtual bool isNegativeEndSwitchEnabled();

    /**
     *  @brief Returns the current calibration mode
     */
    virtual StepperMotorCalibrationMode getCalibrationMode();

    friend class StepperMotorWithReferenceStateMachine;
    friend class ::StepperMotorWithReferenceTestFixture;

  protected:
    virtual bool motorActive();
    virtual bool limitsOK(int newPositionInSteps);
    virtual StepperMotorRet checkNewPosition(int newPositionInSteps);
    virtual bool verifyMoveAction();


    /// Loads end switch calibration from the HW and sets the calibration mode accordingly.
    virtual void loadEndSwitchCalibration();

    /// True if end switch is activated, checks for error
    bool isEndSwitchActive(Sign sign);

    std::atomic<bool> _positiveEndSwitchEnabled;
    std::atomic<bool> _negativeEndSwitchEnabled;
    std::atomic<bool> _calibrationFailed;
    std::atomic<bool> _toleranceCalcFailed;
    std::atomic<bool> _toleranceCalculated;
    std::atomic<int> _calibNegativeEndSwitchInSteps;
    std::atomic<int> _calibPositiveEndSwitchInSteps;
    std::atomic<float> _tolerancePositiveEndSwitch;
    std::atomic<float> _toleranceNegativeEndSwitch;
  };
}
#endif /* CHIMERATK_LINEAR_STEPPER_MOTOR_H */
