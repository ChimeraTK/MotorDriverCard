#include "MotorDriverCardConfigXML.h"

#include <ChimeraTK/Exception.h>
using namespace mtca4u;

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

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
  plusOneControlerConfig.VARIABLE.setDataWord(defaultControlerConfig.VARIABLE.getDataWord() + 1)

#define ASSIGN_PLUS_ONE_CARD(VARIABLE, CARD_CONFIG)                                                                    \
  CARD_CONFIG.VARIABLE.setDataWord(defaultCardConfig.VARIABLE.getDataWord() + 1)

#define CONTROLER_CHECK(VARIABLE)                                                                                      \
  BOOST_CHECK_MESSAGE(inputControlerConfig.VARIABLE == plusOneControlerConfig.VARIABLE, message.str() + #VARIABLE)

class MotorDriverCardConfigXMLTest {
 public:
  MotorDriverCardConfigXMLTest();

  static void testInvalidInput();

  void testReadMinimal();
  void testReadComplete();
  void testReadFirstHalf();
  void testReadSecondHalf();

  void testWrite();
  void testInvalidOutput();
  void testOldInvalidRegister(std::string const& registerName);

 private:
  MotorDriverCardConfig defaultCardConfig;
  MotorDriverCardConfig completeCardConfig;   ///< all values differ from the default
  MotorDriverCardConfig firstHalfCardConfig;  ///< first half differs from default
  MotorDriverCardConfig secondHalfCardConfig; ///< second half differs from default

  MotorControlerConfig defaultControlerConfig;
  MotorControlerConfig plusOneControlerConfig; // all values are default + 1, true <-> false,
                                               // signed values are -(default+1)

  void checkControlerConfig(MotorControlerConfig const& inputControlerConfig, unsigned int motorID);
};

class MotorDriverCardConfigXMLTestSuite : public test_suite {
 public:
  MotorDriverCardConfigXMLTestSuite() : test_suite(" MotorDriverCardConfigXML test suite") {
    // create an instance of the test class
    boost::shared_ptr<MotorDriverCardConfigXMLTest> xmlTest(new MotorDriverCardConfigXMLTest);

    add(BOOST_TEST_CASE(&MotorDriverCardConfigXMLTest::testInvalidInput));
    add(BOOST_CLASS_TEST_CASE(&MotorDriverCardConfigXMLTest::testReadMinimal, xmlTest));
    add(BOOST_CLASS_TEST_CASE(&MotorDriverCardConfigXMLTest::testReadComplete, xmlTest));
    add(BOOST_CLASS_TEST_CASE(&MotorDriverCardConfigXMLTest::testReadFirstHalf, xmlTest));
    add(BOOST_CLASS_TEST_CASE(&MotorDriverCardConfigXMLTest::testReadSecondHalf, xmlTest));
    add(BOOST_CLASS_TEST_CASE(&MotorDriverCardConfigXMLTest::testWrite, xmlTest));
    add(BOOST_CLASS_TEST_CASE(&MotorDriverCardConfigXMLTest::testInvalidOutput, xmlTest));
    add(BOOST_TEST_CASE(boost::bind(&MotorDriverCardConfigXMLTest::testOldInvalidRegister, xmlTest,
        "maximumAccelleration"))); // (typo with two 'l's)
  }
};

test_suite* init_unit_test_suite(int argc, char* argv[]) {
  if(argc != 1) {
    std::cerr << argv[0] << " does not accept any arguments. They are ignored." << std::endl;
  }

  framework::master_test_suite().p_name.value = "MotorDriverCardConfigXML test suite";
  return new MotorDriverCardConfigXMLTestSuite;
}

MotorDriverCardConfigXMLTest::MotorDriverCardConfigXMLTest()
: defaultCardConfig(), completeCardConfig(), firstHalfCardConfig(), secondHalfCardConfig(), defaultControlerConfig(),
  plusOneControlerConfig() {
  ASSIGN_PLUS_ONE_CONTROLER(accelerationThresholdData);
  // plusOneControlerConfig.actualPosition =
  // -(plusOneControlerConfig.actualPosition+1);
  ASSIGN_PLUS_ONE_CONTROLER(chopperControlData);
  ASSIGN_PLUS_ONE_CONTROLER(coolStepControlData);
  plusOneControlerConfig.decoderReadoutMode++;
  ASSIGN_PLUS_ONE_CONTROLER(dividersAndMicroStepResolutionData);
  ASSIGN_PLUS_ONE_CONTROLER(driverConfigData);
  ASSIGN_PLUS_ONE_CONTROLER(driverControlData);
  plusOneControlerConfig.enabled = !(plusOneControlerConfig.enabled);
  ASSIGN_PLUS_ONE_CONTROLER(interruptData);
  plusOneControlerConfig.maximumAcceleration++;
  plusOneControlerConfig.maximumVelocity++;
  plusOneControlerConfig.microStepCount++;
  plusOneControlerConfig.minimumVelocity++;
  plusOneControlerConfig.positionTolerance++;
  ASSIGN_PLUS_ONE_CONTROLER(proportionalityFactorData);
  ASSIGN_PLUS_ONE_CONTROLER(referenceConfigAndRampModeData);
  ASSIGN_PLUS_ONE_CONTROLER(stallGuardControlData);
  plusOneControlerConfig.targetPosition = -(plusOneControlerConfig.targetPosition + 1);
  plusOneControlerConfig.targetVelocity = -(plusOneControlerConfig.targetVelocity + 1);
  plusOneControlerConfig.driverSpiWaitingTime++;

  completeCardConfig.coverDatagram++;
  firstHalfCardConfig.coverDatagram++;

  ASSIGN_PLUS_ONE_CARD(coverPositionAndLength, completeCardConfig);
  ASSIGN_PLUS_ONE_CARD(coverPositionAndLength, secondHalfCardConfig);

  completeCardConfig.datagramHighWord++;
  firstHalfCardConfig.datagramHighWord++;

  completeCardConfig.datagramLowWord++;
  secondHalfCardConfig.datagramLowWord++;

  ASSIGN_PLUS_ONE_CARD(interfaceConfiguration, completeCardConfig);
  ASSIGN_PLUS_ONE_CARD(interfaceConfiguration, firstHalfCardConfig);

  ASSIGN_PLUS_ONE_CARD(positionCompareInterruptData, completeCardConfig);
  ASSIGN_PLUS_ONE_CARD(positionCompareInterruptData, secondHalfCardConfig);

  completeCardConfig.positionCompareWord++;
  firstHalfCardConfig.positionCompareWord++;

  ASSIGN_PLUS_ONE_CARD(stepperMotorGlobalParameters, completeCardConfig);
  ASSIGN_PLUS_ONE_CARD(stepperMotorGlobalParameters, secondHalfCardConfig);

  completeCardConfig.controlerSpiWaitingTime++;
  firstHalfCardConfig.controlerSpiWaitingTime++;

  firstHalfCardConfig.motorControlerConfigurations[0] = plusOneControlerConfig;
  secondHalfCardConfig.motorControlerConfigurations[1] = plusOneControlerConfig;
  completeCardConfig.motorControlerConfigurations[0] = plusOneControlerConfig;
  completeCardConfig.motorControlerConfigurations[1] = plusOneControlerConfig;
}

void MotorDriverCardConfigXMLTest::testInvalidInput() {
  BOOST_CHECK_THROW(MotorDriverCardConfigXML::read("invalid.xml"), ChimeraTK::logic_error);
}

void MotorDriverCardConfigXMLTest::testReadMinimal() {
  MotorDriverCardConfig inputCardConfig = MotorDriverCardConfigXML::read(MINIMAL_XML_FILE_NAME);
  BOOST_CHECK(inputCardConfig == defaultCardConfig);
}

void MotorDriverCardConfigXMLTest::testReadComplete() {
  MotorDriverCardConfig inputCardConfig = MotorDriverCardConfigXML::read(COMPLETE_XML_FILE_NAME);
  BOOST_CHECK(inputCardConfig.coverDatagram == completeCardConfig.coverDatagram);
  BOOST_CHECK(inputCardConfig.coverPositionAndLength == completeCardConfig.coverPositionAndLength);
  BOOST_CHECK(inputCardConfig.datagramHighWord == completeCardConfig.datagramHighWord);
  BOOST_CHECK(inputCardConfig.datagramLowWord == completeCardConfig.datagramLowWord);
  BOOST_CHECK(inputCardConfig.interfaceConfiguration == completeCardConfig.interfaceConfiguration);
  BOOST_CHECK(inputCardConfig.positionCompareInterruptData == completeCardConfig.positionCompareInterruptData);
  BOOST_CHECK(inputCardConfig.positionCompareWord == completeCardConfig.positionCompareWord);
  BOOST_CHECK(inputCardConfig.stepperMotorGlobalParameters == completeCardConfig.stepperMotorGlobalParameters);
  BOOST_CHECK(inputCardConfig.controlerSpiWaitingTime == completeCardConfig.controlerSpiWaitingTime);

  for(size_t i = 0; i < inputCardConfig.motorControlerConfigurations.size(); ++i) {
    checkControlerConfig(inputCardConfig.motorControlerConfigurations[i], i);
  }
}

void MotorDriverCardConfigXMLTest::checkControlerConfig(
    MotorControlerConfig const& inputControlerConfig, unsigned int motorID) {
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

void MotorDriverCardConfigXMLTest::testReadFirstHalf() {
  MotorDriverCardConfig inputCardConfig = MotorDriverCardConfigXML::read(FIRST_HALF_XML_FILE_NAME);
  BOOST_CHECK(inputCardConfig.coverDatagram == completeCardConfig.coverDatagram);
  BOOST_CHECK(inputCardConfig.coverPositionAndLength == defaultCardConfig.coverPositionAndLength);
  BOOST_CHECK(inputCardConfig.datagramHighWord == completeCardConfig.datagramHighWord);
  BOOST_CHECK(inputCardConfig.datagramLowWord == defaultCardConfig.datagramLowWord);
  BOOST_CHECK(inputCardConfig.interfaceConfiguration == completeCardConfig.interfaceConfiguration);
  BOOST_CHECK(inputCardConfig.positionCompareInterruptData == defaultCardConfig.positionCompareInterruptData);
  BOOST_CHECK(inputCardConfig.positionCompareWord == completeCardConfig.positionCompareWord);
  BOOST_CHECK(inputCardConfig.stepperMotorGlobalParameters == defaultCardConfig.stepperMotorGlobalParameters);
  BOOST_CHECK(inputCardConfig.controlerSpiWaitingTime == completeCardConfig.controlerSpiWaitingTime);

  checkControlerConfig(inputCardConfig.motorControlerConfigurations[0], 0);
  BOOST_CHECK(inputCardConfig.motorControlerConfigurations[1] == defaultControlerConfig);
}

void MotorDriverCardConfigXMLTest::testReadSecondHalf() {
  MotorDriverCardConfig inputCardConfig = MotorDriverCardConfigXML::read(SECOND_HALF_XML_FILE_NAME);
  BOOST_CHECK(inputCardConfig.coverDatagram == defaultCardConfig.coverDatagram);
  BOOST_CHECK(inputCardConfig.coverPositionAndLength == completeCardConfig.coverPositionAndLength);
  BOOST_CHECK(inputCardConfig.datagramHighWord == defaultCardConfig.datagramHighWord);
  BOOST_CHECK(inputCardConfig.datagramLowWord == completeCardConfig.datagramLowWord);
  BOOST_CHECK(inputCardConfig.interfaceConfiguration == defaultCardConfig.interfaceConfiguration);
  BOOST_CHECK(inputCardConfig.positionCompareInterruptData == completeCardConfig.positionCompareInterruptData);
  BOOST_CHECK(inputCardConfig.positionCompareWord == defaultCardConfig.positionCompareWord);
  BOOST_CHECK(inputCardConfig.stepperMotorGlobalParameters == completeCardConfig.stepperMotorGlobalParameters);
  BOOST_CHECK(inputCardConfig.controlerSpiWaitingTime == defaultCardConfig.controlerSpiWaitingTime);

  BOOST_CHECK(inputCardConfig.motorControlerConfigurations[0] == defaultControlerConfig);
  checkControlerConfig(inputCardConfig.motorControlerConfigurations[1], 1);
}

void MotorDriverCardConfigXMLTest::testWrite() {
  MotorDriverCardConfigXML::write(WRITE_FULL_COMPLETE_OUTPUT_FILE_NAME, completeCardConfig);
  MotorDriverCardConfigXML::writeSparse(WRITE_SPARSE_COMPLETE_OUTPUT_FILE_NAME, completeCardConfig);
  MotorDriverCardConfigXML::writeSparse(WRITE_SPARSE_FIRST_HALF_OUTPUT_FILE_NAME, firstHalfCardConfig);
  MotorDriverCardConfigXML::write(WRITE_FULL_SECOND_HALF_OUTPUT_FILE_NAME, secondHalfCardConfig);
  MotorDriverCardConfigXML::writeSparse(WRITE_MINIMAL_OUTPUT_FILE_NAME, defaultCardConfig);

  // the written output is checked in a separate test at shell level
}

void MotorDriverCardConfigXMLTest::testInvalidOutput() {
  BOOST_CHECK_THROW(
      MotorDriverCardConfigXML::write("/some/not/existing/file.xml", completeCardConfig), ChimeraTK::logic_error);
}

void MotorDriverCardConfigXMLTest::testOldInvalidRegister(std::string const& registerName) {
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
