// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "MotorControllerConfig.h"
#include "TMC429Words.h"

#include <vector>

namespace ChimeraTK::MotorDriver {

  /** This configuration contains start values for all data words that
   *  can be written to the MotorDriverImpl class, and configurations for
   *  the MotorDrivers is contains.
   */
  struct MotorDriverCardConfig {
    unsigned int coverDatagram{0};
    CoverPositionAndLength coverPositionAndLength;
    unsigned int datagramHighWord{0};
    unsigned int datagramLowWord{0};
    InterfaceConfiguration interfaceConfiguration;
    PositionCompareInterruptData positionCompareInterruptData;
    unsigned int positionCompareWord{0};
    StepperMotorGlobalParameters stepperMotorGlobalParameters;

    unsigned int controllerSpiWaitingTime;

    std::vector<MotorControllerConfig> motorControllerConfigurations;

    MotorDriverCardConfig();

    bool operator==(MotorDriverCardConfig const& right) const;
  };
} // namespace ChimeraTK::MotorDriver
