#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE MotorControlerTest
#include <boost/test/unit_test.hpp>

using namespace boost::unit_test_framework;

#include "ChimeraTK/BackendFactory.h"
#include "DFMC_MD22Constants.h"
#include "DFMC_MD22Dummy.h"
#include "impl/SPIviaPCIe.h"
#include "testConfigConstants.h"
#include "TMC260Words.h"
#include "TMC429Words.h"

#include <ChimeraTK/Device.h>
#include <ChimeraTK/MapFileParser.h>
using namespace ChimeraTK::MotorDriver;

class SPIviaPCIeTestFixture {
 public:
  SPIviaPCIeTestFixture();

 protected:
  boost::shared_ptr<ChimeraTK::MotorDriver::DFMC_MD22Dummy> _dummyBackend;
  boost::shared_ptr<ChimeraTK::Device> _device;

  boost::shared_ptr<ChimeraTK::MotorDriver::SPIviaPCIe> _readWriteSPIviaPCIe; // use controler which has read/write
  boost::shared_ptr<ChimeraTK::MotorDriver::SPIviaPCIe> _writeSPIviaPCIe;     // use a motor address which has debug
                                                                              // readback in the dummy
};

SPIviaPCIeTestFixture::SPIviaPCIeTestFixture() {
  const auto& moduleName = MODULE_NAME_0;
  ChimeraTK::setDMapFilePath("./dummies.dmap");

  _dummyBackend = boost::dynamic_pointer_cast<ChimeraTK::MotorDriver::DFMC_MD22Dummy>(
      ChimeraTK::BackendFactory::getInstance().createBackend(DFMC_ALIAS));

  _device = boost::make_shared<ChimeraTK::Device>();
  _device->open(DFMC_ALIAS);

  _readWriteSPIviaPCIe = boost::make_shared<ChimeraTK::MotorDriver::SPIviaPCIe>(_device, moduleName,
      ChimeraTK::MotorDriver::dfmc_md22::CONTROLER_SPI_WRITE_ADDRESS_STRING,
      ChimeraTK::MotorDriver::dfmc_md22::CONTROLER_SPI_SYNC_ADDRESS_STRING,
      ChimeraTK::MotorDriver::dfmc_md22::CONTROLER_SPI_READBACK_ADDRESS_STRING);

  _writeSPIviaPCIe = boost::make_shared<ChimeraTK::MotorDriver::SPIviaPCIe>(_device, moduleName,
      ChimeraTK::MotorDriver::dfmc_md22::MOTOR_REGISTER_PREFIX + "2_" +
          ChimeraTK::MotorDriver::dfmc_md22::SPI_WRITE_SUFFIX,
      ChimeraTK::MotorDriver::dfmc_md22::MOTOR_REGISTER_PREFIX + "2_" +
          ChimeraTK::MotorDriver::dfmc_md22::SPI_SYNC_SUFFIX);
}

BOOST_FIXTURE_TEST_CASE(TestRead, SPIviaPCIeTestFixture) {
  // prepare a cover datagram word
  ChimeraTK::MotorDriver::TMC429InputWord coverDatagram;
  coverDatagram.setSMDA(ChimeraTK::MotorDriver::tmc429::SMDA_COMMON);
  coverDatagram.setADDRESS(ChimeraTK::MotorDriver::tmc429::JDX_COVER_DATAGRAM);

  // set the payload content and write it
  coverDatagram.setDATA(0xAAAAAA);
  _readWriteSPIviaPCIe->write(int32_t(coverDatagram.getDataWord()));

  // now try to read back. prepare the coverDatagram word for reading
  coverDatagram.setRW(ChimeraTK::MotorDriver::tmc429::RW_READ);
  coverDatagram.setDATA(0);

  unsigned int readbackInt;
  BOOST_REQUIRE_NO_THROW(readbackInt = _readWriteSPIviaPCIe->read(coverDatagram.getDataWord()));
  // For optimised code the require is not enough. The code might continue if an
  // exception occured (at least the compiler says so an gives a warning) Just
  // reexecute and let the exception through
  readbackInt = _readWriteSPIviaPCIe->read(int32_t(coverDatagram.getDataWord()));

  ChimeraTK::MotorDriver::TMC429OutputWord readbackWord;
  readbackWord.setDataWord(readbackInt);
  BOOST_CHECK(readbackWord.getDATA() == 0xAAAAAA);

  // test the error cases. More than 2 timeouts cause an error
  _dummyBackend->causeSpiTimeouts(true, 3);
  BOOST_CHECK_THROW(_readWriteSPIviaPCIe->read(coverDatagram.getDataWord()), ChimeraTK::runtime_error);
  // up to two timeouts are caught by the backend via retry
  _dummyBackend->causeSpiTimeouts(true, 2);
  BOOST_CHECK_NO_THROW(_readWriteSPIviaPCIe->read(coverDatagram.getDataWord()));

  _dummyBackend->causeSpiTimeouts(false);

  _dummyBackend->causeSpiErrors(true);

  BOOST_CHECK_THROW(_readWriteSPIviaPCIe->read(coverDatagram.getDataWord()), ChimeraTK::runtime_error);
  _dummyBackend->causeSpiErrors(false);
}

BOOST_FIXTURE_TEST_CASE(TestWrite, SPIviaPCIeTestFixture) {
  // This test is hard coded against the dummy implementation of the TCM260
  // driver chip
  for(uint32_t motorID = 1; motorID < ChimeraTK::MotorDriver::dfmc_md22::N_MOTORS_MAX; ++motorID) {
    // the readDriverSpiRegister is a debug function of the dummy which bypasses
    // the SPI interface
    unsigned int registerContent =
        _dummyBackend->readDriverSpiRegister(motorID, ChimeraTK::MotorDriver::ChopperControlData().getAddress());

    ChimeraTK::MotorDriver::ChopperControlData chopperControlData(registerContent + 5);
    BOOST_CHECK_NO_THROW(_writeSPIviaPCIe->write(chopperControlData.getDataWord()));

    BOOST_CHECK(_dummyBackend->readDriverSpiRegister(motorID,
                    ChimeraTK::MotorDriver::ChopperControlData().getAddress()) == chopperControlData.getPayloadData());

    // test the error cases
    _dummyBackend->causeSpiTimeouts(true);
    chopperControlData.setPayloadData(chopperControlData.getPayloadData() + 1);
    BOOST_CHECK_THROW(_writeSPIviaPCIe->write(chopperControlData.getDataWord()), ChimeraTK::runtime_error);
    BOOST_CHECK(
        _dummyBackend->readDriverSpiRegister(motorID, ChimeraTK::MotorDriver::ChopperControlData().getAddress()) ==
        chopperControlData.getPayloadData() - 1);
    _dummyBackend->causeSpiTimeouts(false);

    _dummyBackend->causeSpiErrors(true);
    BOOST_CHECK_THROW(_writeSPIviaPCIe->write(chopperControlData.getDataWord()), ChimeraTK::runtime_error);
    BOOST_CHECK(
        _dummyBackend->readDriverSpiRegister(motorID, ChimeraTK::MotorDriver::ChopperControlData().getAddress()) ==
        chopperControlData.getPayloadData() - 1);
    _dummyBackend->causeSpiErrors(false);
  }
}

BOOST_FIXTURE_TEST_CASE(TestGetSetWaitingTime, SPIviaPCIeTestFixture) {
  BOOST_CHECK(_writeSPIviaPCIe->getSpiWaitingTime() == ChimeraTK::MotorDriver::SPIviaPCIe::SPI_DEFAULT_WAITING_TIME);

  _writeSPIviaPCIe->setSpiWaitingTime(2 * ChimeraTK::MotorDriver::SPIviaPCIe::SPI_DEFAULT_WAITING_TIME);
  BOOST_CHECK(
      _writeSPIviaPCIe->getSpiWaitingTime() == 2 * ChimeraTK::MotorDriver::SPIviaPCIe::SPI_DEFAULT_WAITING_TIME);
}
