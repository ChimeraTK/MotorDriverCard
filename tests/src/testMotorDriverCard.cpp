#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "DFMC_MD22Dummy.h"
#include "MotorDriverCardImpl.h"
#include <MtcaMappedDevice/devMap.h>
#include <MtcaMappedDevice/libmap.h>

#include "DFMC_MD22Constants.h"
using namespace mtca4u::dfmc_md22;
#include "testWordFromSpiAddress.h"
using namespace mtca4u::tmc429;

#define MAP_FILE_NAME "DFMC_MD22_test.map"
#define BROKEN_MAP_FILE_NAME "DFMC_MD22_broken.map"

#define DECLARE_GET_SET_TEST( NAME )\
  void testGet ## NAME ();\
  void testSet ## NAME ()

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
  void testConfiguration(MotorDriverCardConfig const & motorDriverCardConfig);
  void testGetControlerChipVersion();
  void testGetReferenceSwitchData();
  DECLARE_GET_SET_TEST( DatagramLowWord );
  DECLARE_GET_SET_TEST( DatagramHighWord );
  DECLARE_GET_SET_TEST( CoverPositionAndLength );
  DECLARE_GET_SET_TEST( CoverDatagram );
  DECLARE_GET_SET_TEST( StepperMotorGlobalParameters );
  DECLARE_GET_SET_TEST( InterfaceConfiguration );
  DECLARE_GET_SET_TEST( PositionCompareWord );
  DECLARE_GET_SET_TEST( PositionCompareInterruptData );
  void testPowerDown();
  void testGetMotorControler();

private:
  boost::shared_ptr<MotorDriverCardImpl> _motorDriverCard;
  boost::shared_ptr<DFMC_MD22Dummy> _dummyDevice;
  std::string _mapFileName;
  
  unsigned int asciiToInt( std::string text );
};

class  MotorDriverCardTestSuite : public test_suite{
 public:
  MotorDriverCardTestSuite() : test_suite(" MotorDriverCard test suite"){
    boost::shared_ptr<MotorDriverCardTest> motorDriverCardTest( new MotorDriverCardTest(MAP_FILE_NAME) );
    
    test_case* constructorTestCase = BOOST_CLASS_TEST_CASE( &MotorDriverCardTest::testConstructor, motorDriverCardTest );
    test_case* getControlerVersionTestCase =  BOOST_CLASS_TEST_CASE( &MotorDriverCardTest::testGetControlerChipVersion, motorDriverCardTest );

    getControlerVersionTestCase->depends_on( constructorTestCase );

    add( constructorTestCase );
    add( getControlerVersionTestCase );
    add( BOOST_CLASS_TEST_CASE( &MotorDriverCardTest::testGetReferenceSwitchData, motorDriverCardTest ) );
    ADD_GET_SET_TEST( DatagramLowWord );
    ADD_GET_SET_TEST( DatagramHighWord );
    ADD_GET_SET_TEST( CoverPositionAndLength );
    ADD_GET_SET_TEST( CoverDatagram );
    ADD_GET_SET_TEST( StepperMotorGlobalParameters );
    ADD_GET_SET_TEST( InterfaceConfiguration );
    ADD_GET_SET_TEST( PositionCompareWord );
    ADD_GET_SET_TEST( PositionCompareInterruptData );
    add( BOOST_CLASS_TEST_CASE( &MotorDriverCardTest::testPowerDown, motorDriverCardTest ) );
    add( BOOST_CLASS_TEST_CASE( &MotorDriverCardTest::testGetMotorControler, motorDriverCardTest ) );
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
  //boost::shared_ptr<devBase> dummyDevice( new DFMC_MD22Dummy );
  _dummyDevice = boost::shared_ptr<DFMC_MD22Dummy>( new DFMC_MD22Dummy );
  _dummyDevice->openDev( _mapFileName );
 
  mapFileParser fileParser;
  boost::shared_ptr<mapFile> registerMapping = fileParser.parse(_mapFileName);

  boost::shared_ptr< devMap<devBase> > mappedDevice(new devMap<devBase>);
  MotorDriverCardConfig motorDriverCardConfig;
  motorDriverCardConfig.coverDatagram = asciiToInt("DTGR");
  motorDriverCardConfig.coverPositionAndLength.setDATA(asciiToInt( "POSL") );
  motorDriverCardConfig.datagramHighWord = asciiToInt("DGRH");
  motorDriverCardConfig.datagramLowWord = asciiToInt("DGRL");
  motorDriverCardConfig.interfaceConfiguration.setDATA( asciiToInt("IFCF") );
  motorDriverCardConfig.positionCompareInterruptData.setDATA( asciiToInt("PCID") );
  motorDriverCardConfig.positionCompareWord = asciiToInt("POSC");
  motorDriverCardConfig.stepperMotorGlobalParameters.setDATA( asciiToInt("SMGP") );

  // has to throw because the device is not open
  BOOST_CHECK_THROW( _motorDriverCard = boost::shared_ptr<MotorDriverCardImpl>(
		       new MotorDriverCardImpl( mappedDevice, motorDriverCardConfig ) ),
		     //FIXME: create a DeviceException. Has to work for real and dummy devices
		     exBase );

  boost::shared_ptr<mapFile> brokenRegisterMapping = fileParser.parse(BROKEN_MAP_FILE_NAME);
  // try opening with bad mapping, also has to throw
  mappedDevice->openDev( _dummyDevice, brokenRegisterMapping );
  BOOST_CHECK_THROW( _motorDriverCard = boost::shared_ptr<MotorDriverCardImpl>(
		       new MotorDriverCardImpl( mappedDevice, motorDriverCardConfig ) ),
		     exLibMap );
  
  mappedDevice->closeDev();

  _dummyDevice->openDev( _mapFileName );
  mappedDevice->openDev( _dummyDevice, registerMapping );
  
  BOOST_CHECK_NO_THROW(  _motorDriverCard = boost::shared_ptr<MotorDriverCardImpl>(new MotorDriverCardImpl( mappedDevice, motorDriverCardConfig )) );

  // test that the configuration with the motorDriverCardConfig actually worked
  testConfiguration(motorDriverCardConfig);
  
  // only after initialising the dummy device with the motorDriverCardConfig in the constructor of the
  // MotorDriverCardImpl we can change the registers for testing
  _dummyDevice->setControlerSpiRegistersForTesting();  
  
}

void MotorDriverCardTest::testConfiguration(MotorDriverCardConfig const & motorDriverCardConfig){
  BOOST_CHECK( _motorDriverCard->getCoverDatagram() == motorDriverCardConfig.coverDatagram );
  BOOST_CHECK( _motorDriverCard->getCoverPositionAndLength() == motorDriverCardConfig.coverPositionAndLength );
BOOST_CHECK( _motorDriverCard->getDatagramHighWord() == motorDriverCardConfig.datagramHighWord );
BOOST_CHECK( _motorDriverCard->getDatagramLowWord() == motorDriverCardConfig.datagramLowWord );
BOOST_CHECK( _motorDriverCard->getInterfaceConfiguration() == motorDriverCardConfig.interfaceConfiguration );
BOOST_CHECK( _motorDriverCard->getPositionCompareInterruptData() == motorDriverCardConfig.positionCompareInterruptData );
BOOST_CHECK( _motorDriverCard->getPositionCompareWord() == motorDriverCardConfig.positionCompareWord );
BOOST_CHECK( _motorDriverCard->getStepperMotorGlobalParameters() == motorDriverCardConfig.stepperMotorGlobalParameters );
}

void MotorDriverCardTest::testGetControlerChipVersion(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_CHIP_VERSION);
  BOOST_CHECK( _motorDriverCard->getControlerChipVersion() == expectedContent );
}

void MotorDriverCardTest::testGetReferenceSwitchData(){
  // seems like a self consistency test, but ReferenceSwitchData has been testes in testTMC429Words, and the data content should be the test word
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_REFERENCE_SWITCH);  
  BOOST_CHECK( _motorDriverCard->getReferenceSwitchData() == ReferenceSwitchData(expectedContent) );
}

void MotorDriverCardTest::testGetDatagramLowWord(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_DATAGRAM_LOW_WORD);
  BOOST_CHECK( _motorDriverCard->getDatagramLowWord() == expectedContent );
}

// just for the fun of it I use AAAA, 5555 and FFFF alternating as test patterns
void MotorDriverCardTest::testSetDatagramLowWord(){
  _motorDriverCard->setDatagramLowWord( 0xAAAAAAAA );
  BOOST_CHECK( _motorDriverCard->getDatagramLowWord() == 0xAAAAAA );
}

void MotorDriverCardTest::testGetDatagramHighWord(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_DATAGRAM_HIGH_WORD);
  BOOST_CHECK( _motorDriverCard->getDatagramHighWord() == expectedContent );
}

void MotorDriverCardTest::testSetDatagramHighWord(){
  _motorDriverCard->setDatagramHighWord( 0x55555555 );
  BOOST_CHECK( _motorDriverCard->getDatagramHighWord() == 0x555555 );
}

void MotorDriverCardTest::testGetCoverPositionAndLength(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_COVER_POSITION_AND_LENGTH);
  BOOST_CHECK( _motorDriverCard->getCoverPositionAndLength() == CoverPositionAndLength(expectedContent) );
}

void MotorDriverCardTest::testSetCoverPositionAndLength(){
  _motorDriverCard->setCoverPositionAndLength( 0xFFFFFFFF );
  BOOST_CHECK( _motorDriverCard->getCoverPositionAndLength() == CoverPositionAndLength(0xFFFFFF) );
}

void MotorDriverCardTest::testGetCoverDatagram(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_COVER_DATAGRAM);
  BOOST_CHECK( _motorDriverCard->getCoverDatagram() == expectedContent );
}

void MotorDriverCardTest::testSetCoverDatagram(){
  _motorDriverCard->setCoverDatagram( 0xAAAAAAAA );
  BOOST_CHECK( _motorDriverCard->getCoverDatagram() == 0xAAAAAA );
}

void MotorDriverCardTest::testGetStepperMotorGlobalParameters(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_STEPPER_MOTOR_GLOBAL_PARAMETERS);
  BOOST_CHECK( _motorDriverCard->getStepperMotorGlobalParameters() == StepperMotorGlobalParameters(expectedContent) );
}

void MotorDriverCardTest::testSetStepperMotorGlobalParameters(){
  _motorDriverCard->setStepperMotorGlobalParameters( 0x55555555 );
  BOOST_CHECK( _motorDriverCard->getStepperMotorGlobalParameters() == StepperMotorGlobalParameters(0x555555) );
}

void MotorDriverCardTest::testGetInterfaceConfiguration(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_INTERFACE_CONFIGURATION);
  BOOST_CHECK( _motorDriverCard->getInterfaceConfiguration() == InterfaceConfiguration(expectedContent) );
}

void MotorDriverCardTest::testSetInterfaceConfiguration(){
  _motorDriverCard->setInterfaceConfiguration( 0x55555555 );
  BOOST_CHECK( _motorDriverCard->getInterfaceConfiguration() == InterfaceConfiguration(0x555555) );
}

void MotorDriverCardTest::testGetPositionCompareWord(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_POSITION_COMPARE);
  BOOST_CHECK( _motorDriverCard->getPositionCompareWord() == expectedContent );
}

void MotorDriverCardTest::testSetPositionCompareWord(){
  _motorDriverCard->setPositionCompareWord( 0xFFFFFFFF );
  BOOST_CHECK( _motorDriverCard->getPositionCompareWord() == 0xFFFFFF );
}

void MotorDriverCardTest::testGetPositionCompareInterruptData(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_POSITION_COMPARE_INTERRUPT);
  BOOST_CHECK( _motorDriverCard->getPositionCompareInterruptData() == PositionCompareInterruptData(expectedContent) );
}

void MotorDriverCardTest::testSetPositionCompareInterruptData(){
  _motorDriverCard->setPositionCompareInterruptData( 0xAAAAAAAA );
  BOOST_CHECK( _motorDriverCard->getPositionCompareInterruptData() == PositionCompareInterruptData(0xAAAAAA) );
}

void MotorDriverCardTest::testPowerDown(){
  _motorDriverCard->powerDown();
  BOOST_CHECK( _dummyDevice->isPowerUp() == false );

  _dummyDevice->powerUp();
  BOOST_CHECK( _dummyDevice->isPowerUp() == true );
}

void MotorDriverCardTest::testGetMotorControler(){
  for (unsigned int i = 0; i < N_MOTORS_MAX ; ++i){
    BOOST_CHECK( _motorDriverCard->getMotorControler(i).getID() == i );
  }
  BOOST_CHECK_THROW( _motorDriverCard->getMotorControler( N_MOTORS_MAX ), 
		     MotorDriverException );
}

unsigned int MotorDriverCardTest::asciiToInt( std::string text ){
  unsigned int returnValue(0);
  int position = 0;
  std::string::reverse_iterator characterIter = text.rbegin();
  for ( /* emtpy */ ; (position < 4) && (characterIter!=  text.rend());  ++position, ++characterIter){
    unsigned int charAsInt = static_cast<unsigned int>(*characterIter);
    returnValue |= charAsInt << 4*position;
  }

  return returnValue;
}

