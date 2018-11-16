/*
 * StepperMotorStateMachine.h
 *
 *  Created on: Feb 2, 2017
 *      Author: vitimic
 */

#include "StateMachine.h"
#include "MotorControler.h"
#include <boost/shared_ptr.hpp> //TODO Move to stdlib
#include <boost/thread.hpp>

#include <memory>
#include <future>

#ifndef INCLUDE_STEPPERMOTORSTATEMACHINE_H_
#define INCLUDE_STEPPERMOTORSTATEMACHINE_H_


namespace ChimeraTK{
  class StepperMotor;
  class  StepperMotorStateMachine : public StateMachine{
  public:
    StepperMotorStateMachine(ChimeraTK::StepperMotor &stepperMotor);
    virtual ~StepperMotorStateMachine();
    static Event initialEvent;
    static Event moveEvent;
    static Event stopEvent;
    static Event emergencyStopEvent;
    static Event actionCompleteEvent;
    static Event enableEvent;
    static Event disableEvent;
  protected:
    State _moving;
    State _idle;
//    State _stop;
//    State _emergencyStop;
    State _disable;
    State _error;
    StepperMotor &_stepperMotor;
    boost::shared_ptr<mtca4u::MotorControler> &_motorControler;
    void getActionCompleteEvent();
    void actionWaitForStandstill();
    void waitForStandstill();
    void actionIdleToMove();
    void actionMovetoStop();
    void actionMoveToFullStep();
    void actionEnable();
    void actionDisable();
    void actionEmergencyStop();
  };
}
#endif /* INCLUDE_STEPPERMOTORSTATEMACHINE_H_ */
