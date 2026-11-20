#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE MotorDriverCardConfigXmlTest

#include "MotorDriverCardConfigXML.h"

#include <ChimeraTK/Exception.h>
using namespace ChimeraTK;

#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

using namespace ChimeraTK::MotorDriver;

static const std::string COMPLETE_XML_FILE_NAME("MotorDriverCardConfig_complete_test.xml");
static const std::string FIRST_HALF_XML_FILE_NAME("MotorDriverCardConfig_first_half_test.xml");
static const std::string SECOND_HALF_XML_FILE_NAME("MotorDriverCardConfig_second_half_test.xml");
static const std::string MINIMAL_XML_FILE_NAME("MotorDriverCardConfig_minimal_test.xml");
static const std::string OLD_INVALID_XML_PREFIX("MotorDriverCardConfig_invalid_");
static const std::string OLD_INVALID_XML_SUFFIX(".xml");

static const std::string WRITE_FULL_COMPLETE_OUTPUT_FILE_NAME("MotorDriverCardConfig_complete_full_output.xml");
static const std::string WRITE_SPARSE_COMPLETE_OUTPUT_FILE_NAME("MotorDriverCardConfig_complete_sparse_output.xml");
static const std::string WRITE_SPARSE_FIRST_HALF_OUTPUT_FILE_NAME("MotorDriverCardConfig_first_half_sparse_output.xml");
static const std::string WRITE_FULL_SECOND_HALF_OUTPUT_FILE_NAME("MotorDriverCardConfig_second_half_full_output.xml");
static const std::string WRITE_MINIMAL_OUTPUT_FILE_NAME("MotorDriverCardConfig_minimal_output.xml");

#define ASSIGN_PLUS_ONE_CONTROLLER(VARIABLE)                                                                           \
  _plusOneControllerConfig.VARIABLE.setDataWord(_defaultControllerConfig.VARIABLE.getDataWord() + 1)

#define ASSIGN_PLUS_ONE_CARD(VARIABLE, CARD_CONFIG)                                                                    \
  CARD_CONFIG.VARIABLE.setDataWord(_defaultCardConfig.VARIABLE.getDataWord() + 1)

#define CONTROLLER_CHECK(VARIABLE)                                                                                     \
  BOOST_CHECK_MESSAGE(inputControllerConfig.VARIABLE == _plusOneControllerConfig.VARIABLE, message.str() + #VARIABLE)

class MotorDriverCardConfigXMLTest {
 public:
  MotorDriverCardConfigXMLTest();

 protected:
  MotorDriverCardConfig _defaultCardConfig;
  MotorDriverCardConfig _completeCardConfig;   ///< all values differ from the default
  MotorDriverCardConfig _firstHalfCardConfig;  ///< first half differs from default
  MotorDriverCardConfig _secondHalfCardConfig; ///< second half differs from default

  MotorControllerConfig _defaultControllerConfig;
  MotorControllerConfig _plusOneControllerConfig; // all values are default + 1, true <-> false,
                                                  // signed values are -(default+1)

  void checkControllerConfig(MotorControllerConfig const& inputControllerConfig, unsigned int motorID) const;
};

MotorDriverCardConfigXMLTest::MotorDriverCardConfigXMLTest() {
  ASSIGN_PLUS_ONE_CONTROLLER(accelerationThresholdData);
  // plusOneControllerConfig.actualPosition =
  // -(plusOneControllerConfig.actualPosition+1);
  ASSIGN_PLUS_ONE_CONTROLLER(chopperControlData);
  ASSIGN_PLUS_ONE_CONTROLLER(coolStepControlData);
  _plusOneControllerConfig.decoderReadoutMode++;
  ASSIGN_PLUS_ONE_CONTROLLER(dividersAndMicroStepResolutionData);
  ASSIGN_PLUS_ONE_CONTROLLER(driverConfigData);
  ASSIGN_PLUS_ONE_CONTROLLER(driverControlData);
  _plusOneControllerConfig.enabled = !(_plusOneControllerConfig.enabled);
  ASSIGN_PLUS_ONE_CONTROLLER(interruptData);
  _plusOneControllerConfig.maximumAcceleration++;
  _plusOneControllerConfig.maximumVelocity++;
  _plusOneControllerConfig.microStepCount++;
  _plusOneControllerConfig.minimumVelocity++;
  _plusOneControllerConfig.positionTolerance++;
  ASSIGN_PLUS_ONE_CONTROLLER(proportionalityFactorData);
  ASSIGN_PLUS_ONE_CONTROLLER(referenceConfigAndRampModeData);
  ASSIGN_PLUS_ONE_CONTROLLER(stallGuardControlData);
  _plusOneControllerConfig.targetPosition = -(_plusOneControllerConfig.targetPosition + 1);
  _plusOneControllerConfig.targetVelocity = -(_plusOneControllerConfig.targetVelocity + 1);
  _plusOneControllerConfig.driverSpiWaitingTime++;

  _completeCardConfig.coverDatagram++;
  _firstHalfCardConfig.coverDatagram++;

  ASSIGN_PLUS_ONE_CARD(coverPositionAndLength, _completeCardConfig);
  ASSIGN_PLUS_ONE_CARD(coverPositionAndLength, _secondHalfCardConfig);

  _completeCardConfig.datagramHighWord++;
  _firstHalfCardConfig.datagramHighWord++;

  _completeCardConfig.datagramLowWord++;
  _secondHalfCardConfig.datagramLowWord++;

  ASSIGN_PLUS_ONE_CARD(interfaceConfiguration, _completeCardConfig);
  ASSIGN_PLUS_ONE_CARD(interfaceConfiguration, _firstHalfCardConfig);

  ASSIGN_PLUS_ONE_CARD(positionCompareInterruptData, _completeCardConfig);
  ASSIGN_PLUS_ONE_CARD(positionCompareInterruptData, _secondHalfCardConfig);

  _completeCardConfig.positionCompareWord++;
  _firstHalfCardConfig.positionCompareWord++;

  ASSIGN_PLUS_ONE_CARD(stepperMotorGlobalParameters, _completeCardConfig);
  ASSIGN_PLUS_ONE_CARD(stepperMotorGlobalParameters, _secondHalfCardConfig);

  _completeCardConfig.controllerSpiWaitingTime++;
  _firstHalfCardConfig.controllerSpiWaitingTime++;

  _firstHalfCardConfig.motorControllerConfigurations[0] = _plusOneControllerConfig;
  _secondHalfCardConfig.motorControllerConfigurations[1] = _plusOneControllerConfig;
  _completeCardConfig.motorControllerConfigurations[0] = _plusOneControllerConfig;
  _completeCardConfig.motorControllerConfigurations[1] = _plusOneControllerConfig;
}

BOOST_AUTO_TEST_CASE(TestInvalidInput) {
  BOOST_CHECK_THROW(MotorDriverCardConfigXML::read("invalid.xml"), ChimeraTK::logic_error);
}

BOOST_FIXTURE_TEST_CASE(TestReadMinimal, MotorDriverCardConfigXMLTest) {
  MotorDriverCardConfig inputCardConfig = MotorDriverCardConfigXML::read(MINIMAL_XML_FILE_NAME);
  BOOST_CHECK(inputCardConfig == _defaultCardConfig);
}

BOOST_FIXTURE_TEST_CASE(TestReadComplete, MotorDriverCardConfigXMLTest) {
  MotorDriverCardConfig inputCardConfig = MotorDriverCardConfigXML::read(COMPLETE_XML_FILE_NAME);
  BOOST_CHECK(inputCardConfig.coverDatagram == _completeCardConfig.coverDatagram);
  BOOST_CHECK(inputCardConfig.coverPositionAndLength == _completeCardConfig.coverPositionAndLength);
  BOOST_CHECK(inputCardConfig.datagramHighWord == _completeCardConfig.datagramHighWord);
  BOOST_CHECK(inputCardConfig.datagramLowWord == _completeCardConfig.datagramLowWord);
  BOOST_CHECK(inputCardConfig.interfaceConfiguration == _completeCardConfig.interfaceConfiguration);
  BOOST_CHECK(inputCardConfig.positionCompareInterruptData == _completeCardConfig.positionCompareInterruptData);
  BOOST_CHECK(inputCardConfig.positionCompareWord == _completeCardConfig.positionCompareWord);
  BOOST_CHECK(inputCardConfig.stepperMotorGlobalParameters == _completeCardConfig.stepperMotorGlobalParameters);
  BOOST_CHECK(inputCardConfig.controllerSpiWaitingTime == _completeCardConfig.controllerSpiWaitingTime);

  for(size_t i = 0; i < inputCardConfig.motorControllerConfigurations.size(); ++i) {
    checkControllerConfig(inputCardConfig.motorControllerConfigurations[i], i);
  }
}

void MotorDriverCardConfigXMLTest::checkControllerConfig(
    MotorControllerConfig const& inputControllerConfig, unsigned int motorID) const {
  std::stringstream message;
  message << "check in motorControllerConfig " << motorID << " failed for variable ";
  CONTROLLER_CHECK(accelerationThresholdData);
  // CONTROLLER_CHECK( actualPosition );
  CONTROLLER_CHECK(chopperControlData);
  CONTROLLER_CHECK(coolStepControlData);
  CONTROLLER_CHECK(decoderReadoutMode);
  CONTROLLER_CHECK(dividersAndMicroStepResolutionData);
  CONTROLLER_CHECK(driverConfigData);
  CONTROLLER_CHECK(driverControlData);
  CONTROLLER_CHECK(enabled);
  CONTROLLER_CHECK(interruptData);
  CONTROLLER_CHECK(maximumAcceleration);
  CONTROLLER_CHECK(maximumVelocity);
  CONTROLLER_CHECK(microStepCount);
  CONTROLLER_CHECK(minimumVelocity);
  CONTROLLER_CHECK(positionTolerance);
  CONTROLLER_CHECK(proportionalityFactorData);
  CONTROLLER_CHECK(referenceConfigAndRampModeData);
  CONTROLLER_CHECK(stallGuardControlData);
  CONTROLLER_CHECK(targetPosition);
  CONTROLLER_CHECK(targetVelocity);
  CONTROLLER_CHECK(driverSpiWaitingTime);
}

BOOST_FIXTURE_TEST_CASE(TestReadFirstHalf, MotorDriverCardConfigXMLTest) {
  MotorDriverCardConfig inputCardConfig = MotorDriverCardConfigXML::read(FIRST_HALF_XML_FILE_NAME);
  BOOST_CHECK(inputCardConfig.coverDatagram == _completeCardConfig.coverDatagram);
  BOOST_CHECK(inputCardConfig.coverPositionAndLength == _defaultCardConfig.coverPositionAndLength);
  BOOST_CHECK(inputCardConfig.datagramHighWord == _completeCardConfig.datagramHighWord);
  BOOST_CHECK(inputCardConfig.datagramLowWord == _defaultCardConfig.datagramLowWord);
  BOOST_CHECK(inputCardConfig.interfaceConfiguration == _completeCardConfig.interfaceConfiguration);
  BOOST_CHECK(inputCardConfig.positionCompareInterruptData == _defaultCardConfig.positionCompareInterruptData);
  BOOST_CHECK(inputCardConfig.positionCompareWord == _completeCardConfig.positionCompareWord);
  BOOST_CHECK(inputCardConfig.stepperMotorGlobalParameters == _defaultCardConfig.stepperMotorGlobalParameters);
  BOOST_CHECK(inputCardConfig.controllerSpiWaitingTime == _completeCardConfig.controllerSpiWaitingTime);

  checkControllerConfig(inputCardConfig.motorControllerConfigurations[0], 0);
  BOOST_CHECK(inputCardConfig.motorControllerConfigurations[1] == _defaultControllerConfig);
}

BOOST_FIXTURE_TEST_CASE(TestReadSecondHalf, MotorDriverCardConfigXMLTest) {
  MotorDriverCardConfig inputCardConfig = MotorDriverCardConfigXML::read(SECOND_HALF_XML_FILE_NAME);
  BOOST_CHECK(inputCardConfig.coverDatagram == _defaultCardConfig.coverDatagram);
  BOOST_CHECK(inputCardConfig.coverPositionAndLength == _completeCardConfig.coverPositionAndLength);
  BOOST_CHECK(inputCardConfig.datagramHighWord == _defaultCardConfig.datagramHighWord);
  BOOST_CHECK(inputCardConfig.datagramLowWord == _completeCardConfig.datagramLowWord);
  BOOST_CHECK(inputCardConfig.interfaceConfiguration == _defaultCardConfig.interfaceConfiguration);
  BOOST_CHECK(inputCardConfig.positionCompareInterruptData == _completeCardConfig.positionCompareInterruptData);
  BOOST_CHECK(inputCardConfig.positionCompareWord == _defaultCardConfig.positionCompareWord);
  BOOST_CHECK(inputCardConfig.stepperMotorGlobalParameters == _completeCardConfig.stepperMotorGlobalParameters);
  BOOST_CHECK(inputCardConfig.controllerSpiWaitingTime == _defaultCardConfig.controllerSpiWaitingTime);

  BOOST_CHECK(inputCardConfig.motorControllerConfigurations[0] == _defaultControllerConfig);
  checkControllerConfig(inputCardConfig.motorControllerConfigurations[1], 1);
}

BOOST_FIXTURE_TEST_CASE(TestWrite, MotorDriverCardConfigXMLTest) {
  MotorDriverCardConfigXML::write(WRITE_FULL_COMPLETE_OUTPUT_FILE_NAME, _completeCardConfig);
  MotorDriverCardConfigXML::writeSparse(WRITE_SPARSE_COMPLETE_OUTPUT_FILE_NAME, _completeCardConfig);
  MotorDriverCardConfigXML::writeSparse(WRITE_SPARSE_FIRST_HALF_OUTPUT_FILE_NAME, _firstHalfCardConfig);
  MotorDriverCardConfigXML::write(WRITE_FULL_SECOND_HALF_OUTPUT_FILE_NAME, _secondHalfCardConfig);
  MotorDriverCardConfigXML::writeSparse(WRITE_MINIMAL_OUTPUT_FILE_NAME, _defaultCardConfig);

  // the written output is checked in a separate test at shell level
}

BOOST_FIXTURE_TEST_CASE(TestInvalidOutput, MotorDriverCardConfigXMLTest) {
  BOOST_CHECK_THROW(
      MotorDriverCardConfigXML::write("/some/not/existing/file.xml", _completeCardConfig), ChimeraTK::logic_error);
}

BOOST_FIXTURE_TEST_CASE(TestOldInvalidRegister, MotorDriverCardConfigXMLTest) {
  const auto* registerName = "maximumAccelleration"; // (typo with two 'l's)
  // check that an exception is thrown if any of the register name appears
  // (note: only do one invalid name per file so it is excatly this name which
  // is triggering
  //  the exception)

  std::string fileName = OLD_INVALID_XML_PREFIX + registerName + OLD_INVALID_XML_SUFFIX;
  BOOST_CHECK_THROW(MotorDriverCardConfigXML::read(fileName), ChimeraTK::logic_error);

  try {
    MotorDriverCardConfigXML::read(fileName);
  }
  catch(ChimeraTK::logic_error& e) {
    std::string expectedMessage{"Failed to validate XML file, please check your config"};
    BOOST_TEST(expectedMessage == e.what());
  }
}
