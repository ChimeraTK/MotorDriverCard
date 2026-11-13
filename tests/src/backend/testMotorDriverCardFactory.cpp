#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE MotorDriverCardFactoryTest
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "DFMC_MD22Constants.h"
#include "MotorControler.h"
#include "MotorDriverCard.h"
#include "MotorDriverCardDummy.h"
#include "MotorDriverCardFactory.h"
#include "testConfigConstants.h"

#include <ChimeraTK/BackendFactory.h>
#include <ChimeraTK/Device.h>

#include <boost/filesystem.hpp>

// using namespace mtca4u;
// using namespace mtca4u::dfmc_md22;

BOOST_AUTO_TEST_SUITE(MotorDriverCardFactoryTestSuite)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(testInstance) {
#pragma GCC diagnostic pop
  // check that the singleton pattern works
  mtca4u::MotorDriverCardFactory const& instance1 = mtca4u::MotorDriverCardFactory::instance();
  mtca4u::MotorDriverCardFactory const& instance2 = mtca4u::MotorDriverCardFactory::instance();

  BOOST_CHECK(&instance1 == &instance2);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(testGetSetDMapFilePath) {
#pragma GCC diagnostic pop
  // make sure there is something different set than the one we want to test
  // we have to use real files, otherwise we get an exception here.
  ChimeraTK::BackendFactory::getInstance().setDMapFilePath("MD22_on_DAMC2.dmap");
  // check that you can read it through the MotorDriverCardFactory interface
  BOOST_CHECK(mtca4u::MotorDriverCardFactory::getDeviceaccessDMapFilePath() == "MD22_on_DAMC2.dmap");

  // set using the MotorDriverCardFactory's interface
  mtca4u::MotorDriverCardFactory::setDeviceaccessDMapFilePath("dummies.dmap");
  // check that it arrived correctly in the BackendFactory
  BOOST_CHECK(ChimeraTK::BackendFactory::getInstance().getDMapFilePath() == "dummies.dmap");
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(testCreate) {
#pragma GCC diagnostic pop
  // test the creation with PcieDevice, mapping file and device file not being
  // the same

  // Prepare the dummy device. It has to have the correct firmware number in the
  // corresponding register.
  // boost::shared_ptr < Device<BaseDevice> > 	mappedMtcadummy =
  // DeviceFactory::getInstance().createDevice(DUMMY_DEV_ALIAS);
  std::string testFilePath = "dummies.dmap";
  ChimeraTK::BackendFactory::getInstance().setDMapFilePath(testFilePath);

  boost::shared_ptr<mtca4u::MotorDriverCard> motorDriverCard_PCIe1 =
      // MotorDriverCardFactory::instance().createMotorDriverCard(DUMMY_DEV_PATH,
      mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(DFMC_ALIAS2, MODULE_NAME_0, CONFIG_FILE);
  BOOST_CHECK(true);
  boost::shared_ptr<mtca4u::MotorDriverCard> md22_dummy1 =
      mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(DFMC_ALIAS, MODULE_NAME_0, CONFIG_FILE);
  BOOST_CHECK(motorDriverCard_PCIe1.get() != md22_dummy1.get());
  // there is one instance here. The factory has a weak reference
  BOOST_CHECK_EQUAL(md22_dummy1.use_count(), 1);
  boost::shared_ptr<mtca4u::MotorDriverCard> md22_dummy2 =
      mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(DFMC_ALIAS, MODULE_NAME_0, CONFIG_FILE);

  BOOST_CHECK_EQUAL(md22_dummy1.get(), md22_dummy2.get());
  // there are two instances here, a weak one in the factory
  BOOST_CHECK_EQUAL(md22_dummy1.use_count(), 2);

  boost::shared_ptr<mtca4u::MotorDriverCard> motorDriverCard_PCIe2 =
      mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(DFMC_ALIAS2, MODULE_NAME_0, CONFIG_FILE);

  BOOST_CHECK_EQUAL(motorDriverCard_PCIe1.get(), motorDriverCard_PCIe2.get());
  // there are two instances here, a weak one in the factory
  BOOST_CHECK_EQUAL(motorDriverCard_PCIe1.use_count(), 2);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(testCreateDummy) {
#pragma GCC diagnostic pop
  mtca4u::MotorDriverCardFactory::instance().setDummyMode(true);
  boost::shared_ptr<mtca4u::MotorDriverCardDummy> motorDriverCardDummy =
      boost::dynamic_pointer_cast<mtca4u::MotorDriverCardDummy>(
          mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard("/dummy/MotorDriverCard",

              MODULE_NAME_0, "alsoIrrelevant"));

  BOOST_CHECK(motorDriverCardDummy);
}

/**
 * @anchor testCreateFromDeviceReturnsSameCard
 * MotorDriverCardFactory::createMotorDriverCard(ChimeraTK::Device&, ...)
 *
 * "Repeated requests for the same device and module return the same card while
 * the previously returned card is still alive." and "The guarantee is one card
 * per device and module, matching the cache key."
 */
BOOST_AUTO_TEST_CASE(CreateFromDeviceReturnsTheSameCard) {
  ChimeraTK::BackendFactory::getInstance().setDMapFilePath("dummies.dmap");
  ChimeraTK::Device device;
  device.open(DFMC_ALIAS);

  boost::shared_ptr<mtca4u::MotorDriverCard> card0 =
      mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(device, MODULE_NAME_0, CONFIG_FILE);
  boost::shared_ptr<mtca4u::MotorDriverCard> sameCard0 =
      mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(device, MODULE_NAME_0, CONFIG_FILE);

  BOOST_TEST(card0.get() == sameCard0.get());

  // newer_firmware_mapfile.map defines MD22_1 at the same addresses as MD22_0,
  // so the second card configures the same dummy registers. Assert only that
  // the pointer differs.
  boost::shared_ptr<mtca4u::MotorDriverCard> card1 =
      mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(device, MODULE_NAME_1, CONFIG_FILE);

  BOOST_TEST(card1.get() != card0.get());
}

/**
 * @anchor testCreateFromDeviceIgnoresDummyMode
 * MotorDriverCardFactory::createMotorDriverCard(ChimeraTK::Device&, ...)
 *
 * "Dummy mode does not apply to an injected device." and "The overload ignores
 * dummy mode."
 */
BOOST_AUTO_TEST_CASE(CreateFromDeviceIgnoresDummyMode) {
  ChimeraTK::BackendFactory::getInstance().setDMapFilePath("dummies.dmap");

  auto& factory = mtca4u::MotorDriverCardFactory::instance();
  // The factory is a singleton and the alias-based cases above run with the
  // dummy backend, so restore the flag before leaving.
  factory.setDummyMode(true);

  ChimeraTK::Device device;
  device.open(DFMC_ALIAS);
  boost::shared_ptr<mtca4u::MotorDriverCard> card = factory.createMotorDriverCard(device, MODULE_NAME_0, CONFIG_FILE);
  factory.setDummyMode(false);

  BOOST_TEST(!boost::dynamic_pointer_cast<mtca4u::MotorDriverCardDummy>(card));
  BOOST_TEST(card);
  // A real card reads the configuration back from the dummy registers.
  BOOST_TEST(card->isFunctional());
  BOOST_TEST(card->getMotorControler(0)->getID() == 0u);
}

/**
 * @anchor testCreateFromDeviceKeepsItOpen
 * MotorDriverCardFactory::createMotorDriverCard(ChimeraTK::Device&, ...)
 *
 * "The library never opens, closes or replaces an injected device." and "The
 * injected device must outlive the card."
 */
BOOST_AUTO_TEST_CASE(CreateFromDeviceKeepsItOpen) {
  ChimeraTK::BackendFactory::getInstance().setDMapFilePath("dummies.dmap");
  ChimeraTK::Device device;
  device.open(DFMC_ALIAS);

  {
    boost::shared_ptr<mtca4u::MotorDriverCard> card =
        mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard(device, MODULE_NAME_0, CONFIG_FILE);
    BOOST_TEST(device.isOpened());
    BOOST_TEST(card->isFunctional());
  }

  // Releasing the card must not close the device the caller owns.
  BOOST_TEST(device.isOpened());
  BOOST_TEST(device.isFunctional());
}

BOOST_AUTO_TEST_SUITE_END()
