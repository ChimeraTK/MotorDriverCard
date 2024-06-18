#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TMC429SPITest

#include <boost/shared_ptr.hpp>
#include <boost/test/included/unit_test.hpp>

#include <sstream>
using namespace boost::unit_test_framework;

#include "DFMC_MD22Constants.h"
#include "DFMC_MD22Dummy.h"
#include "impl/TMC429SPI.h"
#include "TMC429Constants.h"
#include "TMC429Words.h"

#include <ChimeraTK/Device.h>
#include <ChimeraTK/MapFileParser.h>

using namespace mtca4u;
using namespace mtca4u::dfmc_md22;
using namespace mtca4u::tmc429;

#include "testConfigConstants.h"
#include "testWordFromSpiAddress.h"

class TMC429SPITestFixture {
 public:
  TMC429SPITestFixture();

 protected:
  boost::shared_ptr<DFMC_MD22Dummy> _dummyDevice;
  boost::shared_ptr<ChimeraTK::Device> _mappedDevice;
  boost::shared_ptr<TMC429SPI> _tmc429Spi;
};

TMC429SPITestFixture::TMC429SPITestFixture()
: _dummyDevice{}, _mappedDevice{boost::make_shared<ChimeraTK::Device>()}, _tmc429Spi{} {
  ChimeraTK::setDMapFilePath("./dummies.dmap");
  _dummyDevice =
      boost::dynamic_pointer_cast<DFMC_MD22Dummy>(ChimeraTK::BackendFactory::getInstance().createBackend(DFMC_ALIAS));

  // we need a mapped device of BaseDevice. Unfortunately this is still really
  // clumsy to produce/open
  _mappedDevice->open(DFMC_ALIAS);
  _dummyDevice->setRegistersForTesting();

  _tmc429Spi = boost::make_shared<TMC429SPI>(_mappedDevice, MODULE_NAME_0, CONTROLER_SPI_WRITE_ADDRESS_STRING,
      CONTROLER_SPI_SYNC_ADDRESS_STRING, CONTROLER_SPI_READBACK_ADDRESS_STRING);
}

BOOST_FIXTURE_TEST_SUITE(TMC429SPITest, TMC429SPITestFixture)

BOOST_AUTO_TEST_CASE(testRead) {
  // use the cover datagram for testing
  unsigned int expectedContent = testWordFromSpiAddress(SMDA_COMMON, JDX_COVER_DATAGRAM);
  TMC429OutputWord readResponse = _tmc429Spi->read(SMDA_COMMON, JDX_COVER_DATAGRAM);
  BOOST_CHECK(readResponse.getDATA() == expectedContent);
}

BOOST_AUTO_TEST_CASE(testWrite) {
  unsigned int expectedContent = testWordFromSpiAddress(SMDA_COMMON, JDX_COVER_DATAGRAM);
  ++expectedContent;
  _tmc429Spi->write(SMDA_COMMON, JDX_COVER_DATAGRAM, expectedContent);

  BOOST_CHECK(_tmc429Spi->read(SMDA_COMMON, JDX_COVER_DATAGRAM).getDATA() == expectedContent);
}

BOOST_AUTO_TEST_SUITE_END()
