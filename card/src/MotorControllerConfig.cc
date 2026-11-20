// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "MotorControllerConfig.h"

#include "MotorController.h"
#include "MotorControllerConfigDefaults.h"

namespace ChimeraTK::MotorDriver {
  MotorControllerConfig::MotorControllerConfig()
  : accelerationThresholdData(0), // actualPosition(0),
    chopperControlData(CHOPPER_CONTROL_DEFAULT), coolStepControlData(COOL_STEP_CONTROL_DEFAULT),
    decoderReadoutMode(MotorController::DecoderReadoutMode::INCREMENTAL),
    dividersAndMicroStepResolutionData(DIVIDIERS_AND_MICRO_STEP_RESOLUTION_DEFAULT),
    driverConfigData(DRIVER_CONFIG_DEFAULT), driverControlData(DRIVER_CONTROL_DEFAULT), interruptData(0),
    maximumAcceleration(MAXIMUM_ACCELERATION_DEFAULT), maximumVelocity(MAXIMUM_VELOCITY_DEFAULT),
    minimumVelocity(MINIMUM_VELOCITY_DEFAULT), proportionalityFactorData(PROPORTIONALITY_FACTORS_DEFAULT),
    referenceConfigAndRampModeData(REFERENCE_CONFIG_AND_RAMP_MODE_DEFAULT),
    stallGuardControlData(STALL_GUARD_CONTROL_DEFAULT), driverSpiWaitingTime(DRIVER_SPI_WAITING_TIME_DEFAULT) {}

  bool MotorControllerConfig::operator==(MotorControllerConfig const& right) const {
    return ((accelerationThresholdData == right.accelerationThresholdData) &&
        // (actualPosition			  == right.actualPosition) &&
        (chopperControlData == right.chopperControlData) && (coolStepControlData == right.coolStepControlData) &&
        (decoderReadoutMode == right.decoderReadoutMode) &&
        (dividersAndMicroStepResolutionData == right.dividersAndMicroStepResolutionData) &&
        (driverConfigData == right.driverConfigData) && (driverControlData == right.driverControlData) &&
        (enabled == right.enabled) && (interruptData == right.interruptData) &&
        (maximumAcceleration == right.maximumAcceleration) && (maximumVelocity == right.maximumVelocity) &&
        (microStepCount == right.microStepCount) && (minimumVelocity == right.minimumVelocity) &&
        (positionTolerance == right.positionTolerance) &&
        (proportionalityFactorData == right.proportionalityFactorData) &&
        (referenceConfigAndRampModeData == right.referenceConfigAndRampModeData) &&
        (stallGuardControlData == right.stallGuardControlData) && (targetPosition == right.targetPosition) &&
        (targetVelocity == right.targetVelocity) && (driverSpiWaitingTime == right.driverSpiWaitingTime));
  }

  bool MotorControllerConfig::operator!=(MotorControllerConfig const& right) const {
    return !(*this == right);
  }
} // namespace ChimeraTK::MotorDriver
