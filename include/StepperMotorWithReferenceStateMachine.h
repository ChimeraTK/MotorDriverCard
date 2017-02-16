/*
 * StepperMotorWithReferenceStateMachine.hh
 *
 *  Created on: Feb 15, 2017
 *      Author: vitimic
 */

#include "StepperMotorStateMachine.h"
#include "StepperMotorWithReference.h"

#ifndef INCLUDE_STEPPERMOTORWITHREFERENCESTATEMACHINE_HH_
#define INCLUDE_STEPPERMOTORWITHREFERENCESTATEMACHINE_HH_

namespace ChimeraTK{

  class StepperMotorWithReferenceStateMachine : public StateMachine{
  public:
    StepperMotorWithReferenceStateMachine(StepperMotorWithReference stepperMotorWithReference);
    virtual ~StepperMotorWithReferenceStateMachine();
    virtual void processEvent();
    static Event calibEvent;
    static Event calcToleranceEvent;
  protected:
    State _calibration;
    State _tolerance;
    StepperMotorStateMachine _baseStateMachine;
    StepperMotorWithReference _stepperMotorWithReference;
    Event getBaseStateMachineEvent();
    Event getCalibCompletedEvent();
    Event getToleranceEvent();
    void actionStartCalib();
    void actionStartCalcTolerance();
    void processSubStateMachine();
  };

}


#endif /* INCLUDE_STEPPERMOTORWITHREFERENCESTATEMACHINE_HH_ */
