/*
 * StepperMotorWithReference.h
 *
 *  Created on: Jan 17, 2017
 *      Author: vitimic
 */

#include "StepperMotor.h"
#include "MotorDriverCardFactory.h"
#include "MotorDriverException.h"
#include "StepperMotorException.h"

namespace ChimeraTK{

  class StepperMotorWithReference : public StepperMotor{
  public:
    /**
     * @brief  Constructor of the class object
     * @param  motorDriverCardDeviceName Name of the device in DMAP file
     * @param  moduleName Name of the module in the map file (there might be more than one MD22 per device/ FMC carrier).
     * @param  motorDriverId Each Motor Card Driver has two independent Motor Drivers (can drive two physical motors). ID defines which motor should be represented by this class instantiation
     * @param  motorDriverCardConfigFileName Name of configuration file
     * @return
     */
    StepperMotorWithReference(std::string const & motorDriverCardDeviceName,
                              std::string const & moduleName,
                              unsigned int motorDriverId,
                              std::string motorDriverCardConfigFileName);

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
  };
}
