#include "getParameters.h"

#include "ParametersPanel.h"

// simple data types and MultiVariableWord have different syntax
// MultiVariableWord:
#define SET_FIELD(fieldName) config.fieldName.setDataWord((*parametersPanel)[#fieldName])
// simple data types can use the assignment operator
#define ASSIGN_FIELD(fieldName) config.fieldName = (*parametersPanel)[#fieldName]

// the panels hold the data word of TMC429 words and the payload of TMC260 words
#define SET_TMC429_PARAMETER(fieldName) parametersPanel->setParameter(#fieldName, config.fieldName.getDATA())
#define SET_TMC260_PARAMETER(fieldName) parametersPanel->setParameter(#fieldName, config.fieldName.getPayloadData())
#define SET_FIELD_PARAMETER(fieldName) parametersPanel->setParameter(#fieldName, config.fieldName)

mtca4u::MotorControlerConfig getMotorParameters(ParametersPanel* parametersPanel) {
  mtca4u::MotorControlerConfig config;
  SET_FIELD(accelerationThresholdData);
  // ASSIGN_FIELD(actualPosition);
  SET_FIELD(chopperControlData);
  SET_FIELD(coolStepControlData);
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

mtca4u::MotorDriverCardConfig getMotorDriverCardParameters(ParametersPanel* parametersPanel) {
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

void setMotorParameters(ParametersPanel* parametersPanel, mtca4u::MotorControlerConfig const& config) {
  SET_TMC429_PARAMETER(accelerationThresholdData);
  SET_TMC260_PARAMETER(chopperControlData);
  SET_TMC260_PARAMETER(coolStepControlData);
  SET_FIELD_PARAMETER(decoderReadoutMode);
  SET_TMC429_PARAMETER(dividersAndMicroStepResolutionData);
  SET_TMC260_PARAMETER(driverConfigData);
  SET_TMC260_PARAMETER(driverControlData);
  SET_FIELD_PARAMETER(driverSpiWaitingTime);
  parametersPanel->setParameter("enabled", static_cast<uint32_t>(config.enabled));
  SET_TMC429_PARAMETER(interruptData);
  SET_FIELD_PARAMETER(maximumAcceleration);
  SET_FIELD_PARAMETER(maximumVelocity);
  SET_FIELD_PARAMETER(microStepCount);
  SET_FIELD_PARAMETER(minimumVelocity);
  SET_FIELD_PARAMETER(positionTolerance);
  SET_TMC429_PARAMETER(proportionalityFactorData);
  SET_TMC429_PARAMETER(referenceConfigAndRampModeData);
  SET_TMC260_PARAMETER(stallGuardControlData);
  parametersPanel->setParameter("targetPosition", static_cast<uint32_t>(config.targetPosition));
  parametersPanel->setParameter("targetVelocity", static_cast<uint32_t>(config.targetVelocity));
}

void setMotorDriverCardParameters(ParametersPanel* parametersPanel, mtca4u::MotorDriverCardConfig const& config) {
  SET_FIELD_PARAMETER(controlerSpiWaitingTime);
  SET_FIELD_PARAMETER(coverDatagram);
  SET_TMC429_PARAMETER(coverPositionAndLength);
  SET_FIELD_PARAMETER(datagramHighWord);
  SET_FIELD_PARAMETER(datagramLowWord);
  SET_TMC429_PARAMETER(interfaceConfiguration);
  SET_TMC429_PARAMETER(positionCompareInterruptData);
  SET_FIELD_PARAMETER(positionCompareWord);
  SET_TMC429_PARAMETER(stepperMotorGlobalParameters);
}
