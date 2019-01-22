/*
 * StepperMotorWithReferenceStateMachine.hh
 *
 *  Created on: Feb 15, 2017
 *      Author: vitimic
 */

#include "StepperMotorStateMachine.h"
#include <atomic>


#ifndef INCLUDE_STEPPERMOTORWITHREFERENCESTATEMACHINE_HH_
#define INCLUDE_STEPPERMOTORWITHREFERENCESTATEMACHINE_HH_

class StepperMotorWithReferenceTestFixture;

namespace ChimeraTK{

  class StepperMotorWithReference;

  class StepperMotorWithReferenceStateMachine : public StepperMotorStateMachine{
  public:
    StepperMotorWithReferenceStateMachine(StepperMotorWithReference &stepperMotorWithReference);
    virtual ~StepperMotorWithReferenceStateMachine();

    static const Event calibEvent;
    static const Event calcToleranceEvent;

    friend class ::StepperMotorWithReferenceTestFixture;
  protected:
    State _calibrating;
    State _calculatingTolerance;
    StepperMotorWithReference& _motor;
    std::atomic<bool> _stopAction;
    std::atomic<bool> _moveInterrupted;
    enum Sign{NEGATIVE = -1, POSITIVE = 1};

    void actionStop();
    void actionStartCalib();
    void actionEndCallback();
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
