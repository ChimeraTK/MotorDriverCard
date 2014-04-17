#include <sstream>

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "DFMC_MD22Constants.h"
#include <MtcaMappedDevice/devMap.h>
//#include <MtcaMappedDevice/libmap.h>
#include "SPIviaPCIe.h"
#include "DFMC_MD22Dummy.h"
#include "DFMC_MD22Constants.h"
using namespace mtca4u;
using namespace mtca4u::dfmc_md22;

#define MAP_FILE_NAME "DFMC_MD22_test.map"

class SPIviaPCIeTest{
 public:
  
  SPIviaPCIeTest(std::string const & mapFileName);
  //  static void testConstructors();

  void testRead();
  void testWrite();

 private:
  boost::shared_ptr<DFMC_MD22Dummy> _dummyDevice;
  boost::shared_ptr< devMap<devBase> > _mappedDevice;
  std::string _mapFileName;

  boost::shared_ptr<SPIviaPCIe> _readWriteSPIviaPCIe; // use controler which has read/write
  boost::shared_ptr<SPIviaPCIe> _writeSPIviaPCIe;// use a motor address which has debug readback in the dummy
};

class SPIviaPCIeTestSuite : public test_suite {
public:
  SPIviaPCIeTestSuite(std::string const & mapFileName): test_suite("SPIviaPCIe test suite") {
    // create an instance of the test class
    boost::shared_ptr<SPIviaPCIeTest> spiViaPCIeTest( new SPIviaPCIeTest(mapFileName) );

    // add the tests
    //   add( BOOST_CLASS_TEST_CASE( &SPIviaPCIeTest::testConstructors, spiViaPCIeTest ) );

    // in case of dependencies store the test cases before adding them and declare the dependency
    test_case* writeTestCase = BOOST_CLASS_TEST_CASE( &SPIviaPCIeTest::testWrite, spiViaPCIeTest );
    test_case* readTestCase = BOOST_CLASS_TEST_CASE( &SPIviaPCIeTest::testRead, spiViaPCIeTest );

    // exceptionally the read depends on the write (usually it is the other way round)
    readTestCase->depends_on(writeTestCase);

    add( readTestCase );
    add( writeTestCase );
  }
};

// Although the compiler complains that argc and argv are not used they 
// cannot be commented out. This somehow changes the signature and linking fails.
test_suite*
init_unit_test_suite( int /*argc*/, char** /* use 'char * argv[]' if you need it */ )
{
  framework::master_test_suite().p_name.value = "SPIviaPCIe test suite";
  return new SPIviaPCIeTestSuite( MAP_FILE_NAME );
}

SPIviaPCIeTest::SPIviaPCIeTest(std::string const & mapFileName)
  :  _dummyDevice( new DFMC_MD22Dummy ), _mappedDevice(new devMap<devBase>){
  /// we need a mapped device of devBase. Unfortunately this is still really clumsy to produce/open
  _dummyDevice->openDev( mapFileName );
 
  mapFileParser fileParser;
  boost::shared_ptr<mapFile> registerMapping = fileParser.parse(mapFileName);

  _mappedDevice->openDev( _dummyDevice, registerMapping );
  
  _dummyDevice->setRegistersForTesting();

  _readWriteSPIviaPCIe.reset( new SPIviaPCIe( _mappedDevice, CONTROLER_SPI_WRITE_ADDRESS_STRING,
					      CONTROLER_SPI_SYNC_ADDRESS_STRING,
					      CONTROLER_SPI_READBACK_ADDRESS_STRING ) );

  _writeSPIviaPCIe.reset( new SPIviaPCIe( _mappedDevice, CONTROLER_SPI_WRITE_ADDRESS_STRING,
					  CONTROLER_SPI_SYNC_ADDRESS_STRING ) );
}

void SPIviaPCIeTest::testRead(){
  BOOST_FAIL( "Not implemented yet" );
}

void SPIviaPCIeTest::testWrite(){
  BOOST_FAIL( "Not implemented yet" );
}
