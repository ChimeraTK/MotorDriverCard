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

    void setActualPositionInSteps(int actualPositionInSteps);

    void calibrate();

    void determineTolerance();

    StepperMotorError getError();

    float getPositiveEndReference();

    int getPositiveEndReferenceInSteps();

    float getNegativeEndReference();

    int getNegativeEndReferenceInSteps();

    float getTolerancePositiveEndSwitch();

    float getToleranceNegativeEndSwitch();

    bool isPositiveSwitchActive();

    bool isNegativeSwitchActive();

    friend class StepperMotorWithReferenceStateMachine;

  protected:

    bool stateMachineInIdleAndNoEvent();
    void createStateMachine();
    bool limitsOK(int newPositionInSteps);
    bool _positiveEndSwitchEnabled;
    bool _negativeEndSwitchEnabled;
    std::atomic<bool> _calibrationFailed;
    std::atomic<bool> _toleranceCalcFailed;
    std::atomic<bool> _toleranceCalculated;
    int _calibNegativeEndSwitchInSteps;
    int _calibPositiveEndSwitchInSteps;
    float _tolerancePositiveEndSwitch;
    float _toleranceNegativeEndSwitch;
  };
}
