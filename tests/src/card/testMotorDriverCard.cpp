#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE MotorControllerTest
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "ChimeraTK/BackendFactory.h"
#include "DFMC_MD22Dummy.h"
#include "impl/MotorDriverCardImpl.h"
#include "MotorControllerExpert.h"

#include <ChimeraTK/Device.h>
#include <ChimeraTK/MapFileParser.h>

#include <boost/shared_ptr.hpp>

using namespace ChimeraTK::MotorDriver::dfmc_md22;
#include "testWordFromPCIeAddress.h"
#include "testWordFromSpiAddress.h"
using namespace ChimeraTK::MotorDriver::tmc429;

#include "DFMC_MD22Constants.h"
#include "testConfigConstants.h"
using namespace ChimeraTK::MotorDriver;

namespace ChimeraTK::MotorDriver {

  unsigned int asciiToInt(std::string text) {
    unsigned int returnValue(0);
    unsigned int position = 0;
    std::string::reverse_iterator characterIter = text.rbegin();
    for(/* empty */; (position < 4) && (characterIter != text.rend()); ++position, ++characterIter) {
      auto charAsInt = static_cast<unsigned int>(static_cast<unsigned char>(*characterIter));
      returnValue |= charAsInt << 4 * position;
    }

    return returnValue;
  }

  class MotorDriverCardTest {
   public:
    MotorDriverCardTest();

    // needed because the constructor of MotorDriverCardImpl is private, and the test is a friend
    static boost::shared_ptr<MotorDriverCardImpl> createCard(boost::shared_ptr<ChimeraTK::Device> const& device,
        std::string const& moduleName, MotorDriverCardConfig const& cardConfiguration);

    boost::shared_ptr<MotorDriverCardImpl> motorDriverCard;
    boost::shared_ptr<DFMC_MD22Dummy> dummyDevice;

    unsigned int asciiToInt(std::string text);
  };

  MotorDriverCardTest gTest;

  MotorDriverCardTest::MotorDriverCardTest() {
    ChimeraTK::setDMapFilePath("./dummies.dmap");
  }

  boost::shared_ptr<MotorDriverCardImpl> MotorDriverCardTest::createCard(
      boost::shared_ptr<ChimeraTK::Device> const& device, std::string const& moduleName,
      MotorDriverCardConfig const& cardConfiguration) {
    return boost::shared_ptr<MotorDriverCardImpl>(new MotorDriverCardImpl(device, moduleName, cardConfiguration));
  }

  void testConfiguration(MotorDriverCardConfig const& motorDriverCardConfig);

  BOOST_AUTO_TEST_CASE(TestConstuctor) {
    gTest.dummyDevice =
        boost::dynamic_pointer_cast<DFMC_MD22Dummy>(ChimeraTK::BackendFactory::getInstance().createBackend(DFMC_ALIAS));
    boost::shared_ptr<Device> device(new Device());

    MotorDriverCardConfig motorDriverCardConfig;
    motorDriverCardConfig.coverDatagram = asciiToInt("DTGR");
    motorDriverCardConfig.coverPositionAndLength.setDATA(asciiToInt("POSL"));
    motorDriverCardConfig.datagramHighWord = asciiToInt("DGRH");
    motorDriverCardConfig.datagramLowWord = asciiToInt("DGRL");
    motorDriverCardConfig.interfaceConfiguration.setDATA(asciiToInt("IFCF"));
    motorDriverCardConfig.positionCompareInterruptData.setDATA(asciiToInt("PCID"));
    motorDriverCardConfig.positionCompareWord = asciiToInt("POSC");
    motorDriverCardConfig.stepperMotorGlobalParameters.setDATA(asciiToInt("SMGP"));

    for(unsigned int motorID = 0; motorID < motorDriverCardConfig.motorControllerConfigurations.size(); ++motorID) {
      MotorControllerConfig motorControllerConfig;

      motorControllerConfig.accelerationThresholdData.setDATA(asciiToInt("THR") + motorID);
      // motorControllerConfig.actualPosition = asciiToInt("ACX") + motorID;;
      motorControllerConfig.chopperControlData.setPayloadData(asciiToInt("cp") + motorID);
      motorControllerConfig.coolStepControlData.setPayloadData(asciiToInt("cp") + motorID);
      motorControllerConfig.decoderReadoutMode = asciiToInt("DRM") + motorID;
      motorControllerConfig.dividersAndMicroStepResolutionData.setDATA(asciiToInt("MUSR") + motorID);
      motorControllerConfig.driverConfigData.setPayloadData(asciiToInt("cf") + motorID);
      motorControllerConfig.driverControlData.setPayloadData(asciiToInt("cl") + motorID);
      motorControllerConfig.enabled = true;
      motorControllerConfig.interruptData.setDATA(asciiToInt("INTR") + motorID);
      motorControllerConfig.maximumAcceleration = asciiToInt("am") + motorID;
      motorControllerConfig.maximumVelocity = asciiToInt("vm") + motorID;
      motorControllerConfig.microStepCount = asciiToInt("mu") + motorID;
      motorControllerConfig.minimumVelocity = asciiToInt("vn");
      motorControllerConfig.positionTolerance = asciiToInt("dx") + motorID;
      motorControllerConfig.proportionalityFactorData.setDATA(asciiToInt("PROP") + motorID);
      motorControllerConfig.referenceConfigAndRampModeData.setDATA(asciiToInt("REF") + motorID);
      motorControllerConfig.stallGuardControlData.setPayloadData(asciiToInt("sg") + motorID);
      motorControllerConfig.targetPosition = int32_t(asciiToInt("tx") + motorID);
      motorControllerConfig.targetVelocity = int32_t(asciiToInt("tv") + motorID);

      motorDriverCardConfig.motorControllerConfigurations[motorID] = motorControllerConfig;
    }

    // has to throw because the device is not open
    BOOST_CHECK_THROW(
        gTest.motorDriverCard = gTest.createCard(device, MODULE_NAME_0, motorDriverCardConfig), ChimeraTK::logic_error);

    // try opening with bad mapping, also has to throw
    //  boost::shared_ptr<ChimeraTK::DeviceBackend> dummyDevice ( new
    //  ChimeraTK::PcieBackend("/dev/mtcadummys0", BROKEN_MAP_FILE_NAME));
    boost::shared_ptr<ChimeraTK::DeviceBackend> brokenDummyDevice =
        boost::dynamic_pointer_cast<ChimeraTK::DeviceBackend>(
            ChimeraTK::BackendFactory::getInstance().createBackend(BROKEN_DUMMY_DEV_ALIAS));
    device->open(BROKEN_DUMMY_DEV_ALIAS);
    BOOST_CHECK_THROW(
        gTest.motorDriverCard = gTest.createCard(device, MODULE_NAME_0, motorDriverCardConfig), ChimeraTK::logic_error);
    device->close();

    // Opens the device corresponding to backend variable _dummyDevice
    device->open(DFMC_ALIAS);

    // try something with a wrong firmware version
    // wrong major (too large by 1):
    gTest.dummyDevice->setFirmwareVersion(dfmc_md22::MINIMAL_FIRMWARE_VERSION + 0x1000000);
    BOOST_CHECK_THROW(gTest.motorDriverCard = gTest.createCard(device, MODULE_NAME_0, motorDriverCardConfig),
        ChimeraTK::runtime_error);

    gTest.dummyDevice->setFirmwareVersion(dfmc_md22::MINIMAL_FIRMWARE_VERSION - 1);
    BOOST_CHECK_THROW(gTest.motorDriverCard = gTest.createCard(device, MODULE_NAME_0, motorDriverCardConfig),
        ChimeraTK::runtime_error);

    gTest.dummyDevice->resetFirmwareVersion();

    gTest.motorDriverCard = MotorDriverCardTest::createCard(device, MODULE_NAME_0, motorDriverCardConfig);

    // test that the configuration with the motorDriverCardConfig actually worked
    testConfiguration(motorDriverCardConfig);

    // only after initialising the dummy device with the motorDriverCardConfig in
    // the constructor of the MotorDriverCardImpl we can change the registers for
    // testing
    gTest.dummyDevice->setRegistersForTesting();
  }

  void testConfiguration(MotorDriverCardConfig const& motorDriverCardConfig) {
    // This test checks that the configuration is written and can be read back
    // under the assumption that the read function is working, which is only a
    // self-consistency test if the latter is not true. The test that the read
    // function actually does what it should is done later, using the
    // index-dependent test words from the dummy device. In this combination this
    // test shows that the config is written correctly.

    BOOST_CHECK(gTest.motorDriverCard->getCoverDatagram() == motorDriverCardConfig.coverDatagram);
    BOOST_CHECK(gTest.motorDriverCard->getCoverPositionAndLength() == motorDriverCardConfig.coverPositionAndLength);
    BOOST_CHECK(gTest.motorDriverCard->getDatagramHighWord() == motorDriverCardConfig.datagramHighWord);
    BOOST_CHECK(gTest.motorDriverCard->getDatagramLowWord() == motorDriverCardConfig.datagramLowWord);
    BOOST_CHECK(gTest.motorDriverCard->getInterfaceConfiguration() == motorDriverCardConfig.interfaceConfiguration);
    BOOST_CHECK(
        gTest.motorDriverCard->getPositionCompareInterruptData() == motorDriverCardConfig.positionCompareInterruptData);
    BOOST_CHECK(gTest.motorDriverCard->getPositionCompareWord() == motorDriverCardConfig.positionCompareWord);
    BOOST_CHECK(
        gTest.motorDriverCard->getStepperMotorGlobalParameters() == motorDriverCardConfig.stepperMotorGlobalParameters);

    for(unsigned int motorID = 0; motorID < motorDriverCardConfig.motorControllerConfigurations.size(); ++motorID) {
      boost::shared_ptr<MotorControllerExpert> motorController =
          boost::dynamic_pointer_cast<MotorControllerExpert>(gTest.motorDriverCard->getMotorController(motorID));
      MotorControllerConfig motorControllerConfig = motorDriverCardConfig.motorControllerConfigurations[motorID];

      // only the data content is identical. The motorID is - for the config, and
      // correct for the readback word
      BOOST_CHECK(motorControllerConfig.accelerationThresholdData.getDATA() ==
          motorController->getAccelerationThresholdData().getDATA());
      // BOOST_CHECK( motorControllerConfig.actualPosition ==
      // motorController->getActualPosition() );
      BOOST_CHECK(motorControllerConfig.chopperControlData == motorController->getChopperControlData());
      BOOST_CHECK(motorControllerConfig.coolStepControlData == motorController->getCoolStepControlData());
      BOOST_CHECK(motorControllerConfig.decoderReadoutMode == motorController->getDecoderReadoutMode());
      BOOST_CHECK(motorControllerConfig.dividersAndMicroStepResolutionData.getDATA() ==
          motorController->getDividersAndMicroStepResolutionData().getDATA());
      BOOST_CHECK(motorControllerConfig.driverConfigData == motorController->getDriverConfigData());
      BOOST_CHECK(motorControllerConfig.driverControlData == motorController->getDriverControlData());
      BOOST_CHECK(motorControllerConfig.enabled == motorController->isEnabled());
      BOOST_CHECK(motorControllerConfig.interruptData.getDATA() == motorController->getInterruptData().getDATA());
      BOOST_CHECK(motorControllerConfig.maximumAcceleration == motorController->getMaximumAcceleration());
      BOOST_CHECK(motorControllerConfig.maximumVelocity == motorController->getMaximumVelocity());
      BOOST_CHECK(motorControllerConfig.microStepCount == motorController->getMicroStepCount());
      BOOST_CHECK(motorControllerConfig.minimumVelocity == motorController->getMinimumVelocity());
      BOOST_CHECK(motorControllerConfig.positionTolerance == motorController->getPositionTolerance());
      BOOST_CHECK(motorControllerConfig.proportionalityFactorData.getDATA() ==
          motorController->getProportionalityFactorData().getDATA());
      BOOST_CHECK(motorControllerConfig.referenceConfigAndRampModeData.getDATA() ==
          motorController->getReferenceConfigAndRampModeData().getDATA());
      BOOST_CHECK(motorControllerConfig.stallGuardControlData == motorController->getStallGuardControlData());
      // BOOST_CHECK( motorControllerConfig.targetPosition ==
      // motorController->getTargetPosition() );
      BOOST_CHECK(motorControllerConfig.targetVelocity == motorController->getTargetVelocity());
    }
  }

  BOOST_AUTO_TEST_CASE(TestGetControllerChipVersion) {
    unsigned int expectedContent = testWordFromSpiAddress(SMDA_COMMON, JDX_CHIP_VERSION);
    BOOST_CHECK(gTest.motorDriverCard->getControllerChipVersion() == expectedContent);
  }

  BOOST_AUTO_TEST_CASE(TestGetReferenceSwitchData) {
    // seems like a self consistency test, but ReferenceSwitchData has been testes
    // in testTMC429Words, and the data content should be the test word
    unsigned int expectedContent = testWordFromSpiAddress(SMDA_COMMON, JDX_REFERENCE_SWITCH);
    BOOST_CHECK(gTest.motorDriverCard->getReferenceSwitchData() == ReferenceSwitchData(expectedContent));
  }

  BOOST_AUTO_TEST_CASE(TestGetStatusWord) {
    auto registerInfo = gTest.dummyDevice->getRegisterInfo(MODULE_NAME_0 / CONTROLLER_STATUS_BITS_ADDRESS_STRING);

    unsigned int expectedContent = testWordFromPCIeAddress(registerInfo.address);

    BOOST_CHECK(gTest.motorDriverCard->getStatusWord().getDataWord() == expectedContent);
  }

  BOOST_AUTO_TEST_CASE(TestGetDatagramLowWord) {
    unsigned int expectedContent = testWordFromSpiAddress(SMDA_COMMON, JDX_DATAGRAM_LOW_WORD);
    BOOST_CHECK(gTest.motorDriverCard->getDatagramLowWord() == expectedContent);
  }

  // just for the fun of it I use AAAA, 5555 and FFFF alternating as test patterns
  BOOST_AUTO_TEST_CASE(TestSetDatagramLowWord) {
    gTest.motorDriverCard->setDatagramLowWord(0xAAAAAA);
    BOOST_CHECK(gTest.motorDriverCard->getDatagramLowWord() == 0xAAAAAA);
  }

  BOOST_AUTO_TEST_CASE(TestGetDatagramHighWord) {
    unsigned int expectedContent = testWordFromSpiAddress(SMDA_COMMON, JDX_DATAGRAM_HIGH_WORD);
    BOOST_CHECK(gTest.motorDriverCard->getDatagramHighWord() == expectedContent);
  }

  BOOST_AUTO_TEST_CASE(TestSetDatagramHighWord) {
    gTest.motorDriverCard->setDatagramHighWord(0x555555);
    BOOST_CHECK(gTest.motorDriverCard->getDatagramHighWord() == 0x555555);
  }

  BOOST_AUTO_TEST_CASE(TestGetCoverPositionAndLength) {
    unsigned int expectedContent = testWordFromSpiAddress(SMDA_COMMON, JDX_COVER_POSITION_AND_LENGTH);
    BOOST_CHECK(gTest.motorDriverCard->getCoverPositionAndLength() == CoverPositionAndLength(expectedContent));
  }

  BOOST_AUTO_TEST_CASE(TestSetCoverPositionAndLength) {
    gTest.motorDriverCard->setCoverPositionAndLength(0xFFFFFF);
    BOOST_CHECK(gTest.motorDriverCard->getCoverPositionAndLength() == CoverPositionAndLength(0xFFFFFF));
  }

  BOOST_AUTO_TEST_CASE(TestGetCoverDatagram) {
    unsigned int expectedContent = testWordFromSpiAddress(SMDA_COMMON, JDX_COVER_DATAGRAM);
    BOOST_CHECK(gTest.motorDriverCard->getCoverDatagram() == expectedContent);
  }

  BOOST_AUTO_TEST_CASE(TestSetCoverDatagram) {
    gTest.motorDriverCard->setCoverDatagram(0xAAAAAA);
    BOOST_CHECK(gTest.motorDriverCard->getCoverDatagram() == 0xAAAAAA);
  }

  BOOST_AUTO_TEST_CASE(TestGetStepperMotorGlobalParameters) {
    unsigned int expectedContent = testWordFromSpiAddress(SMDA_COMMON, JDX_STEPPER_MOTOR_GLOBAL_PARAMETERS);
    BOOST_CHECK(
        gTest.motorDriverCard->getStepperMotorGlobalParameters() == StepperMotorGlobalParameters(expectedContent));
  }

  BOOST_AUTO_TEST_CASE(TestSetStepperMotorGlobalParameters) {
    gTest.motorDriverCard->setStepperMotorGlobalParameters(0x555555);
    BOOST_CHECK(gTest.motorDriverCard->getStepperMotorGlobalParameters() == StepperMotorGlobalParameters(0x555555));
  }

  BOOST_AUTO_TEST_CASE(TestGetInterfaceConfiguration) {
    unsigned int expectedContent = testWordFromSpiAddress(SMDA_COMMON, JDX_INTERFACE_CONFIGURATION);
    BOOST_CHECK(gTest.motorDriverCard->getInterfaceConfiguration() == InterfaceConfiguration(expectedContent));
  }

  BOOST_AUTO_TEST_CASE(TestSetInterfaceConfiguration) {
    gTest.motorDriverCard->setInterfaceConfiguration(0x555555);
    BOOST_CHECK(gTest.motorDriverCard->getInterfaceConfiguration() == InterfaceConfiguration(0x555555));
  }

  BOOST_AUTO_TEST_CASE(TestGetPositionCompareWord) {
    unsigned int expectedContent = testWordFromSpiAddress(SMDA_COMMON, JDX_POSITION_COMPARE);
    BOOST_CHECK(gTest.motorDriverCard->getPositionCompareWord() == expectedContent);
  }

  BOOST_AUTO_TEST_CASE(TestSetPositionCompareWord) {
    gTest.motorDriverCard->setPositionCompareWord(0xFFFFFF);
    BOOST_CHECK(gTest.motorDriverCard->getPositionCompareWord() == 0xFFFFFF);
  }

  BOOST_AUTO_TEST_CASE(TestGetPositionCompareInterruptData) {
    unsigned int expectedContent = testWordFromSpiAddress(SMDA_COMMON, JDX_POSITION_COMPARE_INTERRUPT);
    BOOST_CHECK(
        gTest.motorDriverCard->getPositionCompareInterruptData() == PositionCompareInterruptData(expectedContent));
  }

  BOOST_AUTO_TEST_CASE(TestSetPositionCompareInterruptData) {
    gTest.motorDriverCard->setPositionCompareInterruptData(0xAAAAAA);
    BOOST_CHECK(gTest.motorDriverCard->getPositionCompareInterruptData() == PositionCompareInterruptData(0xAAAAAA));
  }

  BOOST_AUTO_TEST_CASE(TestPowerDown) {
    gTest.motorDriverCard->powerDown();
    BOOST_CHECK(gTest.dummyDevice->isPowerUp() == false);

    gTest.dummyDevice->powerUp();
    BOOST_CHECK(gTest.dummyDevice->isPowerUp() == true);
  }

  BOOST_AUTO_TEST_CASE(TestGetMotorController) {
    for(unsigned int i = 0; i < N_MOTORS_MAX; ++i) {
      BOOST_CHECK(gTest.motorDriverCard->getMotorController(i)->getID() == i);
    }
    BOOST_CHECK_THROW(gTest.motorDriverCard->getMotorController(N_MOTORS_MAX), ChimeraTK::logic_error);
  }

} // namespace ChimeraTK::MotorDriver
