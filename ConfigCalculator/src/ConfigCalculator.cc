#include "ConfigCalculator.h"

#include <stdexcept>

#include "TMC429Words.h"
#include "TMC260Words.h"

using namespace mtca4u;

MotorControlerConfig ConfigCalculator::calculateConfig( 
    ParametersCalculator::TMC429Parameters const & tmc429Parameters,
    EndSwitchConfig const & endSwitchConfig){

  // the return value
  MotorControlerConfig motorControlerConfig;

  motorControlerConfig.driverControlData.setMicroStepResolution( 
    tmc429Parameters.driverMicroStepValue);

  motorControlerConfig.stallGuardControlData.setCurrentScale(
    tmc429Parameters.currentScale );

  // This is cheating a bit. We know that the endSwitchConfig
  // already contains the right bit sequence for RampMode 0, which 
  // we want.
  // At least we do a check that the values are ok (an enum
  // can hold any value if you static_cast an in to it)
  checkEndSwitchConfig( endSwitchConfig );
  motorControlerConfig.referenceConfigAndRampModeData.setDATA(
    endSwitchConfig );

  motorControlerConfig.proportionalityFactorData.setDivisionParameter(
    tmc429Parameters.pDiv);
  //special treatment for pmul. The config only wants the lower 7 bits
  //Blank out the highest bit (bit 7), but leave the invalid bits
  // 16..31 for a range check
  motorControlerConfig.proportionalityFactorData.setMultiplicationParameter(
    tmc429Parameters.pMul & 0xFFFFFF7F);

  // the name is a bit length. get a variable for better readability
  DividersAndMicroStepResolutionData & dividersAndMicroStepResolutionData
    = motorControlerConfig.dividersAndMicroStepResolutionData;
  dividersAndMicroStepResolutionData.setMicroStepResolution(
    tmc429Parameters.controllerMicroStepValue);
  dividersAndMicroStepResolutionData.setRampDivider(
    tmc429Parameters.rampDiv);
  dividersAndMicroStepResolutionData.setPulseDivider(
    tmc429Parameters.pulseDiv);

  motorControlerConfig.maximumVelocity = tmc429Parameters.vMax;
  motorControlerConfig.maximumAcceleration = tmc429Parameters.aMax;

  return motorControlerConfig;
}

void ConfigCalculator::checkEndSwitchConfig(
  EndSwitchConfig const & endSwitchConfig){
  switch(endSwitchConfig){
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
