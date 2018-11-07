/*
 * StepperMotorStateMachine.h
 *
 *  Created on: Feb 2, 2017
 *      Author: vitimic
 */

#include "StateMachine.h"
#include "MotorControler.h"
#include <boost/shared_ptr.hpp>

#include <future>

#ifndef INCLUDE_STEPPERMOTORSTATEMACHINE_H_
#define INCLUDE_STEPPERMOTORSTATEMACHINE_H_


namespace ChimeraTK{
  class StepperMotor;
  class  StepperMotorStateMachine : public StateMachine{
  public:
    StepperMotorStateMachine(ChimeraTK::StepperMotor &stepperMotor);
    virtual ~StepperMotorStateMachine();
    static Event moveEvent;
    static Event stopEvent;
    static Event emergencyStopEvent;
    static Event actionCompleteEvent;
    static Event disableEvent;
  protected:
    State _moving;
    State _idle;
    State _stop;
    State _emergencyStop;
    State _disable;
    StepperMotor &_stepperMotor;
    boost::shared_ptr<mtca4u::MotorControler> &_motorControler;
    std::future<void> _future;
    void getActionCompleteEvent();
    void actionWaitForStandstill();
    void waitForStandstillThreadFunction();
    void actionIdleToMove();
    void actionMovetoStop();
    void actionMoveToFullStep();
    void actionToIdle();
    void actionDisable();
    void actionEmergencyStop();
  };
}
#endif /* INCLUDE_STEPPERMOTORSTATEMACHINE_H_ */
