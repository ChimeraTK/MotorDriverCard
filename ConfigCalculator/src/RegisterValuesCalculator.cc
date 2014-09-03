#include "RegisterValuesCalculator.h"

#include "TMC429Words.h"
#include "TMC260Words.h"

using namespace mtca4u;

RegisterValuesCalculator::RegisterValues
  RegisterValuesCalculator::calculateRegisterValues( 
    ParametersCalculator::TMC429Parameters tmc429Parameters,
    EndSwitchConfig endSwitchConfig){

  DriverControlData driverControlData;
  driverControlData.setMicroStepResolution( 
    tmc429Parameters.driverMicroStepValue);

  StallGuardControlData stallGuardControlData;
  stallGuardControlData.setFilterEnable(1);
  stallGuardControlData.setCurrentScale( tmc429Parameters.currentScale );

  ReferenceConfigAndRampModeData referenceConfigAndRampModeData;
  // This is cheating a bit. We know that the endSwitchConfig
  // already contains the right bit sequence for RampMode 0, which 
  // we want.
  referenceConfigAndRampModeData.setDATA( endSwitchConfig );

  ProportionalityFactorData proportionalityFactorData;
  proportionalityFactorData.setDivisionParameter(tmc429Parameters.pDiv);
  proportionalityFactorData.setMultiplicationParameter(
    tmc429Parameters.pMul);

  DividersAndMicroStepResolutionData dividersAndMicroStepResolutionData;
  dividersAndMicroStepResolutionData.setMicroStepResolution(
    tmc429Parameters.controllerMicroStepValue);
  dividersAndMicroStepResolutionData.setRampDivider(
    tmc429Parameters.rampDiv);
  dividersAndMicroStepResolutionData.setPulseDivider(
    tmc429Parameters.pulseDiv);

  uint32_t maximumVelocity = tmc429Parameters.vMax;
  uint32_t maximumAcceleration = tmc429Parameters.aMax;

  return RegisterValues( driverControlData.getPayloadData(),
			 stallGuardControlData.getPayloadData(),
			 referenceConfigAndRampModeData.getDATA(),
			 proportionalityFactorData.getDATA(),
			 dividersAndMicroStepResolutionData.getDATA(),
			 maximumVelocity,
			 maximumAcceleration );
	 
}
