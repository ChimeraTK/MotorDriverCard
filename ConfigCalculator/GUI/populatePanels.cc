#include "populatePanels.h"

#include "MotorControlerConfig.h"
#include "MotorDriverCardConfig.h"
#include "ParametersPanel.h"

#define ADD_TMC429_PARAMETER(parameter, extraText)                                                                     \
  parametersPanel->addParameter(#parameter, defaultConfig.parameter.getDATA(), extraText)
#define ADD_TMC260_PARAMETER(parameter, extraText)                                                                     \
  parametersPanel->addParameter(#parameter, defaultConfig.parameter.getPayloadData(), extraText)
#define ADD_INT_PARAMETER(parameter, extraText)                                                                        \
  parametersPanel->addParameter(#parameter, defaultConfig.parameter, extraText)

void populateMotorExpertPanel(ParametersPanel* parametersPanel) {
  mtca4u::MotorControlerConfig defaultConfig;
  ADD_TMC429_PARAMETER(accelerationThresholdData, "");
  // ADD_INT_PARAMETER(actualPosition,"");
  ADD_INT_PARAMETER(decoderReadoutMode, "");
  ADD_TMC429_PARAMETER(dividersAndMicroStepResolutionData, "(*)");
  ADD_INT_PARAMETER(enabled, "");
  ADD_TMC429_PARAMETER(interruptData, "");
  ADD_INT_PARAMETER(maximumAcceleration, "(*)");
  ADD_INT_PARAMETER(maximumVelocity, "(*)");
  ADD_INT_PARAMETER(microStepCount, "");
  ADD_INT_PARAMETER(minimumVelocity, "");
  ADD_INT_PARAMETER(positionTolerance, "");
  ADD_TMC429_PARAMETER(proportionalityFactorData, "(*)");
  ADD_TMC429_PARAMETER(referenceConfigAndRampModeData, "(*)");
  ADD_INT_PARAMETER(targetPosition, "");
  ADD_INT_PARAMETER(targetVelocity, "");
  ADD_INT_PARAMETER(driverSpiWaitingTime, "");

  ADD_TMC260_PARAMETER(driverConfigData, "");
  ADD_TMC260_PARAMETER(driverControlData, "(*)");
  ADD_TMC260_PARAMETER(chopperControlData, "");
  ADD_TMC260_PARAMETER(coolStepControlData, "");
  ADD_TMC260_PARAMETER(stallGuardControlData, "(*)");
}

void populateDriverCardExpertPanel(ParametersPanel* parametersPanel) {
  mtca4u::MotorDriverCardConfig defaultConfig;
  ADD_INT_PARAMETER(controlerSpiWaitingTime, "");
  ADD_INT_PARAMETER(coverDatagram, "");
  ADD_TMC429_PARAMETER(coverPositionAndLength, "");
  ADD_INT_PARAMETER(datagramHighWord, "");
  ADD_INT_PARAMETER(datagramLowWord, "");
  ADD_TMC429_PARAMETER(interfaceConfiguration, "(*)");
  ADD_TMC429_PARAMETER(positionCompareInterruptData, "");
  ADD_INT_PARAMETER(positionCompareWord, "");
  ADD_TMC429_PARAMETER(stepperMotorGlobalParameters, "");
}
