// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "TMC260Words.h"
#include "TMC429Words.h"

namespace ChimeraTK::MotorDriver {
  /** This configuration contains start values for all data words that
   *  can be written to the MotorController class. Please note that the IDX field
   *  of the TMC429 words are ignored. It is replaced by the motor ID when the
   *  settings are applied, so settings can be reused for several motors.
   */
  struct MotorControllerConfig {
    AccelerationThresholdData accelerationThresholdData;
    //   int actualPosition;
    ChopperControlData chopperControlData;
    CoolStepControlData coolStepControlData;
    // NOLINTNEXTLINE(modernize-use-default-member-init) the enum lives in MotorController.h, which is not included here
    uint32_t decoderReadoutMode;
    DividersAndMicroStepResolutionData dividersAndMicroStepResolutionData;
    DriverConfigData driverConfigData;
    DriverControlData driverControlData;
    bool enabled{false};
    InterruptData interruptData;
    uint32_t maximumAcceleration;
    uint32_t maximumVelocity;
    unsigned int microStepCount{0};
    uint32_t minimumVelocity;
    unsigned int positionTolerance{0};
    ProportionalityFactorData proportionalityFactorData;
    ReferenceConfigAndRampModeData referenceConfigAndRampModeData;
    StallGuardControlData stallGuardControlData;
    int targetPosition{0}; // want it? need it in the config?
    int targetVelocity{0}; // want it? need it in the config?

    unsigned int driverSpiWaitingTime; //< in microseconds

    /** The constructor sets reasonable default values to all data words.
     */
    MotorControllerConfig();

    bool operator==(MotorControllerConfig const& right) const;
    bool operator!=(MotorControllerConfig const& right) const;
  };

} // namespace ChimeraTK::MotorDriver
