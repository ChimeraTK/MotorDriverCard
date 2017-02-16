/*
 * StepperMotorStateMachine.h
 *
 *  Created on: Feb 2, 2017
 *      Author: vitimic
 */

#include "StateMachine.h"

#ifndef INCLUDE_STEPPERMOTORSTATEMACHINE_H_
#define INCLUDE_STEPPERMOTORSTATEMACHINE_H_

class StepperMotor;

namespace ChimeraTK{
  class  StepperMotorStateMachine : public StateMachine{
  public:
    StepperMotorStateMachine(StepperMotor &stepperMotor);
    virtual ~StepperMotorStateMachine();
    //virtual void processEvent();
    //virtual State* performTransition(Event event);
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
