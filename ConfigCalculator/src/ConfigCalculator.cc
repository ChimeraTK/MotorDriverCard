// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ConfigCalculator.h"

#include "TMC260Words.h"
#include "TMC429Words.h"

#include <sstream>
#include <stdexcept>
using namespace ChimeraTK::MotorDriver;

using namespace ChimeraTK;

MotorControllerConfig ConfigCalculator::calculateConfig(
    ParametersCalculator::ChipParameters const& chipParameters, EndSwitchConfig const& endSwitchConfig) {
  // the return value
  MotorControllerConfig motorControllerConfig;

  motorControllerConfig.driverControlData.setMicroStepResolution(chipParameters.driverMicroStepValue);

  motorControllerConfig.stallGuardControlData.setCurrentScale(chipParameters.currentScale);

  // This is cheating a bit. We know that the endSwitchConfig
  // already contains the right bit sequence for RampMode 0, which
  // we want.
  // At least we do a check that the values are ok (an enum
  // can hold any value if you static_cast an in to it)
  checkEndSwitchConfig(endSwitchConfig);
  motorControllerConfig.referenceConfigAndRampModeData.setDATA(endSwitchConfig);

  motorControllerConfig.proportionalityFactorData.setDivisionParameter(chipParameters.pDiv);
  // special treatment for pmul. The config only wants the lower 7 bits
  // Blank out the highest bit (bit 7), but leave the invalid bits
  // 16..31 for a range check
  motorControllerConfig.proportionalityFactorData.setMultiplicationParameter(chipParameters.pMul & 0xFFFFFF7F);

  // the name is a bit length. get a variable for better readability
  DividersAndMicroStepResolutionData& dividersAndMicroStepResolutionData =
      motorControllerConfig.dividersAndMicroStepResolutionData;
  dividersAndMicroStepResolutionData.setMicroStepResolution(chipParameters.controllerMicroStepValue);
  dividersAndMicroStepResolutionData.setRampDivider(chipParameters.rampDiv);
  dividersAndMicroStepResolutionData.setPulseDivider(chipParameters.pulseDiv);

  motorControllerConfig.maximumVelocity = chipParameters.vMax;
  motorControllerConfig.maximumAcceleration = chipParameters.aMax;

  return motorControllerConfig;
}

void ConfigCalculator::checkEndSwitchConfig(EndSwitchConfig const& endSwitchConfig) {
  switch(endSwitchConfig) {
    case USE_BOTH:
    case IGNORE_NEGATIVE:
    case IGNORE_POSITIVE:
    case IGNORE_BOTH:
      return;
    default:
      std::stringstream errorMessage;
      errorMessage << endSwitchConfig << " is not a valid EndSwitchConfig!";
      throw std::invalid_argument(errorMessage.str());
  }
}
