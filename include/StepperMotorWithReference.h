/*
 * StepperMotorWithReference.h
 *
 *  Created on: Jan 17, 2017
 *      Author: vitimic
 */

#include "StepperMotor.h"


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

    /**
     * @brief overloaded method from StepperMotor
     */
    virtual void moveToPosition(float newPosition);

    /**
     * @brief overloaded method from StepperMotor
     */
    virtual void moveToPositionInSteps(int newPositionInSteps);

    virtual void calibrate();

    void determineTolerance();

    bool isCalibrated();

    int getPositiveEndReference();

    float getPositiveEndReferenceInSteps();

    int getNegativeEndReference();

    float getNegativeEndReferenceInSteps();

    float getTolerance();

  protected:

    bool stateMachineInIdleAndNoEvent();
    void createStateMachine();
  };
}
