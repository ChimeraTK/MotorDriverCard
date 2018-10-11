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

  class StepperMotorWithReference : public StepperMotor{
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
    StepperMotorWithReference(std::string const & motorDriverCardDeviceName,
                              std::string const & moduleName,
                              unsigned int motorDriverId,
                              std::string motorDriverCardConfigFileName,
                              std::shared_ptr<StepperMotorUnitsConverter> motorUnitsConverter = std::make_shared<StepperMotorUnitsConverterTrivia>(),
                              std::shared_ptr<StepperMotorUnitsConverter> encoderUnitsConverter = std::make_shared<StepperMotorUnitsConverterTrivia>());

    /**
     * @brief  Destructor of the class object
     */
    virtual ~StepperMotorWithReference();

    virtual void setActualPositionInSteps(int actualPositionInSteps);

    virtual void setActualPosition(float actualPosition);

    virtual void translateAxisInSteps(int translationInSteps);

    virtual void translateAxis(float translationInUnits);

    virtual void calibrate();

    virtual void determineTolerance();

    virtual StepperMotorError getError();

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

    friend class StepperMotorWithReferenceStateMachine;
    friend class StepperMotorWithReferenceTest;

  protected:

    virtual bool stateMachineInIdleAndNoEvent();
    virtual void createStateMachine();
    virtual bool limitsOK(int newPositionInSteps);
    virtual void resetMotorControlerAndCheckOverFlowSoftLimits(int translationInSteps);
    std::atomic<bool> _positiveEndSwitchEnabled;
    std::atomic<bool> _negativeEndSwitchEnabled;
    std::atomic<bool> _calibrationFailed;
    std::atomic<bool> _toleranceCalcFailed;
    std::atomic<bool> _toleranceCalculated;
    std::atomic<int> _calibNegativeEndSwitchInSteps;
    std::atomic<int> _calibPositiveEndSwitchInSteps;
    std::atomic<float> _tolerancePositiveEndSwitch;
    std::atomic<float> _toleranceNegativeEndSwitch;
    std::atomic<uint32_t> _index; //variable used only in tests. to be removed?
  };
}
#endif /* CHIMERATK_STEPPER_MOTOR_H */
