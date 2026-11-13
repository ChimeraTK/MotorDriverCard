#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE testMotorDriverCardRecovery

#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "DFMC_MD22Constants.h"
#include "DFMC_MD22Dummy.h"
#include "impl/MotorDriverCardImpl.h"
#include "MotorDriverCardFactory.h"
#include "testConfigConstants.h"
#include "TMC429Words.h"

#include <ChimeraTK/BackendFactory.h>
#include <ChimeraTK/Device.h>
#include <ChimeraTK/Exception.h>

#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace mtca4u::dfmc_md22;

namespace mtca4u {

  /**
   * The cards are created through the injected factory overload, so the private
   * implementation constructor needs no test friend. The emulated registers are
   * read through a dummy backend handle of the test's own.
   */
  class MotorDriverCardRecoveryTest {
   public:
    MotorDriverCardRecoveryTest() : _device(), _dummyBackend(), _card(), _configFile(CONFIG_FILE) {
      ChimeraTK::BackendFactory::getInstance().setDMapFilePath("dummies.dmap");
      _dummyBackend = boost::dynamic_pointer_cast<DFMC_MD22Dummy>(
          ChimeraTK::BackendFactory::getInstance().createBackend(DFMC_ALIAS));
    }

    /// Open the device and create the card on it through the injected overload.
    void createCard(std::string const& configFile) {
      _configFile = configFile;
      _device.open(DFMC_ALIAS);
      _card = MotorDriverCardFactory::instance().createMotorDriverCard(_device, MODULE_NAME_0, _configFile);
    }

    /// Close and re-open the device, which invalidates all accessors.
    void reopenDevice() {
      _device.close();
      _device.open(DFMC_ALIAS);
    }

    /**
     * Reset the emulated registers to values that differ from the configured
     * ones: the emulated controller and driver SPI spaces are filled with test
     * patterns and mirrored into the FPGA registers. This is what a device reset
     * looks like to the card.
     *
     * setRegistersForTesting() leaves the enable registers at the test pattern,
     * which decodes as enabled, so they are cleared explicitly.
     */
    void disturbRegisters() {
      _dummyBackend->setRegistersForTesting();
      writeRegister(CONTROLER_STATUS_BITS_ADDRESS_STRING, 0);
      for(unsigned int motorID = 0; motorID < N_MOTORS_MAX; ++motorID) {
        writeRegister(createMotorRegisterName(motorID, MOTOR_CURRENT_ENABLE_SUFFIX), 0);
      }
    }

    /// Read an FPGA register through the device, bypassing the card's accessors.
    unsigned int readRegister(std::string const& registerName) {
      // The register information is fetched first, so the dummy backend knows the register.
      _dummyBackend->getRegisterInfo(_moduleName / registerName);
      auto accessor =
          _device.getScalarRegisterAccessor<int32_t>(_moduleName / registerName, 0, {ChimeraTK::AccessMode::raw});
      accessor.read();
      return static_cast<unsigned int>(accessor);
    }

    /// Write an FPGA register through the device, bypassing the card's accessors.
    void writeRegister(std::string const& registerName, int32_t value) {
      _dummyBackend->getRegisterInfo(_moduleName / registerName);
      auto accessor =
          _device.getScalarRegisterAccessor<int32_t>(_moduleName / registerName, 0, {ChimeraTK::AccessMode::raw});
      accessor = value;
      accessor.write();
    }

    unsigned int readMotorRegister(unsigned int motorID, std::string const& suffix) {
      return readRegister(createMotorRegisterName(motorID, suffix));
    }

    MotorDriverCardImpl& cardImpl() {
      // getStatusWord() is a public member of the implementation only.
      boost::shared_ptr<MotorDriverCardImpl> impl = boost::dynamic_pointer_cast<MotorDriverCardImpl>(_card);
      assert(impl);
      return *impl;
    }

    ChimeraTK::Device _device;
    boost::shared_ptr<DFMC_MD22Dummy> _dummyBackend;
    boost::shared_ptr<MotorDriverCard> _card;
    std::string _configFile;

    ChimeraTK::RegisterPath _moduleName{MODULE_NAME_0};
  };

} // namespace mtca4u

using mtca4u::MotorDriverCardRecoveryTest;

/**
 * @anchor ReconfigureRestoresConfiguration
 * MotorDriverCard::reconfigure()
 *
 * "It re-applies the complete card and motor configuration from the
 * configuration objects already held by the library."
 */
BOOST_FIXTURE_TEST_CASE(ReconfigureRestoresConfiguration, MotorDriverCardRecoveryTest) {
  createCard(CONFIG_FILE);

  // Take the configured values from the freshly configured card, so the case
  // does not duplicate the configuration file.
  auto controler = _card->getMotorControler(0);
  const unsigned int configuredMaximumAcceleration = controler->getMaximumAcceleration();
  const unsigned int configuredMaximumVelocity = controler->getMaximumVelocity();
  const unsigned int configuredMicroStepCount = controler->getMicroStepCount();
  const unsigned int configuredPositionTolerance = controler->getPositionTolerance();

  disturbRegisters();
  BOOST_TEST(controler->getMaximumAcceleration() != configuredMaximumAcceleration);

  _card->reconfigure();

  BOOST_TEST(controler->getMaximumAcceleration() == configuredMaximumAcceleration);
  BOOST_TEST(controler->getMaximumVelocity() == configuredMaximumVelocity);
  BOOST_TEST(controler->getMicroStepCount() == configuredMicroStepCount);
  BOOST_TEST(controler->getPositionTolerance() == configuredPositionTolerance);
}

/**
 * @anchor ReconfigureKeepsStatusWordReadable
 * MotorDriverCardImpl::getStatusWord()
 *
 * "Re-creates `_controlerStatusRegister` with `{ChimeraTK::AccessMode::raw}`"
 * and "`getStatusWord()` also locks `_reconfigureMutex`."
 */
BOOST_FIXTURE_TEST_CASE(ReconfigureKeepsStatusWordReadable, MotorDriverCardRecoveryTest) {
  createCard(CONFIG_FILE);

  reopenDevice();
  _card->reconfigure();

  // The status word is a slice of the status register, so the raw access mode
  // is part of what is read back.
  const unsigned int statusRegister = readRegister(CONTROLER_STATUS_BITS_ADDRESS_STRING);
  BOOST_TEST(cardImpl().getStatusWord().getDataWord() == mtca4u::TMC429StatusWord(statusRegister).getDataWord());
}

/**
 * @anchor ReconfigureLeavesMotorDisabledByDefault
 * MotorDriverCard::reconfigure(bool enableMotor)
 *
 * "It leaves the motor current disabled unless the caller asks to enable it."
 */
BOOST_FIXTURE_TEST_CASE(ReconfigureLeavesMotorDisabledByDefault, MotorDriverCardRecoveryTest) {
  // CONFIG_FILE carries no enabled register and leaves the motors disabled, so
  // the enabling configuration is used here.
  createCard("MotorDriverCardConfig_complete_test.xml");

  BOOST_TEST(_card->getMotorControler(0)->isMotorCurrentEnabled());

  disturbRegisters();
  BOOST_TEST(!_card->getMotorControler(0)->isMotorCurrentEnabled());

  _card->reconfigure();
  BOOST_TEST(!_card->getMotorControler(0)->isMotorCurrentEnabled());
  BOOST_TEST(!_card->getMotorControler(1)->isMotorCurrentEnabled());

  _card->reconfigure(true);
  BOOST_TEST(_card->getMotorControler(0)->isMotorCurrentEnabled());
  BOOST_TEST(_card->getMotorControler(1)->isMotorCurrentEnabled());
}

/**
 * @anchor ReconfigureKeepsControlerInstances
 * MotorDriverCard::reconfigure()
 *
 * "Existing `MotorControler` references stay valid across a reconfigure."
 */
BOOST_FIXTURE_TEST_CASE(ReconfigureKeepsControlerInstances, MotorDriverCardRecoveryTest) {
  createCard(CONFIG_FILE);

  auto controler0 = _card->getMotorControler(0);
  auto controler1 = _card->getMotorControler(1);
  mtca4u::MotorControler* rawControler0 = controler0.get();

  _card->reconfigure();

  BOOST_TEST(_card->getMotorControler(0).get() == rawControler0);
  BOOST_TEST(controler0.get() == rawControler0);
  BOOST_TEST(controler0->getID() == 0u);
  BOOST_TEST(controler1->getID() == 1u);
}

/**
 * @anchor ReconfigureLeavesPositionUntouched
 * MotorDriverCard::reconfigure()
 *
 * "It leaves the actual position untouched."
 */
BOOST_FIXTURE_TEST_CASE(ReconfigureLeavesPositionUntouched, MotorDriverCardRecoveryTest) {
  createCard(CONFIG_FILE);

  reopenDevice();
  // The re-open zeroes the emulated controller SPI space and mirrors it into
  // the actual-position register, so the reference is read after the re-open
  // and not before the close.
  const unsigned int positionAfterReopen = readMotorRegister(0, ACTUAL_POSITION_SUFFIX);

  _card->reconfigure();

  // A re-enabled position write would mirror the distinctive position into the
  // register; the configured target position of CONFIG_FILE is not distinctive.
  BOOST_TEST(readMotorRegister(0, ACTUAL_POSITION_SUFFIX) == positionAfterReopen);
}

/**
 * @anchor ReconfigureResetsRuntimeState
 * MotorControlerImpl::reconfigure(bool)
 *
 * "Resets `_currentVmax` to the configured maximum velocity."
 */
BOOST_FIXTURE_TEST_CASE(ReconfigureResetsRuntimeState, MotorDriverCardRecoveryTest) {
  createCard(CONFIG_FILE);

  auto controler = _card->getMotorControler(0);
  const double configuredMaximum = controler->getMaxSpeedCapability();
  const double reducedLimit = controler->setUserSpeedLimit(0.5 * configuredMaximum);
  BOOST_TEST(reducedLimit < configuredMaximum);

  _card->reconfigure();

  BOOST_TEST(controler->getUserSpeedLimit() > reducedLimit);
  BOOST_TEST(controler->getUserSpeedLimit() <= configuredMaximum);
}

/**
 * @anchor ReconfigureOnClosedDeviceThrows
 * MotorDriverCard::reconfigure()
 *
 * "A failed reconfigure throws; the caller must then not use the card except to
 * reopen the device and call `reconfigure()` again."
 */
BOOST_FIXTURE_TEST_CASE(ReconfigureOnClosedDeviceThrows, MotorDriverCardRecoveryTest) {
  createCard(CONFIG_FILE);

  _device.close();

  BOOST_CHECK_THROW(_card->reconfigure(), ChimeraTK::logic_error);

  reopenDevice();
  BOOST_CHECK_NO_THROW(_card->reconfigure());
  BOOST_TEST(_card->getMotorControler(0)->getID() == 0u);
}

/**
 * @anchor RepeatedReconfigureLeavesConfiguredValues
 * MotorDriverCard::reconfigure()
 *
 * "Repeated calls leave the configured values unchanged."
 */
BOOST_FIXTURE_TEST_CASE(RepeatedReconfigureLeavesConfiguredValues, MotorDriverCardRecoveryTest) {
  createCard(CONFIG_FILE);

  auto controler = _card->getMotorControler(0);

  reopenDevice();
  _card->reconfigure();
  const unsigned int acceleration = controler->getMaximumAcceleration();
  const unsigned int velocity = controler->getMaximumVelocity();
  const unsigned int tolerance = controler->getPositionTolerance();
  const unsigned int microStepCount = controler->getMicroStepCount();

  reopenDevice();
  _card->reconfigure();
  _card->reconfigure();

  BOOST_TEST(controler->getMaximumAcceleration() == acceleration);
  BOOST_TEST(controler->getMaximumVelocity() == velocity);
  BOOST_TEST(controler->getPositionTolerance() == tolerance);
  BOOST_TEST(controler->getMicroStepCount() == microStepCount);
}

/**
 * @anchor IsFunctionalFollowsTheDevice
 * MotorDriverCard::isFunctional()
 *
 * "`isFunctional()` reports whether the underlying device is functional."
 */
BOOST_FIXTURE_TEST_CASE(IsFunctionalFollowsTheDevice, MotorDriverCardRecoveryTest) {
  createCard(CONFIG_FILE);
  BOOST_TEST(_card->isFunctional());

  _device.close();
  BOOST_TEST(!_card->isFunctional());

  reopenDevice();
  BOOST_TEST(_card->isFunctional());
}
