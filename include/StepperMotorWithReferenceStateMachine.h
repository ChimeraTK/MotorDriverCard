/*
 * StepperMotorWithReferenceStateMachine.hh
 *
 *  Created on: Feb 15, 2017
 *      Author: vitimic
 */

#include "StepperMotorStateMachine.h"
#include <atomic>
#include <future>

#ifndef INCLUDE_STEPPERMOTORWITHREFERENCESTATEMACHINE_HH_
#define INCLUDE_STEPPERMOTORWITHREFERENCESTATEMACHINE_HH_

namespace ChimeraTK{

  class StepperMotorWithReference;

  class StepperMotorWithReferenceStateMachine : public StateMachine{
  public:
    StepperMotorWithReferenceStateMachine(StepperMotorWithReference &stepperMotorWithReference);
    virtual ~StepperMotorWithReferenceStateMachine();
    static Event calibEvent;
    static Event calcToleranceEvent;

    friend class StepperMotorWithReferenceTest;
  protected:
    State _calibrating;
    State _calculatingTolerance;
    State _interruptingAction;
    StepperMotorStateMachine _baseStateMachine;
    StepperMotorWithReference &_stepperMotorWithReference;
    std::future<void> _future;
    std::atomic<bool> _stopAction;
    std::atomic<bool> _moveInterrupted;
    enum Sign{NEGATIVE = -1, POSITIVE = 1};
    void actionStop();
    void getActionCompletedEvent();
    void actionStartCalib();
    void actionStartCalcTolercance();
    void calibrationThreadFunction();
    void toleranceCalcThreadFunction();
    void moveToEndSwitch(Sign sign);
    void findEndSwitch(Sign sign);
    bool isEndSwitchActive(Sign sign);
    double getToleranceEndSwitch(Sign sign);
    int getPositionEndSwitch(Sign sign);
  };

}


#endif /* INCLUDE_STEPPERMOTORWITHREFERENCESTATEMACHINE_HH_ */
