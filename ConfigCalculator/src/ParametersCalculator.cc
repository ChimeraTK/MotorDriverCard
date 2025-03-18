// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ParametersCalculator.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>

const double ParametersCalculator::secondsPerMinute = 60.;
const double ParametersCalculator::nVMaxValues = 2048;
const double ParametersCalculator::maximumAllowedVMax = nVMaxValues - 1;
const double ParametersCalculator::nAMaxValues = 2048;
const double ParametersCalculator::maximumAllowedAMax = nAMaxValues - 1;
const double ParametersCalculator::minimumRecommendedAMax = 20;

// see page 23, block clk_div32
const double ParametersCalculator::systemClockPreDivider = 32;

// see section 9.14, page 29 (part of the 2^29)
const double ParametersCalculator::velocityAccumulationClockPreDivider = 256;

const double ParametersCalculator::minimumAllowedPMul = 128;
const double ParametersCalculator::maximumAllowedPMul = 255;

const double ParametersCalculator::minimumAllowedPDiv = 0;
const double ParametersCalculator::maximumAllowedPDiv = 13;

const double ParametersCalculator::minimumAllowedPulseDiv = 0;
const double ParametersCalculator::maximumAllowedPulseDiv = 13;

const double ParametersCalculator::minimumAllowedRampDiv = 0;
const double ParametersCalculator::maximumAllowedRampDiv = 13;

// 5 % as recommended in section 9.11 page 24
const double ParametersCalculator::reductionValue = 0.05;

const double ParametersCalculator::iMaxTMC260 = 1.8;
const double ParametersCalculator::nCurrentScaleValues = 32;
const double ParametersCalculator::minimumAllowedCurrentScale = 0;

ParametersCalculator::ChipParameters ParametersCalculator::calculateParameters(
    ParametersCalculator::PhysicalParameters physicalParameters) {
  std::list<std::string> warnings = inputcheck(physicalParameters);

  // this also checks the validity of microsteps
  unsigned int driverMicroStepValue = calculateDriverMicroStepValue(physicalParameters.microsteps);

  // safe because the validity of microsteps has been checked before
  unsigned int controllerMicroStepValue = static_cast<unsigned int>(log2(physicalParameters.microsteps));

  double targetMicrostepFrequency = physicalParameters.nStepsPerTurn // full steps per turn
      * physicalParameters.maxRPM / secondsPerMinute                 // motor rotation in Hz
      * physicalParameters.microsteps;                               // microsteps per full step

  double dividedSystemClockInHz = physicalParameters.systemClock * 1e6 / systemClockPreDivider;

  // I use double instead of unsigned to check for range overflows
  // (also int does not cut it because static_cast<int>(-0.8)
  // yields 0, but it should issue a warning
  double pulseDiv = log2(dividedSystemClockInHz / targetMicrostepFrequency * maximumAllowedVMax / nVMaxValues);

  // range check on pulseDiv (0..13)
  if(pulseDiv < minimumAllowedPulseDiv) {
    pulseDiv = minimumAllowedPulseDiv;
    warnings.push_back("pulseDiv was too small. Check you settings, you might "
                       "have to decrease maxRPM.");
  }

  // now that we know that pulseDiv is not negative
  //  "round" to the next lowest integet (as a cast to int would do).
  // This is essential for the following calculations
  pulseDiv = std::floor(pulseDiv);

  // after rounding the value has to be <= 13
  if(pulseDiv > maximumAllowedPulseDiv) {
    pulseDiv = maximumAllowedPulseDiv;
    warnings.push_back("pulseDiv was too large. Check you settings, you might "
                       "have to increase maxRPM.");
  }

  // This for instance would give the target frequency if
  // pulseDiv was not rounded down.
  double maxMirostepFrequencyWithThisPulseDiv =
      dividedSystemClockInHz * maximumAllowedVMax / nVMaxValues / pow(2, pulseDiv);

  int vMax = static_cast<int>(targetMicrostepFrequency / maxMirostepFrequencyWithThisPulseDiv * maximumAllowedVMax);

  if(vMax > maximumAllowedVMax) {
    std::stringstream warning;
    warning << "vMax was too high. The maximum possible RPM now is "
            << maximumAllowedVMax / vMax * physicalParameters.maxRPM << ". Adjust your input parameters.";
    warnings.push_back(warning.str());
    vMax = maximumAllowedVMax;
  }

  // this is only possible for insanely low speed, but we cannot
  // continue because the motor would never move
  if(vMax == 0) {
    throw std::out_of_range("vMax is calculated to 0. The motor will never move with these "
                            "settings. Check you input Parameters!");
  }

  double targetMicrostepAcceleration = targetMicrostepFrequency / physicalParameters.timeToVMax;

  double rampDiv = log2(dividedSystemClockInHz * dividedSystemClockInHz * maximumAllowedAMax /
      targetMicrostepAcceleration / pow(2, pulseDiv) / nAMaxValues / velocityAccumulationClockPreDivider);

  // range check on rampDiv (0..13)
  if(rampDiv < minimumAllowedRampDiv) {
    rampDiv = minimumAllowedRampDiv;
    warnings.push_back("rampDiv was too small. Check you settings, you might "
                       "have to increase timeToVMax.");
  }

  // now that we know that rampDiv is not negative
  //  "round" to the next lowest integet (as a cast to int would do).
  // This is essential for the following calculations
  rampDiv = std::floor(rampDiv);

  // after rounding the value has to be <= 13
  if(rampDiv > maximumAllowedRampDiv) {
    rampDiv = maximumAllowedRampDiv;
    warnings.push_back("rampDiv was too large. Check you settings, you might "
                       "have to decrease timeToVMax.");
  }

  double maxMicrostepAccelerationWithThisRampDiv = dividedSystemClockInHz * dividedSystemClockInHz *
      maximumAllowedAMax / nAMaxValues / velocityAccumulationClockPreDivider / pow(2, rampDiv + pulseDiv);

  double aMax = targetMicrostepAcceleration / maxMicrostepAccelerationWithThisRampDiv * maximumAllowedAMax;

  if(aMax > maximumAllowedAMax) {
    std::stringstream warning;
    warning << "aMax was too high. The time to reach VMax now is "
            << aMax / maximumAllowedAMax * physicalParameters.timeToVMax << ". Adjust your input parameters.";
    warnings.push_back(warning.str());
    aMax = maximumAllowedAMax;
  }

  if(aMax < minimumRecommendedAMax) {
    warnings.push_back("aMax is very low. Check your input parameters.");
  }

  // after rounding down 0 and negative values are not allowed
  if(aMax < 1) {
    throw std::out_of_range("aMax is calculated as 0. The motor would never move. Check your input "
                            "parameters (especially timeToVMax)!");
  }

  aMax = std::floor(aMax);

  double p = aMax / minimumAllowedPMul / pow(2, rampDiv - pulseDiv);

  double reducedP = (1. - reductionValue) * p;

  // This is the formula as calculated from the data sheet. Please leave
  // this block for reference, although a different calculation is used.
  //
  // Sorry for the hard coded 3. it is log2( maximumAllowedAMax / 256 )
  // where it is not clear where the 256 comes from. Yet another divider.
  // int pDiv =
  // static_cast<int>(log2( maximumAllowedPMul / reducedP ) - 3);

  // The formula above itself is not in the data sheet, they use the
  // method to calculate all pairs and pick the "right" combination
  // manually. This derrived formula leads to pmul being in the range
  // 127..254, while it has to be in 128..255. This is fixed by exchanging
  // maximumAllowedPMul=255 with 256, which simplifies the formula to
  double pDiv = log2(32 / reducedP);

  if(pDiv < minimumAllowedPDiv) {
    throw std::out_of_range("pDiv is too small and pMul too large. Check your "
                            "input parameters, expecially timeToVMax.");
  }

  // now that we know that pDiv is not negative round
  // to the next lowest int
  pDiv = std::floor(pDiv);

  if(pDiv > maximumAllowedPDiv) {
    throw std::out_of_range("pDiv is too large and pMul too small. Check your "
                            "input parameters, expecially timeToVMax.");
  }

  int pMul = static_cast<int>(reducedP * pow(2, pDiv + 3));

  double maxPossibleCurrent = std::min(iMaxTMC260, physicalParameters.iMax);

  // use floor to always get the current scale which is below or
  // equal to the maximim current, never above
  int currentScale = static_cast<int>(std::floor(maxPossibleCurrent / iMaxTMC260 * nCurrentScaleValues)) - 1;

  // For small currents < 1/32 iMaxTMC260 the calculated current scale now
  // is negative, which is invalid. Change it to 0 and add a warning.
  // There is nothing we can do about it, one has to allow some current in
  // the motor, it cannot be 0.
  if(currentScale < minimumAllowedCurrentScale) {
    currentScale = minimumAllowedCurrentScale;

    std::stringstream warning;
    warning << "Current scale at minimum."
            << " The configured maximal current of " << iMaxTMC260 / nCurrentScaleValues * (currentScale + 1)
            << " A is larger than the maximal coil current of " << physicalParameters.iMax << " A!";
    warnings.push_back(warning.str());
  }

  return ChipParameters(pulseDiv, rampDiv, aMax, vMax, pDiv, pMul, controllerMicroStepValue, driverMicroStepValue,
      currentScale, warnings);
}

unsigned int ParametersCalculator::calculateDriverMicroStepValue(double microsteps) {
  // the switch statement needs an int, not a double
  int microstepsInt = static_cast<int>(microsteps);
  if(microstepsInt != microsteps) {
    throw std::invalid_argument("microsteps has to an integer");
  }

  switch(microstepsInt) {
    case 1:
      return 8;
    case 2:
      return 7;
    case 4:
      return 6;
    case 8:
      return 5;
    case 16:
      return 4;
    case 32:
      return 3;
    case 64:
      return 2;
      // although 128 and 256 microsteps are valid for the driver
      // they are invalid here because the controller can only to 64
    default:
      throw std::invalid_argument("The number of microsteps has to be 2^i with i "
                                  "being an integer in the range 0..6.");
  }
}

std::list<std::string> ParametersCalculator::inputcheck(PhysicalParameters physicalParameters) {
  std::list<std::string> warnings;

  if((physicalParameters.systemClock > 32) || (physicalParameters.systemClock < 1)) {
    throw std::invalid_argument("systemClock must be in the range 1..32");
  }

  if(physicalParameters.iMax <= 0) {
    throw std::invalid_argument("iMax must be positive");
  }

  if(physicalParameters.maxRPM <= 0) {
    throw std::invalid_argument("maxRPM must be positive");
  }

  if(physicalParameters.nStepsPerTurn <= 0) {
    throw std::invalid_argument("nStepsPerTurn must be positive");
  }

  if(physicalParameters.timeToVMax <= 0) {
    throw std::invalid_argument("timeToVMax must be positive");
  }

  if(physicalParameters.iMax > iMaxTMC260) {
    warnings.push_back("iMax is too large. Current is limited to 1.8 A");
  }

  return warnings;
}
