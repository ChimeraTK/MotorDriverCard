/*
 * StepperMotorStateMachine.cc
 *
 *  Created on: Feb 2, 2017
 *      Author: vitimic
 */

#include"StepperMotorStateMachine.h"
#include"StateMachine.h"
#include <memory>
namespace chimeratk{
  StepperMotorStateMachine::StepperMotorStateMachine(StepperMotor &stepperMotor) : StateMachine<StepperMotor>(stepperMotor,
                                                                                                              "StepperMotorBaseAction",
                                                                                                              std::shared_ptr< StateMachinethis){

  }
}


