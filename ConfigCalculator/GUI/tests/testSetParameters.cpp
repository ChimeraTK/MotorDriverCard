#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SetParametersTest

#include "getParameters.h"
#include "MotorDriverCardConfigXML.h"
#include "ParametersPanel.h"
#include "populatePanels.h"

#include <boost/test/unit_test.hpp>

#include <array>
#include <cstdio>
#include <fstream>
#include <QApplication>
#include <sstream>
#include <string>

namespace {
  /** Qt needs an application object before widgets can be created. */
  struct QApplicationFixture {
    QApplicationFixture() : application(argc, argv.data()) {}

    int argc{1};
    std::array<char, 9> applicationName{"test"};
    std::array<char*, 2> argv{applicationName.data(), nullptr};
    QApplication application;
  };
} // namespace

static std::string readFile(std::string const& fileName) {
  std::ifstream file(fileName);
  std::ostringstream contents;
  contents << file.rdbuf();
  return contents.str();
}

BOOST_GLOBAL_FIXTURE(QApplicationFixture);

BOOST_AUTO_TEST_SUITE(SetParametersTest)

BOOST_AUTO_TEST_CASE(RoundTripThroughPanels) {
  std::string const inputFileName("SetParameters_input.xml");
  std::string const expectedFileName("SetParameters_expected.xml");
  std::string const actualFileName("SetParameters_actual.xml");

  mtca4u::MotorDriverCardConfig config;
  config.coverDatagram = 0x123;
  config.coverPositionAndLength.setDATA(0x456);
  config.datagramHighWord = 0x789;
  config.datagramLowWord = 0xABC;
  config.interfaceConfiguration.setDATA(0x1);
  config.positionCompareInterruptData.setDATA(0x234);
  config.positionCompareWord = 0x567;
  config.stepperMotorGlobalParameters.setDATA(0x89A);
  config.controlerSpiWaitingTime = 0xBCD;

  for(unsigned int motorID = 0; motorID < config.motorControlerConfigurations.size(); ++motorID) {
    mtca4u::MotorControlerConfig& motorConfig = config.motorControlerConfigurations[motorID];
    motorConfig.accelerationThresholdData.setDATA(0x111 + motorID);
    motorConfig.chopperControlData.setPayloadData(0x222 + motorID);
    motorConfig.coolStepControlData.setPayloadData(0x333 + motorID);
    motorConfig.decoderReadoutMode = 0x444 + motorID;
    motorConfig.dividersAndMicroStepResolutionData.setDATA(0x555 + motorID);
    motorConfig.driverConfigData.setPayloadData(0x666 + motorID);
    motorConfig.driverControlData.setPayloadData(0x777 + motorID);
    motorConfig.enabled = true;
    motorConfig.interruptData.setDATA(0x888 + motorID);
    motorConfig.maximumAcceleration = 0x999 + motorID;
    motorConfig.maximumVelocity = 0xAAA + motorID;
    motorConfig.microStepCount = 0xBBB + motorID;
    motorConfig.minimumVelocity = 0xCCC + motorID;
    motorConfig.positionTolerance = 0xDDD + motorID;
    motorConfig.proportionalityFactorData.setDATA(0xEEE + motorID);
    motorConfig.referenceConfigAndRampModeData.setDATA(0x100 + motorID);
    motorConfig.stallGuardControlData.setPayloadData(0x200 + motorID);
    motorConfig.targetPosition = -0x300 - static_cast<int>(motorID);
    motorConfig.targetVelocity = 0x400 + static_cast<int>(motorID);
    motorConfig.driverSpiWaitingTime = 0x500 + motorID;
  }

  mtca4u::MotorDriverCardConfigXML::write(inputFileName, config);

  // simulate loading: read the file, put the values into the panels and read them back
  mtca4u::MotorDriverCardConfig const loadedConfig = mtca4u::MotorDriverCardConfigXML::read(inputFileName);

  ParametersPanel cardPanel(nullptr);
  populateDriverCardExpertPanel(&cardPanel);
  setMotorDriverCardParameters(&cardPanel, loadedConfig);
  mtca4u::MotorDriverCardConfig roundTripConfig = getMotorDriverCardParameters(&cardPanel);

  for(unsigned int motorID = 0; motorID < loadedConfig.motorControlerConfigurations.size(); ++motorID) {
    ParametersPanel motorPanel(nullptr);
    populateMotorExpertPanel(&motorPanel);
    setMotorParameters(&motorPanel, loadedConfig.motorControlerConfigurations[motorID]);
    roundTripConfig.motorControlerConfigurations[motorID] = getMotorParameters(&motorPanel);
  }

  mtca4u::MotorDriverCardConfigXML::write(expectedFileName, loadedConfig);
  mtca4u::MotorDriverCardConfigXML::write(actualFileName, roundTripConfig);

  BOOST_TEST(readFile(actualFileName) == readFile(expectedFileName));

  std::remove(inputFileName.c_str());
  std::remove(expectedFileName.c_str());
  std::remove(actualFileName.c_str());
}

BOOST_AUTO_TEST_SUITE_END()
