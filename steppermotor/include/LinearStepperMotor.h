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

namespace ChimeraTK {
namespace MotorDriver {

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
  LinearStepperMotor(const StepperMotorParameters &);

  /**
   * @brief  Destructor of the class object
   */
  virtual ~LinearStepperMotor();

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
  virtual ExitStatus setActualPositionInSteps(int actualPositionInSteps);

  /**
   * @brief Translate axis of the linear stage by offset translationInSteps
   *
   * The actual position and calibrated values of the end switch positions are
   * shifted by the given offset. The resulting new position will be truncated
   * if the calculated value exceeds the numeric limits of an int.
   */
  virtual ExitStatus translateAxisInSteps(int translationInSteps);

  virtual bool hasHWReferenceSwitches();

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
  virtual ExitStatus calibrate();

  /**
   *  @brief Determines the standard deviation of the end switch position.
   *
   *  The standard deviation is determined by running into each end switch
   * multiple times.\n If this has been called on system setup, getError() will
   * return Error::CALIBRATION_LOST\n error when an end switch is activated and
   * the actual position is not within a 3 sigma band around the\n calibrated
   * end switch position.\n
   */
  virtual ExitStatus determineTolerance();

  virtual Error getError();

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
  virtual CalibrationMode getCalibrationMode();

  // friend class utility::StepperMotorWithReferenceStateMachine;
  friend class ::StepperMotorWithReferenceTestFixture;

protected:
  /**
   * StateMachine subclass specific to this implementation of the StepperMotor
   */
  class StateMachine : public BasicStepperMotor::StateMachine {

    friend class LinearStepperMotor;

  public:
    StateMachine(LinearStepperMotor &stepperMotorWithReference);
    virtual ~StateMachine();

    static const Event calibEvent;
    static const Event calcToleranceEvent;

    friend class ::StepperMotorWithReferenceTestFixture;

  protected:
    State _calibrating;
    State _calculatingTolerance;
    LinearStepperMotor &_motor;
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

  virtual bool motorActive();
  virtual bool limitsOK(int newPositionInSteps);
  virtual ExitStatus checkNewPosition(int newPositionInSteps);
  virtual bool verifyMoveAction();

  /// Loads end switch calibration from the HW and sets the calibration mode
  /// accordingly.
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
} // namespace MotorDriver
} // namespace ChimeraTK
#endif /* CHIMERATK_LINEAR_STEPPER_MOTOR_H */
