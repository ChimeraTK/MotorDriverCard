/*
 * StepperMotorStateMachine.h
 *
 *  Created on: Feb 2, 2017
 *      Author: vitimic
 */

#include "StateMachine.h"

#ifndef INCLUDE_STEPPERMOTORSTATEMACHINE_H_
#define INCLUDE_STEPPERMOTORSTATEMACHINE_H_


namespace ChimeraTK{
  class StepperMotor;
  class  StepperMotorStateMachine : public StateMachine{
  public:
    StepperMotorStateMachine(ChimeraTK::StepperMotor &stepperMotor);
    virtual ~StepperMotorStateMachine();
    static Event moveEvent;
    static Event waitEvent;
    static Event stopEvent;
    static Event actionCompleteEvent;
  protected:
    State _moving;
    State _idle;
    State _stop;
    StepperMotor &_stepperMotor;
    void getActionCompleteEvent();
    void actionIdleToMove();
    void actionMovetoStop();
    void actionToIdle();
    virtual bool propagateEvent();
  };
}
#endif /* INCLUDE_STEPPERMOTORSTATEMACHINE_H_ */
