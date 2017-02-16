#include <sstream>

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "DFMC_MD22Constants.h"
#include <mtca4u/Device.h>
#include <mtca4u/MapFileParser.h>
#include "SPIviaPCIe.h"
#include "DFMC_MD22Dummy.h"
#include "DFMC_MD22Constants.h"
#include "TMC260Words.h"
#include "TMC429Words.h"
#include "MotorDriverException.h" 
using namespace mtca4u;
using namespace mtca4u::dfmc_md22;

#include "testConfigConstants.h"

class SPIviaPCIeTest {
public:

    SPIviaPCIeTest(std::string const & mapFileName, std::string const & moduleName);
    //  static void testConstructors();

    void testRead();
    void testWrite();
    void testGetSetWaitingTime();

private:
    boost::shared_ptr<DFMC_MD22Dummy> _dummyBackend;
    boost::shared_ptr< Device > _device;
    std::string _mapFileName;

    boost::shared_ptr<SPIviaPCIe> _readWriteSPIviaPCIe; // use controler which has read/write
    boost::shared_ptr<SPIviaPCIe> _writeSPIviaPCIe; // use a motor address which has debug readback in the dummy
};

class SPIviaPCIeTestSuite : public test_suite {
public:

    SPIviaPCIeTestSuite(std::string const & mapFileName, std::string const & moduleName) : test_suite("SPIviaPCIe test suite") {
        // create an instance of the test class
        boost::shared_ptr<SPIviaPCIeTest> spiViaPCIeTest(new SPIviaPCIeTest(mapFileName, moduleName));

        // add the tests
        //   add( BOOST_CLASS_TEST_CASE( &SPIviaPCIeTest::testConstructors, spiViaPCIeTest ) );

        // in case of dependencies store the test cases before adding them and declare the dependency
        test_case* writeTestCase = BOOST_CLASS_TEST_CASE(&SPIviaPCIeTest::testWrite, spiViaPCIeTest);
        test_case* readTestCase = BOOST_CLASS_TEST_CASE(&SPIviaPCIeTest::testRead, spiViaPCIeTest);

        // exceptionally the read depends on the write (usually it is the other way round)
        readTestCase->depends_on(writeTestCase);

        add(writeTestCase);
        add(readTestCase);
        add(BOOST_CLASS_TEST_CASE(&SPIviaPCIeTest::testGetSetWaitingTime, spiViaPCIeTest));

    }
};

test_suite*
init_unit_test_suite(int /*argc*/, char * /*argv*/ []) {
    framework::master_test_suite().p_name.value = "SPIviaPCIe test suite";
    return new SPIviaPCIeTestSuite(MAP_FILE_NAME, MODULE_NAME_0);
}

SPIviaPCIeTest::SPIviaPCIeTest(std::string const & mapFileName, std::string const & moduleName) 
  : _dummyBackend(), _device(), _mapFileName(), _readWriteSPIviaPCIe(), _writeSPIviaPCIe()
{


	 _dummyBackend.reset( new DFMC_MD22Dummy(mapFileName, moduleName) );
	//_dummyBackend.reset(new DFMC_MD22Dummy(moduleName));

    // we need a mapped device of BaseDevice. Unfortunately this is still really clumsy to produce/open

    _device.reset(new Device());

    //_dummyBackend->open(mapFileName);
    //_dummyBackend->open();

    MapFileParser fileParser;
    boost::shared_ptr<RegisterInfoMap> registerMapping = fileParser.parse(mapFileName);

    _device->open(_dummyBackend, registerMapping);

    //_dummyBackend->setRegistersForTesting();



    _readWriteSPIviaPCIe.reset(new SPIviaPCIe(_device, moduleName,
            CONTROLER_SPI_WRITE_ADDRESS_STRING,
            CONTROLER_SPI_SYNC_ADDRESS_STRING,
            CONTROLER_SPI_READBACK_ADDRESS_STRING));

    _writeSPIviaPCIe.reset(new SPIviaPCIe(_device, moduleName,
            MOTOR_REGISTER_PREFIX + "2_" + SPI_WRITE_SUFFIX,
            MOTOR_REGISTER_PREFIX + "2_" + SPI_SYNC_SUFFIX));




}

void SPIviaPCIeTest::testRead() {
    // prepare a cover datagram word
    TMC429InputWord coverDatagram;
    coverDatagram.setSMDA(tmc429::SMDA_COMMON);
    coverDatagram.setADDRESS(tmc429::JDX_COVER_DATAGRAM);

    // set the payload content and write it
    coverDatagram.setDATA(0xAAAAAA);
    _readWriteSPIviaPCIe->write(coverDatagram.getDataWord());

    // now try to read back. prepare the coverDatagram word for reading
    coverDatagram.setRW(tmc429::RW_READ);
    coverDatagram.setDATA(0);

    unsigned int readbackInt;
    BOOST_REQUIRE_NO_THROW(readbackInt = _readWriteSPIviaPCIe->read(coverDatagram.getDataWord()));
    // For optimised code the require is not enough. The code might continue if an exception occured 
    // (at least the compiler says so an gives a warning)
    // Just reexecute and let the exception through
    readbackInt = _readWriteSPIviaPCIe->read(coverDatagram.getDataWord());

    TMC429OutputWord readbackWord;
    readbackWord.setDataWord(readbackInt);
    BOOST_CHECK(readbackWord.getDATA() == 0xAAAAAA);

    // test the error cases. More than 2 timeouts cause an error
    _dummyBackend->causeSpiTimeouts(true,3);
    try {
        _readWriteSPIviaPCIe->read(coverDatagram.getDataWord());
        BOOST_ERROR("SPIviaPCIe::read did not throw as expected.");
    } catch (MotorDriverException & e) {
        if (e.getID() != MotorDriverException::SPI_TIMEOUT) {
            BOOST_ERROR(std::string("SPIviaPCIe::read did not throw the right exception ID. Error message: ")
                    + e.what());
        }
    }
    // up to two timeouts are caught by the backend via retry
    _dummyBackend->causeSpiTimeouts(true,2);
    BOOST_CHECK_NO_THROW(_readWriteSPIviaPCIe->read(coverDatagram.getDataWord()));
    
    _dummyBackend->causeSpiTimeouts(false);

    _dummyBackend->causeSpiErrors(true);
    try {
        _readWriteSPIviaPCIe->read(coverDatagram.getDataWord());
        BOOST_ERROR("SPIviaPCIe::read did not throw as expected.");
    } catch (MotorDriverException & e) {
        if (e.getID() != MotorDriverException::SPI_ERROR) {
            BOOST_ERROR(std::string("SPIviaPCIe::read did not throw the right exception ID. Error message: ")
                    + e.what());
        }
    }
    _dummyBackend->causeSpiErrors(false);
}

void SPIviaPCIeTest::testWrite() {
    // This test is hard coded against the dummy implementation of the TCM260 driver chip
    for (uint32_t motorID = 1; motorID < dfmc_md22::N_MOTORS_MAX; ++motorID) {
        // the readDriverSpiRegister is a debug function of the dummy which bypasses the SPI interface
        unsigned int registerContent = _dummyBackend->readDriverSpiRegister(motorID,
                ChopperControlData().getAddress());

        ChopperControlData chopperControlData(registerContent + 5);
        BOOST_CHECK_NO_THROW(_writeSPIviaPCIe->write(chopperControlData.getDataWord()));

        BOOST_CHECK(_dummyBackend->readDriverSpiRegister(motorID, ChopperControlData().getAddress()) ==
                chopperControlData.getPayloadData());

        // test the error cases
        _dummyBackend->causeSpiTimeouts(true);
        chopperControlData.setPayloadData(chopperControlData.getPayloadData() + 1);
        try {
            _writeSPIviaPCIe->write(chopperControlData.getDataWord());
            BOOST_ERROR("SPIviaPCIe::write did not throw as expected.");
        } catch (MotorDriverException & e) {
            if (e.getID() != MotorDriverException::SPI_TIMEOUT) {
                BOOST_ERROR(std::string("SPIviaPCIe::write did not throw the right exception ID. Error message: ")
                        + e.what());
            }
        }
        BOOST_CHECK(_dummyBackend->readDriverSpiRegister(motorID, ChopperControlData().getAddress()) ==
                chopperControlData.getPayloadData() - 1);
        _dummyBackend->causeSpiTimeouts(false);

        _dummyBackend->causeSpiErrors(true);
        try {
            _writeSPIviaPCIe->write(chopperControlData.getDataWord());
            BOOST_ERROR("SPIviaPCIe::write did not throw as expected.");
        } catch (MotorDriverException & e) {
            if (e.getID() != MotorDriverException::SPI_ERROR) {
                BOOST_ERROR(std::string("SPIviaPCIe::write did not throw the right exception ID. Error message: ")
                        + e.what());
            }
        }
        BOOST_CHECK(_dummyBackend->readDriverSpiRegister(motorID, ChopperControlData().getAddress()) ==
                chopperControlData.getPayloadData() - 1);
        _dummyBackend->causeSpiErrors(false);
    }

}

void SPIviaPCIeTest::testGetSetWaitingTime() {
    BOOST_CHECK(_writeSPIviaPCIe->getSpiWaitingTime() == SPIviaPCIe::SPI_DEFAULT_WAITING_TIME);

    _writeSPIviaPCIe->setSpiWaitingTime(2 * SPIviaPCIe::SPI_DEFAULT_WAITING_TIME);
    BOOST_CHECK(_writeSPIviaPCIe->getSpiWaitingTime() == 2 * SPIviaPCIe::SPI_DEFAULT_WAITING_TIME);
}
