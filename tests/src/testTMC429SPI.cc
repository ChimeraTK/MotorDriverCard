#include <sstream>

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "DFMC_MD22Constants.h"
#include <MtcaMappedDevice/devMap.h>
//#include <MtcaMappedDevice/libmap.h>
#include "TMC429SPI.h"
#include "DFMC_MD22Dummy.h"
#include "DFMC_MD22Constants.h"
#include "TMC429Words.h"
#include "TMC429Constants.h"
#include "MotorDriverException.h" 
using namespace mtca4u;
using namespace mtca4u::dfmc_md22;
using namespace mtca4u::tmc429;
#include "testWordFromSpiAddress.h"

#define MAP_FILE_NAME "DFMC_MD22_test.map"
#define MODULE_NAME ""

class TMC429SPITest{
 public:
  
  TMC429SPITest(std::string const & mapFileName, std::string const & moduleName);
  //  static void testConstructors();

  void testRead();
  void testWrite();

 private:
  boost::shared_ptr<DFMC_MD22Dummy> _dummyDevice;
  boost::shared_ptr< devMap<devBase> > _mappedDevice;
  std::string _mapFileName;

  boost::shared_ptr<TMC429SPI> _tmc429Spi;
};

class TMC429SPITestSuite : public test_suite {
public:
  TMC429SPITestSuite(std::string const & mapFileName, std::string const & moduleName): test_suite("TMC429SPI test suite") {
    // create an instance of the test class
    boost::shared_ptr<TMC429SPITest> tmc429SpiTest( new TMC429SPITest(mapFileName, moduleName) );

    // in case of dependencies store the test cases before adding them and declare the dependency
    test_case* readTestCase = BOOST_CLASS_TEST_CASE( &TMC429SPITest::testRead, tmc429SpiTest );
    test_case* writeTestCase = BOOST_CLASS_TEST_CASE( &TMC429SPITest::testWrite, tmc429SpiTest );

    writeTestCase->depends_on(readTestCase);

    add( readTestCase );
    add( writeTestCase );
  }
};

test_suite*
init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
  framework::master_test_suite().p_name.value = "TMC429SPI test suite";
  return new TMC429SPITestSuite( MAP_FILE_NAME, MODULE_NAME );
}

TMC429SPITest::TMC429SPITest(std::string const & mapFileName, std::string const & moduleName){
  _dummyDevice.reset( new DFMC_MD22Dummy );

  // we need a mapped device of devBase. Unfortunately this is still really clumsy to produce/open
  _mappedDevice.reset(new devMap<devBase>);
  _dummyDevice->openDev( mapFileName );
 
  mapFileParser fileParser;
  boost::shared_ptr<mapFile> registerMapping = fileParser.parse(mapFileName);

  _mappedDevice->openDev( _dummyDevice, registerMapping );
  
  _dummyDevice->setRegistersForTesting();

   _tmc429Spi.reset( new TMC429SPI( _mappedDevice, moduleName, CONTROLER_SPI_WRITE_ADDRESS_STRING,
				    CONTROLER_SPI_SYNC_ADDRESS_STRING,
				    CONTROLER_SPI_READBACK_ADDRESS_STRING ) );
}

void TMC429SPITest::testRead(){
  // use the cover datagram for testing
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_COVER_DATAGRAM);
  TMC429OutputWord readResponse =  _tmc429Spi->read(  SMDA_COMMON, JDX_COVER_DATAGRAM);
  BOOST_CHECK( readResponse.getDATA() == expectedContent );
}

void TMC429SPITest::testWrite(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_COVER_DATAGRAM);
  ++expectedContent;
  _tmc429Spi->write( SMDA_COMMON, JDX_COVER_DATAGRAM, expectedContent);

  BOOST_CHECK( _tmc429Spi->read(SMDA_COMMON, JDX_COVER_DATAGRAM).getDATA() == expectedContent );
}
