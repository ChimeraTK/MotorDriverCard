#include "populateMotorExpertPanel.h"
#include "ParametersPanel.h"

void populateMotorExpertPanel(ParametersPanel *parametersPanel){
  parametersPanel->addParameter("accelerationThresholdData");
  parametersPanel->addParameter("actualPosition");
  parametersPanel->addParameter("decoderReadoutMode");
  parametersPanel->addParameter("dividersAndMicroStepResolutionData");
  parametersPanel->addParameter("enabled");
  parametersPanel->addParameter("interruptData");
  parametersPanel->addParameter("maximumAcceleration");
  parametersPanel->addParameter("maximumVelocity");
  parametersPanel->addParameter("microStepCount");
  parametersPanel->addParameter("minimumVelocity");
  parametersPanel->addParameter("positionTolerance");
  parametersPanel->addParameter("proportionalityFactorData");
  parametersPanel->addParameter("referenceConfigAndRampModeData");
  parametersPanel->addParameter("targetPosition");
  parametersPanel->addParameter("targetVelocity");
  parametersPanel->addParameter("driverSpiWaitingTime");

  parametersPanel->addParameter("driverConfigData");
  parametersPanel->addParameter("driverControlData");
  parametersPanel->addParameter("chopperControlData");
  parametersPanel->addParameter("coolStepControlData");
  parametersPanel->addParameter("stallGuardControlData");
}
