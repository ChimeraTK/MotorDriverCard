#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "DFMC_MD22Dummy.h"
#include "MotorDriverCardImpl.h"
#include <MtcaMappedDevice/devMap.h>
#include <MtcaMappedDevice/libmap.h>

#include "DFMC_MD22Constants.h"
using namespace mtca4u::tmc429;

#define MAP_FILE_NAME "DFMC_MD22_test.map"
#define BROKEN_MAP_FILE_NAME "DFMC_MD22_broken.map"

#define DECLARE_GET_SET_TEST( NAME )\
  void testGet ## NAME ();\
  void testSet ## NAME ();

#define ADD_GET_SET_TEST( NAME )\
  test_case* set ## NAME ## TestCase =  BOOST_CLASS_TEST_CASE( &MotorDriverCardTest::testSet ## NAME , motorDriverCardTest );\
  test_case* get ## NAME ## TestCase =  BOOST_CLASS_TEST_CASE( &MotorDriverCardTest::testGet ## NAME , motorDriverCardTest );\
  set ## NAME ## TestCase->depends_on( get ## NAME ## TestCase );\
  add( get ## NAME ## TestCase );\
  add( set ## NAME ## TestCase )

using namespace mtca4u;

class MotorDriverCardTest{
public:
  MotorDriverCardTest(std::string const & mapFileName);

  // the constructor might throw, better test it.
  void testConstructor();
  void testGetControlerChipVersion();
  void testGetReferenceSwitchRegister();
  DECLARE_GET_SET_TEST( DatagramLowWord )
  DECLARE_GET_SET_TEST( DatagramHighWord )
  DECLARE_GET_SET_TEST( CoverPositionAndLength )
  DECLARE_GET_SET_TEST( CoverDatagram )

private:
  boost::shared_ptr<MotorDriverCardImpl> motorDriverCard;
  std::string _mapFileName;
  unsigned int testWordFromSpiAddress(unsigned int smda, unsigned int idx_jdx);
};

class  MotorDriverCardTestSuite : public test_suite{
 public:
  MotorDriverCardTestSuite() : test_suite(" MultiVariableWord test suite"){
    boost::shared_ptr<MotorDriverCardTest> motorDriverCardTest( new MotorDriverCardTest(MAP_FILE_NAME) );
    
    test_case* constructorTestCase = BOOST_CLASS_TEST_CASE( &MotorDriverCardTest::testConstructor, motorDriverCardTest );
    test_case* getControlerVersionTestCase =  BOOST_CLASS_TEST_CASE( &MotorDriverCardTest::testGetControlerChipVersion, motorDriverCardTest );

    getControlerVersionTestCase->depends_on( constructorTestCase );

    add( constructorTestCase );
    add( getControlerVersionTestCase );
    add( BOOST_CLASS_TEST_CASE( &MotorDriverCardTest::testGetReferenceSwitchRegister, motorDriverCardTest ) );
    ADD_GET_SET_TEST( DatagramLowWord );
    ADD_GET_SET_TEST( DatagramHighWord );
    ADD_GET_SET_TEST( CoverPositionAndLength );
    ADD_GET_SET_TEST( CoverDatagram );
  }
};

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
   framework::master_test_suite().p_name.value = "MotorDriverCard test suite";

  return new MotorDriverCardTestSuite;
}

MotorDriverCardTest::MotorDriverCardTest(std::string const & mapFileName)
  : _mapFileName(mapFileName){
}

void MotorDriverCardTest::testConstructor(){
  boost::shared_ptr<devBase> dummyDevice( new DFMC_MD22Dummy );
  dummyDevice->openDev( _mapFileName );
 
  mapFileParser fileParser;
  boost::shared_ptr<mapFile> registerMapping = fileParser.parse(_mapFileName);

  boost::shared_ptr< devMap<devBase> > mappedDevice(new devMap<devBase>);
  MotorDriverCardImpl::MotorDriverConfiguration  motorDriverConfiguration;

  // has to throw because the device is not open
  BOOST_CHECK_THROW( motorDriverCard = boost::shared_ptr<MotorDriverCardImpl>(
		       new MotorDriverCardImpl( mappedDevice, motorDriverConfiguration ) ),
		     //FIXME: create a DeviceException. Has to work for real and dummy devices
		     exBase );

  boost::shared_ptr<mapFile> brokenRegisterMapping = fileParser.parse(BROKEN_MAP_FILE_NAME);
  // try opening with bad mapping, also has to throw
  mappedDevice->openDev( dummyDevice, brokenRegisterMapping );
  BOOST_CHECK_THROW( motorDriverCard = boost::shared_ptr<MotorDriverCardImpl>(
		       new MotorDriverCardImpl( mappedDevice, motorDriverConfiguration ) ),
		     exLibMap );
  
  mappedDevice->closeDev();

  dummyDevice->openDev( _mapFileName );
  boost::dynamic_pointer_cast<DFMC_MD22Dummy>(dummyDevice)->setSPIRegistersForTesting();
  mappedDevice->openDev( dummyDevice, registerMapping );
  
  BOOST_CHECK_NO_THROW(  motorDriverCard = boost::shared_ptr<MotorDriverCardImpl>(new MotorDriverCardImpl( mappedDevice, motorDriverConfiguration )) );
  
}

void MotorDriverCardTest::testGetControlerChipVersion(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_CHIP_VERSION);
  BOOST_CHECK( motorDriverCard->getControlerChipVersion() == expectedContent );
}

void MotorDriverCardTest::testGetReferenceSwitchRegister(){
  // seems like a self consistency test, but ReferenceSwitchData has been testes in testTMC429Words, and the data content should be the test word
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_REFERENCE_SWITCH);  
  BOOST_CHECK( motorDriverCard->getReferenceSwitchRegister() == ReferenceSwitchData(expectedContent) );
}

void MotorDriverCardTest::testGetDatagramLowWord(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_DATAGRAM_LOW_WORD);
  BOOST_CHECK( motorDriverCard->getDatagramLowWord() == expectedContent );
}

void MotorDriverCardTest::testSetDatagramLowWord(){
  motorDriverCard->setDatagramLowWord( 0xAAAAAAAA );
  BOOST_CHECK( motorDriverCard->getDatagramLowWord() == 0xAAAAAA );
}

unsigned int MotorDriverCardTest::testWordFromSpiAddress(unsigned int smda,
							 unsigned int idx_jdx){
  unsigned int spiAddress = (smda << 4) | idx_jdx;
  // the test word as defined in the DUMMY (independent implementation)
  return spiAddress*spiAddress+13;
}

void MotorDriverCardTest::testGetDatagramHighWord(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_DATAGRAM_HIGH_WORD);
  BOOST_CHECK( motorDriverCard->getDatagramHighWord() == expectedContent );
}

void MotorDriverCardTest::testSetDatagramHighWord(){
  motorDriverCard->setDatagramHighWord( 0x55555555 );
  BOOST_CHECK( motorDriverCard->getDatagramHighWord() == 0x555555 );
}

void MotorDriverCardTest::testGetCoverPositionAndLength(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_COVER_POSITION_AND_LENGTH);
  BOOST_CHECK( motorDriverCard->getCoverPositionAndLength() == CoverPositionAndLength(expectedContent) );
}

void MotorDriverCardTest::testSetCoverPositionAndLength(){
  motorDriverCard->setCoverPositionAndLength( 0xFFFFFFFF );
  BOOST_CHECK( motorDriverCard->getCoverPositionAndLength() == CoverPositionAndLength(0xFFFFFF) );
}

void MotorDriverCardTest::testGetCoverDatagram(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_COVER_DATAGRAM);
  BOOST_CHECK( motorDriverCard->getCoverDatagram() == expectedContent );
}

void MotorDriverCardTest::testSetCoverDatagram(){
  motorDriverCard->setCoverDatagram( 0xAAAAAAAA );
  BOOST_CHECK( motorDriverCard->getCoverDatagram() == 0xAAAAAA );
}
