// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "MotorDriverCardConfig.h"

#include "MotorDriverCardConfigDefaults.h"

namespace ChimeraTK::MotorDriver {

  MotorDriverCardConfig::MotorDriverCardConfig()
  : coverPositionAndLength(0), interfaceConfiguration(INTERFACE_CONFIGURATION_DEFAULT), positionCompareInterruptData(0),
    stepperMotorGlobalParameters(STEPPER_MOTOR_GLOBAL_PARAMETERS_DEFAULT),
    controllerSpiWaitingTime(CONTROLLER_SPI_WAITING_TIME_DEFAULT),
    motorControllerConfigurations(dfmc_md22::N_MOTORS_MAX) // initialises the motor controller configs
                                                           // with their default constructors
  {}

  bool MotorDriverCardConfig::operator==(MotorDriverCardConfig const& right) const {
    if(coverDatagram != right.coverDatagram) {
      return false;
    }
    if(coverPositionAndLength != right.coverPositionAndLength) {
      return false;
    }
    if(datagramHighWord != right.datagramHighWord) {
      return false;
    }
    if(datagramLowWord != right.datagramLowWord) {
      return false;
    }
    if(interfaceConfiguration != right.interfaceConfiguration) {
      return false;
    }
    if(positionCompareInterruptData != right.positionCompareInterruptData) {
      return false;
    }
    if(positionCompareWord != right.positionCompareWord) {
      return false;
    }
    if(stepperMotorGlobalParameters != right.stepperMotorGlobalParameters) {
      return false;
    }
    if(controllerSpiWaitingTime != right.controllerSpiWaitingTime) {
      return false;
    }

    for(size_t i = 0; i < motorControllerConfigurations.size(); ++i) {
      if(motorControllerConfigurations[i] != right.motorControllerConfigurations[i]) {
        return false;
      }
    }

    return true;
  }

} // namespace ChimeraTK::MotorDriver
