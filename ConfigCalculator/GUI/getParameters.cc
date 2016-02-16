#include "getParameters.h"
#include "ParametersPanel.h"

// simple data types and MultiVariableWord have different syntax
// MultiVariableWord:
#define SET_FIELD(fieldName) config.fieldName.setDataWord( (*parametersPanel)[#fieldName])
// simple data types can use the assignment operator
#define ASSIGN_FIELD(fieldName) config.fieldName = (*parametersPanel)[#fieldName]

mtca4u::MotorControlerConfig getMotorParameters(ParametersPanel *parametersPanel){
  mtca4u::MotorControlerConfig config;
  SET_FIELD(accelerationThresholdData);
  ASSIGN_FIELD(actualPosition);
  SET_FIELD( chopperControlData );
  SET_FIELD( coolStepControlData );
  ASSIGN_FIELD(decoderReadoutMode);
  SET_FIELD(dividersAndMicroStepResolutionData);
  SET_FIELD(driverConfigData);
  SET_FIELD(driverControlData);
  ASSIGN_FIELD(driverSpiWaitingTime);
  ASSIGN_FIELD(enabled);
  SET_FIELD(interruptData);
  ASSIGN_FIELD(maximumAcceleration);
  ASSIGN_FIELD(maximumVelocity);
  ASSIGN_FIELD(microStepCount);
  ASSIGN_FIELD(minimumVelocity);
  ASSIGN_FIELD(positionTolerance);
  SET_FIELD(proportionalityFactorData);
  SET_FIELD(referenceConfigAndRampModeData);
  SET_FIELD(stallGuardControlData);
  ASSIGN_FIELD(targetPosition);
  ASSIGN_FIELD(targetVelocity);

  return config;
}

mtca4u::MotorDriverCardConfig getMotorDriverCardParameters(ParametersPanel *parametersPanel){
  mtca4u::MotorDriverCardConfig config;
  ASSIGN_FIELD(controlerSpiWaitingTime);
  ASSIGN_FIELD(coverDatagram);
  SET_FIELD(coverPositionAndLength);
  ASSIGN_FIELD(datagramHighWord);
  ASSIGN_FIELD(datagramLowWord);
  SET_FIELD(interfaceConfiguration);
  SET_FIELD(positionCompareInterruptData);
  ASSIGN_FIELD(positionCompareWord);
  SET_FIELD(stepperMotorGlobalParameters);

  return config;
}
