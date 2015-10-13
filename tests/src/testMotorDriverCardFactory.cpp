#define BOOST_TEST_MODULE MotorDriverCardFactoryTest
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "MotorDriverCardFactory.h"
#include <mtca4u/Device.h>
#include "DFMC_MD22Constants.h"
#include "MotorDriverCardDummy.h"
#include <mtca4u/PcieBackend.h>

//#include <mtca4u/BackendFactory.h>
#include "testConfigConstants.h"

using namespace mtca4u;
using namespace mtca4u::dfmc_md22;

BOOST_AUTO_TEST_SUITE( MotorDriverCardFactoryTestSuite )

BOOST_AUTO_TEST_CASE( testInstance ){
	// check that the singleton pattern works
	MotorDriverCardFactory const & instance1 = MotorDriverCardFactory::instance();
	MotorDriverCardFactory const & instance2 = MotorDriverCardFactory::instance();

	BOOST_CHECK( &instance1 == &instance2);
}

BOOST_AUTO_TEST_CASE( testCreate ){
	// test the creation with PcieDevice, mapping file and device file not being the same

	// Prepare the dummy device. It has to have the correct firmware number in the
	// corresponding register.
	//boost::shared_ptr < Device<BaseDevice> > 	mappedMtcadummy = DeviceFactory::getInstance().createDevice(DUMMY_DEV_ALIAS);
	 std::string testFilePath = boost::filesystem::initial_path().string() + (std::string)TEST_DMAP_FILE_PATH;
	 BackendFactory::getInstance().setDMapFilePath(testFilePath);

	boost::shared_ptr < Device > 	mappedMtcadummy (new Device());
	mappedMtcadummy->open(DUMMY_DEV_ALIAS);
	//mappedMtcadummy.openDev(DUMMY_DEV_PATH, DUMMY_MOC_MAP);
	mappedMtcadummy->writeReg(PROJECT_VERSION_ADDRESS_STRING, MODULE_NAME_0,  &MINIMAL_FIRMWARE_VERSION);
	boost::shared_ptr<MotorDriverCard> motorDriverCard_PCIe1 =
			//MotorDriverCardFactory::instance().createMotorDriverCard(DUMMY_DEV_PATH,
			MotorDriverCardFactory::instance().createMotorDriverCard(DUMMY_DEV_ALIAS,
					MODULE_NAME_0,CONFIG_FILE);
	boost::shared_ptr<MotorDriverCard> md22_dummy1 =
			MotorDriverCardFactory::instance().createMotorDriverCard(DFMC_ALIAS,
					MODULE_NAME_0, CONFIG_FILE);
	BOOST_CHECK( motorDriverCard_PCIe1.get() != md22_dummy1.get() );
	// there is one instance here and one in the factory
	BOOST_CHECK( md22_dummy1.use_count() == 2 );
	boost::shared_ptr<MotorDriverCard> md22_dummy2 =
			MotorDriverCardFactory::instance().createMotorDriverCard(DFMC_ALIAS,
					MODULE_NAME_0, CONFIG_FILE);

	BOOST_CHECK( md22_dummy1.get() == md22_dummy2.get() );
	// there are two instances here, and one in the factory
	BOOST_CHECK( md22_dummy1.use_count() == 3 );

	// change the firmware version to 0. Still 'creation' has to work because the
	// device must not be reopened but the same instance has to be used.

	mappedMtcadummy->writeReg(PROJECT_VERSION_ADDRESS_STRING, MODULE_NAME_0, &MINIMAL_FIRMWARE_VERSION);
	boost::shared_ptr<MotorDriverCard> motorDriverCard_PCIe2 =
			MotorDriverCardFactory::instance().createMotorDriverCard(DUMMY_DEV_ALIAS,
					MODULE_NAME_0, CONFIG_FILE);

	BOOST_CHECK( motorDriverCard_PCIe1.get() == motorDriverCard_PCIe2.get() );
	// there are two instances here, and one in the factory
	BOOST_CHECK( motorDriverCard_PCIe1.use_count() == 3 );
}

BOOST_AUTO_TEST_CASE( testCreateDummy ){
	MotorDriverCardFactory::instance().setDummyMode(true);
	boost::shared_ptr<MotorDriverCardDummy> motorDriverCardDummy =
			boost::dynamic_pointer_cast<MotorDriverCardDummy>(
					MotorDriverCardFactory::instance().createMotorDriverCard("/dummy/MotorDriverCard",

							MODULE_NAME_0,
							"alsoIrrelevant"));

	BOOST_CHECK(motorDriverCardDummy);
}


BOOST_AUTO_TEST_SUITE_END()
