/*
 * StepperMotorWithReference.cc
 *
 *  Created on: Feb 17, 2017
 *      Author: vitimic
 */

#include "StepperMotorWithReference.h"

namespace ChimeraTK{
  StepperMotorWithReference::StepperMotorWithReference(std::string const & motorDriverCardDeviceName,
                                                       std::string const & moduleName,
                                                       unsigned int motorDriverId,
                                                       std::string motorDriverCardConfigFileName):
                                                	   StepperMotor(),
                                                	   _motorDriverCardDeviceName(motorDriverCardDeviceName),
                                                	   _motorDriverId(motorDriverId),
                                                	   _motorDriverCard( mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(
                                                	       motorDriverCardDeviceName, moduleName, motorDriverCardConfigFileName)),
                                                	   _motorControler(_motorDriverCard->getMotorControler(_motorDriverId)){
    createStateMachine();
  }

  StepperMotorWithReference::~StepperMotorWithReference(){}


}

