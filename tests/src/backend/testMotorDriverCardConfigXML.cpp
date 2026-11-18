#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE MotorDriverCardConfigXmlTest

#include "MotorDriverCardConfigXML.h"

#include <ChimeraTK/Exception.h>
using namespace mtca4u;

#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

#include <boost/bind/bind.hpp>

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

#define ASSIGN_PLUS_ONE_CONTROLER(VARIABLE)                                                                            \
  _plusOneControlerConfig.VARIABLE.setDataWord(_defaultControlerConfig.VARIABLE.getDataWord() + 1)

#define ASSIGN_PLUS_ONE_CARD(VARIABLE, CARD_CONFIG)                                                                    \
  CARD_CONFIG.VARIABLE.setDataWord(_defaultCardConfig.VARIABLE.getDataWord() + 1)

#define CONTROLER_CHECK(VARIABLE)                                                                                      \
  BOOST_CHECK_MESSAGE(inputControlerConfig.VARIABLE == _plusOneControlerConfig.VARIABLE, message.str() + #VARIABLE)

class MotorDriverCardConfigXMLTest {
 public:
  MotorDriverCardConfigXMLTest();

 protected:
  MotorDriverCardConfig _defaultCardConfig;
  MotorDriverCardConfig _completeCardConfig;   ///< all values differ from the default
  MotorDriverCardConfig _firstHalfCardConfig;  ///< first half differs from default
  MotorDriverCardConfig _secondHalfCardConfig; ///< second half differs from default

  MotorControlerConfig _defaultControlerConfig;
  MotorControlerConfig _plusOneControlerConfig; // all values are default + 1, true <-> false,
                                                // signed values are -(default+1)

  void checkControlerConfig(MotorControlerConfig const& inputControlerConfig, unsigned int motorID) const;
};

MotorDriverCardConfigXMLTest::MotorDriverCardConfigXMLTest() {
  ASSIGN_PLUS_ONE_CONTROLER(accelerationThresholdData);
  // plusOneControlerConfig.actualPosition =
  // -(plusOneControlerConfig.actualPosition+1);
  ASSIGN_PLUS_ONE_CONTROLER(chopperControlData);
  ASSIGN_PLUS_ONE_CONTROLER(coolStepControlData);
  _plusOneControlerConfig.decoderReadoutMode++;
  ASSIGN_PLUS_ONE_CONTROLER(dividersAndMicroStepResolutionData);
  ASSIGN_PLUS_ONE_CONTROLER(driverConfigData);
  ASSIGN_PLUS_ONE_CONTROLER(driverControlData);
  _plusOneControlerConfig.enabled = !(_plusOneControlerConfig.enabled);
  ASSIGN_PLUS_ONE_CONTROLER(interruptData);
  _plusOneControlerConfig.maximumAcceleration++;
  _plusOneControlerConfig.maximumVelocity++;
  _plusOneControlerConfig.microStepCount++;
  _plusOneControlerConfig.minimumVelocity++;
  _plusOneControlerConfig.positionTolerance++;
  ASSIGN_PLUS_ONE_CONTROLER(proportionalityFactorData);
  ASSIGN_PLUS_ONE_CONTROLER(referenceConfigAndRampModeData);
  ASSIGN_PLUS_ONE_CONTROLER(stallGuardControlData);
  _plusOneControlerConfig.targetPosition = -(_plusOneControlerConfig.targetPosition + 1);
  _plusOneControlerConfig.targetVelocity = -(_plusOneControlerConfig.targetVelocity + 1);
  _plusOneControlerConfig.driverSpiWaitingTime++;

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

  _completeCardConfig.controlerSpiWaitingTime++;
  _firstHalfCardConfig.controlerSpiWaitingTime++;

  _firstHalfCardConfig.motorControlerConfigurations[0] = _plusOneControlerConfig;
  _secondHalfCardConfig.motorControlerConfigurations[1] = _plusOneControlerConfig;
  _completeCardConfig.motorControlerConfigurations[0] = _plusOneControlerConfig;
  _completeCardConfig.motorControlerConfigurations[1] = _plusOneControlerConfig;
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
  BOOST_CHECK(inputCardConfig.controlerSpiWaitingTime == _completeCardConfig.controlerSpiWaitingTime);

  for(size_t i = 0; i < inputCardConfig.motorControlerConfigurations.size(); ++i) {
    checkControlerConfig(inputCardConfig.motorControlerConfigurations[i], i);
  }
}

void MotorDriverCardConfigXMLTest::checkControlerConfig(
    MotorControlerConfig const& inputControlerConfig, unsigned int motorID) const {
  std::stringstream message;
  message << "check in motorControlerConfig " << motorID << " failed for variable ";
  CONTROLER_CHECK(accelerationThresholdData);
  // CONTROLER_CHECK( actualPosition );
  CONTROLER_CHECK(chopperControlData);
  CONTROLER_CHECK(coolStepControlData);
  CONTROLER_CHECK(decoderReadoutMode);
  CONTROLER_CHECK(dividersAndMicroStepResolutionData);
  CONTROLER_CHECK(driverConfigData);
  CONTROLER_CHECK(driverControlData);
  CONTROLER_CHECK(enabled);
  CONTROLER_CHECK(interruptData);
  CONTROLER_CHECK(maximumAcceleration);
  CONTROLER_CHECK(maximumVelocity);
  CONTROLER_CHECK(microStepCount);
  CONTROLER_CHECK(minimumVelocity);
  CONTROLER_CHECK(positionTolerance);
  CONTROLER_CHECK(proportionalityFactorData);
  CONTROLER_CHECK(referenceConfigAndRampModeData);
  CONTROLER_CHECK(stallGuardControlData);
  CONTROLER_CHECK(targetPosition);
  CONTROLER_CHECK(targetVelocity);
  CONTROLER_CHECK(driverSpiWaitingTime);
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
  BOOST_CHECK(inputCardConfig.controlerSpiWaitingTime == _completeCardConfig.controlerSpiWaitingTime);

  checkControlerConfig(inputCardConfig.motorControlerConfigurations[0], 0);
  BOOST_CHECK(inputCardConfig.motorControlerConfigurations[1] == _defaultControlerConfig);
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
  BOOST_CHECK(inputCardConfig.controlerSpiWaitingTime == _defaultCardConfig.controlerSpiWaitingTime);

  BOOST_CHECK(inputCardConfig.motorControlerConfigurations[0] == _defaultControlerConfig);
  checkControlerConfig(inputCardConfig.motorControlerConfigurations[1], 1);
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
