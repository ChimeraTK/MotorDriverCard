/*
 * StepperMotorStateMachine.h
 *
 *  Created on: Feb 2, 2017
 *      Author: vitimic
 */

#ifndef INCLUDE_STEPPERMOTORSTATEMACHINE_H_
#define INCLUDE_STEPPERMOTORSTATEMACHINE_H_

#include "StateMachine.h"
#include "MotorControler.h"
#include <boost/shared_ptr.hpp> //Kept for compatibilty with lower layers
#include <boost/thread.hpp>
#include <memory>


namespace ChimeraTK{

  class BasicStepperMotor;

  class  StepperMotorStateMachine : public StateMachine{

  public:
    StepperMotorStateMachine(BasicStepperMotor &stepperMotor);
    virtual ~StepperMotorStateMachine();
    static const Event initialEvent;
    static const Event moveEvent;
    static const Event stopEvent;
    static const Event errorEvent;
    static const Event emergencyStopEvent;
    static const Event actionCompleteEvent;
    static const Event enableEvent;
    static const Event disableEvent;
    static const Event resetToIdleEvent;
    static const Event resetToDisableEvent;
  protected:
    State _moving;
    State _idle;
    State _disabled;
    State _error;
    BasicStepperMotor &_stepperMotor;
    boost::shared_ptr<mtca4u::MotorControler> &_motorControler;
    void getActionCompleteEvent();
    void waitForStandstill();
    void actionIdleToMove();
    void actionMovetoStop();
    void actionMoveToFullStep();
    void actionEnable();
    void actionDisable();
    void actionEmergencyStop();
    void actionResetError();
  };
}
#endif /* INCLUDE_STEPPERMOTORSTATEMACHINE_H_ */
