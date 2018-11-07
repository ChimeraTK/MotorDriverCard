/*
 * StepperMotorWithReference.h
 *
 *  Created on: Jan 17, 2017
 *      Author: vitimic
 */

#ifndef CHIMERATK_STEPPER_MOTOR_H
#define CHIMERATK_STEPPER_MOTOR_H

#include "StepperMotor.h"
#include "MotorDriverCardFactory.h"
#include "MotorDriverException.h"
#include "StepperMotorException.h"

#include <memory>

namespace ChimeraTK{

  /**
   *  @brief Enum type describing how the system is calibrated:
   *
   *  0 - NONE:   No calibration has been determined or it has been lost.\n
   *  1 - SIMPLE: The motor position has been defined by setActualPosition().\n
   *              No valid position for the end switches can be determined. Useful if driving in full range between the end switches is possible.\n
   *  2 - FULL:   The motor has been calibrated by calling calibrate(). The end switch position has been determined.\n
   */
  enum class StepperMotorCalibrationMode{NONE, SIMPLE, FULL};

  /**
   *  @brief This class provides the user interface for a linear stepper motor stage with end switches.
   */
  class StepperMotorWithReference : public StepperMotor{
  public:
    /**
     * @brief  Constructor of the class object
     * @param  motorDriverCardDeviceName Name of the device in DMAP file
     * @param  moduleName Name of the module in the map file (there might be more than one MD22 per device/ FMC carrier).
     * @param  motorDriverId Each Motor Card Driver has two independent Motor Drivers (can drive two physical motors). ID defines which motor should be represented by this class instantiation
     * @param  motorDriverCardConfigFileName Name of configuration file
     * @param  motorUnitsConverter A converter between motor steps and user unit. Based on the abstract class StepperMotorUnitsConverter. Defaults to a 1:1 converter between units and steps.
     * @param  @param  encoderUnitsToStepsRatio Ratio between user position unit ad encoder steps. Defaults to a 1.
     * @return
     */
    StepperMotorWithReference(std::string const & motorDriverCardDeviceName,
                              std::string const & moduleName,
                              unsigned int motorDriverId,
                              std::string motorDriverCardConfigFileName,
                              std::shared_ptr<StepperMotorUnitsConverter> motorUnitsConverter = std::make_shared<StepperMotorUnitsConverterTrivia>(),
                              double encoderUnitToStepsRatio = 1.0);

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
    virtual void setActualPositionInSteps(int actualPositionInSteps);

    /**
     *  @brief Simple calibration of the linear stage by defining the actual position in user-defined units.
     *
     *  See: setActualPositionInSteps()
     */
    virtual void setActualPosition(float actualPosition);

    virtual void translateAxisInSteps(int translationInSteps);

    virtual void translateAxis(float translationInUnits);

    /**
     *  @brief Calibration of the linear stage.
     *
     *  This initiates a drive through the full range between both end switches and determines their position.\n
     *  The negative end switch will be defined as zero. The resulting coordinates can later be translated by the
     *  translateAxis() routines.\n
     *
     *  On success, this function will result in the calibration mode being StepperMotorCalibrationMode::FULL.\n
     */
    virtual void calibrate();

    /**
     *  @brief Determines the standard deviation of the end switch position.
     *
     *  The standard deviation is determined by running into each end switch multiple times.\n
     *  If this has been called on system setup, getError() will return StepperMotorError::CALIBRATION_LOST\n
     *  error when an end switch is activated and the actual position is not within a 3 sigma band around the\n
     *  calibrated end switch position.\n
     */
    virtual void determineTolerance();

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
    friend class StepperMotorWithReferenceTest;

  protected:
    virtual bool limitsOK(int newPositionInSteps);
    virtual void resetMotorControlerAndCheckOverFlowSoftLimits(int translationInSteps);
    std::atomic<bool> _positiveEndSwitchEnabled;
    std::atomic<bool> _negativeEndSwitchEnabled;
    std::atomic<bool> _calibrationFailed;
    std::atomic<bool> _toleranceCalcFailed;
    std::atomic<bool> _toleranceCalculated;
    std::atomic<int> _calibNegativeEndSwitchInSteps;
    std::atomic<int> _calibPositiveEndSwitchInSteps;
    std::atomic<StepperMotorCalibrationMode> _calibrationMode;
    std::atomic<float> _tolerancePositiveEndSwitch;
    std::atomic<float> _toleranceNegativeEndSwitch;
    std::atomic<uint32_t> _index; //variable used only in tests. to be removed?
  };
}
#endif /* CHIMERATK_STEPPER_MOTOR_H */
