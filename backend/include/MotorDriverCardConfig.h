// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#ifndef MOTOR_DRIVER_CARD_CONFIG_H
#  define MOTOR_DRIVER_CARD_CONFIG_H

#  include "MotorControlerConfig.h"
#  include "TMC429Words.h"

#  include <vector>

namespace ChimeraTK {

  /** This configuration contains start values for all data words that
   *  can be written to the MotorDriverImpl class, and configurations for
   *  the MotorDrivers is contains.
   */
  struct MotorDriverCardConfig {
    unsigned int coverDatagram;
    CoverPositionAndLength coverPositionAndLength;
    unsigned int datagramHighWord;
    unsigned int datagramLowWord;
    InterfaceConfiguration interfaceConfiguration;
    PositionCompareInterruptData positionCompareInterruptData;
    unsigned int positionCompareWord;
    StepperMotorGlobalParameters stepperMotorGlobalParameters;

    unsigned int controlerSpiWaitingTime;

    std::vector<MotorControlerConfig> motorControlerConfigurations;

    MotorDriverCardConfig();

    bool operator==(MotorDriverCardConfig const& right) const;
  };
} // namespace ChimeraTK

#endif // MOTOR_DRIVER_CARD_CONFIG_H
