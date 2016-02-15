#include "populateMotorExpertPanel.h"
#include "ParametersPanel.h"

void populateMotorExpertPanel(ParametersPanel *parametersPanel){
  parametersPanel->addParameter("accelerationThresholdData");
  parametersPanel->addParameter("actualPosition");
  parametersPanel->addParameter("decoderReadoutMode");
  parametersPanel->addParameter("dividersAndMicroStepResolutionData", 0, "(*)");
  parametersPanel->addParameter("enabled");
  parametersPanel->addParameter("interruptData");
  parametersPanel->addParameter("maximumAcceleration", 0, "(*)");
  parametersPanel->addParameter("maximumVelocity", 0, "(*)");
  parametersPanel->addParameter("microStepCount");
  parametersPanel->addParameter("minimumVelocity");
  parametersPanel->addParameter("positionTolerance");
  parametersPanel->addParameter("proportionalityFactorData", 0, "(*)");
  parametersPanel->addParameter("referenceConfigAndRampModeData", 0xDEADB00B, "(*)");
  parametersPanel->addParameter("targetPosition");
  parametersPanel->addParameter("targetVelocity");
  parametersPanel->addParameter("driverSpiWaitingTime");

  parametersPanel->addParameter("driverConfigData");
  parametersPanel->addParameter("driverControlData", 0xDEADB00B, "(*)");
  parametersPanel->addParameter("chopperControlData");
  parametersPanel->addParameter("coolStepControlData");
  parametersPanel->addParameter("stallGuardControlData", 0, "(*)");
}
