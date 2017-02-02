/*
 * StepperMotorStateMachine.h
 *
 *  Created on: Feb 2, 2017
 *      Author: vitimic
 */

#include "StepperMotor.h"
#include "StateMachine.h"

#ifndef INCLUDE_STEPPERMOTORSTATEMACHINE_H_
#define INCLUDE_STEPPERMOTORSTATEMACHINE_H_

namespace chimeratk{
  class StepperMotor;

  class  StepperMotorStateMachine : public StateMachine<StepperMotor>{
  public:
    StepperMotorStateMachine(StepperMotor &steppermotor, std::shared_ptr< StateMachine<StepperMotor>  > parentStateMachine);
    StepperMotorStateMachine(StepperMotor &steppermotor);
    virtual ~StepperMotorStateMachine();
  protected:
    State<StepperMotor> _moving;
    State<StepperMotor> _movingInDeltaSteps;
    State<StepperMotor> _stopMotor;
    State<StepperMotor> _idle;
  };
}


#endif /* INCLUDE_STEPPERMOTORSTATEMACHINE_H_ */
