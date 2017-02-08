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

namespace ChimeraTK{
  class StepperMotor;
  class StateIdle;
  class SateMoving;
  class StateDeltaMoving;

  class  StepperMotorStateMachine : public StateMachine<StepperMotor>{
  public:
    StepperMotorStateMachine(StepperMotor &stepperMotor);
    virtual ~StepperMotorStateMachine();
    virtual void processEvent();
    virtual void init();
    virtual void isTerminated();
  protected:
    SateMoving _moving;
    StateDeltaMoving _deltaMoving;
    StateIdle _idle;
  };

  class StateIdle : public State<StepperMotor>{
  public:
    StateIdle(StepperMotor &stepperMotor);
    ~StateIdle();
  private:
    void eventGenerationForIdle();
    void actionToMoveState();
    void actionToMoveDeltaState();
  };

  class StateMoving : public State<StepperMotor>{
  public:
  StateMoving(StepperMotor &stepperMotor);
  ~StateMoving();
  private:
  void eventGenerationForMoving();
  void actioToIdle();
  };

  class StateDeltaMoving: public State<StepperMotor>{
  public:
    StateDeltaMoving(StepperMotor &stepperMotor);
    ~StateDeltaMoving();
  private:
    void eventGenerationForMoving();
    void actionToIdle();
  };

}
#endif /* INCLUDE_STEPPERMOTORSTATEMACHINE_H_ */
