#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "DFMC_MD22Dummy.h"
#include "MotorDriverCardImpl.h"
#include "MotorDriverException.h"
#include <MtcaMappedDevice/devMap.h>
#include <MtcaMappedDevice/libmap.h>

#include "DFMC_MD22Constants.h"
using namespace mtca4u::dfmc_md22;
#include "testWordFromSpiAddress.h"
#include "testWordFromPCIeAddress.h"
using namespace mtca4u::tmc429;

#include "MotorDriverCardConfigDefaults.h"

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

namespace mtca4u{

class MotorDriverCardTest{
public:
  MotorDriverCardTest(std::string const & mapFileName);

  // the constructor might throw, better test it.
  void testConstructor();
  void testConfiguration(MotorDriverCardConfig const & motorDriverCardConfig);
  void testGetControlerChipVersion();
  void testGetReferenceSwitchData();
  void testGetStatusWord();
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
  boost::shared_ptr<mapFile> _registerMapping;
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
    add( BOOST_CLASS_TEST_CASE( &MotorDriverCardTest::testGetStatusWord, motorDriverCardTest ) );
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

MotorDriverCardTest::MotorDriverCardTest(std::string const & mapFileName)
  : _mapFileName(mapFileName){
}

void MotorDriverCardTest::testConstructor(){
  //boost::shared_ptr<devBase> dummyDevice( new DFMC_MD22Dummy );
  _dummyDevice.reset( new DFMC_MD22Dummy );
  _dummyDevice->openDev( _mapFileName );
 
  mapFileParser fileParser;
  _registerMapping = fileParser.parse(_mapFileName);

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

  for (unsigned int motorID = 0; motorID <  motorDriverCardConfig.motorControlerConfigurations.size();
       ++motorID){
    MotorControlerConfig motorControlerConfig;

    motorControlerConfig.accelerationThresholdData.setDATA( asciiToInt("THR") + motorID );
    motorControlerConfig.actualPosition = asciiToInt("ACX") + motorID;;
    motorControlerConfig.chopperControlData.setPayloadData( asciiToInt("cp") + motorID );
    motorControlerConfig.coolStepControlData.setPayloadData( asciiToInt("cp") + motorID );
    motorControlerConfig.decoderReadoutMode = asciiToInt("DRM") + motorID;
    motorControlerConfig.dividersAndMicroStepResolutionData.setDATA( asciiToInt("MUSR") + motorID );
    motorControlerConfig.driverConfigData.setPayloadData( asciiToInt("cf") + motorID );
    motorControlerConfig.driverControlData.setPayloadData( asciiToInt("cl") + motorID );
    motorControlerConfig.enabled = true;
    motorControlerConfig.interruptData.setDATA( asciiToInt("INTR") + motorID );
    motorControlerConfig.maximumAccelleration =  asciiToInt("am") + motorID;
    motorControlerConfig.maximumVelocity = asciiToInt("vm") + motorID;
    motorControlerConfig.microStepCount =  asciiToInt("mu") + motorID;
    motorControlerConfig.minimumVelocity = asciiToInt("vn");
    motorControlerConfig.positionTolerance = asciiToInt("dx") + motorID;
    motorControlerConfig.proportionalityFactorData.setDATA( asciiToInt("PROP") + motorID );
    motorControlerConfig.referenceConfigAndRampModeData.setDATA( asciiToInt("REF") + motorID );
    motorControlerConfig.stallGuardControlData.setPayloadData( asciiToInt("sg") + motorID );
    motorControlerConfig.targetPosition = asciiToInt("tx") + motorID;
    motorControlerConfig.targetVelocity = asciiToInt("tv") + motorID;

    motorDriverCardConfig.motorControlerConfigurations[motorID] = motorControlerConfig;
  }

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
  mappedDevice->openDev( _dummyDevice, _registerMapping );

  //try something with a wrong firmware version
  // wrong major (too large by 1):
  _dummyDevice->setFirmwareVersion( dfmc_md22::MINIMAL_FIRMWARE_VERSION + 0x1000000 );
  BOOST_CHECK_THROW( _motorDriverCard.reset( new MotorDriverCardImpl( mappedDevice, motorDriverCardConfig ) ),
		     MotorDriverException );

  _dummyDevice->setFirmwareVersion( dfmc_md22::MINIMAL_FIRMWARE_VERSION -1 );
  BOOST_CHECK_THROW( _motorDriverCard.reset( new MotorDriverCardImpl( mappedDevice, motorDriverCardConfig ) ),
		     MotorDriverException );

  _dummyDevice->resetFirmwareVersion();
  
  _motorDriverCard.reset(new MotorDriverCardImpl( mappedDevice, motorDriverCardConfig ));

  // test that the configuration with the motorDriverCardConfig actually worked
  testConfiguration(motorDriverCardConfig);
  
  // only after initialising the dummy device with the motorDriverCardConfig in the constructor of the
  // MotorDriverCardImpl we can change the registers for testing
  _dummyDevice->setRegistersForTesting();  
  
}

void MotorDriverCardTest::testConfiguration(MotorDriverCardConfig const & motorDriverCardConfig){
  // This test checks that the configuration is written and can be read back under the assumption
  // that the read function is working, which is only a self-consistency test if the latter is not true.
  // The test that the read function actually does what it should is done later, using the index-dependent
  // test words from the dummy device. In this combination this test shows that the config is written
  // correctly.

  BOOST_CHECK( _motorDriverCard->getCoverDatagram() == motorDriverCardConfig.coverDatagram );
  BOOST_CHECK( _motorDriverCard->getCoverPositionAndLength() == motorDriverCardConfig.coverPositionAndLength );
  BOOST_CHECK( _motorDriverCard->getDatagramHighWord() == motorDriverCardConfig.datagramHighWord );
  BOOST_CHECK( _motorDriverCard->getDatagramLowWord() == motorDriverCardConfig.datagramLowWord );
  BOOST_CHECK( _motorDriverCard->getInterfaceConfiguration() == motorDriverCardConfig.interfaceConfiguration );
  BOOST_CHECK( _motorDriverCard->getPositionCompareInterruptData() == motorDriverCardConfig.positionCompareInterruptData );
  BOOST_CHECK( _motorDriverCard->getPositionCompareWord() == motorDriverCardConfig.positionCompareWord );
  BOOST_CHECK( _motorDriverCard->getStepperMotorGlobalParameters() == motorDriverCardConfig.stepperMotorGlobalParameters );

   for (unsigned int motorID = 0; motorID <  motorDriverCardConfig.motorControlerConfigurations.size();
       ++motorID){
     boost::shared_ptr<MotorControler> motorControler =  _motorDriverCard->getMotorControler(motorID);
     MotorControlerConfig motorControlerConfig = motorDriverCardConfig.motorControlerConfigurations[motorID];

     // only the data content is identical. The motorID is - for the config, and correct for the readback word
     BOOST_CHECK( motorControlerConfig.accelerationThresholdData.getDATA()
		  == motorControler->getAccelerationThresholdData().getDATA() );
     BOOST_CHECK( motorControlerConfig.actualPosition ==  motorControler->getActualPosition() );
     BOOST_CHECK( motorControlerConfig.chopperControlData == motorControler->getChopperControlData() );
     BOOST_CHECK( motorControlerConfig.coolStepControlData == motorControler->getCoolStepControlData() );
     BOOST_CHECK( motorControlerConfig.decoderReadoutMode ==  motorControler->getDecoderReadoutMode() );
     BOOST_CHECK( motorControlerConfig.dividersAndMicroStepResolutionData.getDATA()
		  == motorControler->getDividersAndMicroStepResolutionData().getDATA() );
     BOOST_CHECK( motorControlerConfig.driverConfigData == motorControler->getDriverConfigData() );
     BOOST_CHECK( motorControlerConfig.driverControlData == motorControler->getDriverControlData() );
     BOOST_CHECK( motorControlerConfig.enabled == motorControler->isEnabled() );
     BOOST_CHECK( motorControlerConfig.interruptData.getDATA()
		  == motorControler->getInterruptData().getDATA() );
     BOOST_CHECK( motorControlerConfig.maximumAccelleration == motorControler->getMaximumAcceleration() );
     BOOST_CHECK( motorControlerConfig.maximumVelocity == motorControler->getMaximumVelocity() );
     BOOST_CHECK( motorControlerConfig.microStepCount == motorControler->getMicroStepCount() );
     BOOST_CHECK( motorControlerConfig.minimumVelocity == motorControler->getMinimumVelocity() );
     BOOST_CHECK( motorControlerConfig.positionTolerance ==  motorControler->getPositionTolerance() );
     BOOST_CHECK( motorControlerConfig.proportionalityFactorData.getDATA()
		  == motorControler->getProportionalityFactorData().getDATA() );
     BOOST_CHECK( motorControlerConfig.referenceConfigAndRampModeData.getDATA()
		  == motorControler->getReferenceConfigAndRampModeData().getDATA() );
     BOOST_CHECK( motorControlerConfig.stallGuardControlData ==  motorControler->getStallGuardControlData() );
     BOOST_CHECK( motorControlerConfig.targetPosition == motorControler->getTargetPosition() );
     BOOST_CHECK( motorControlerConfig.targetVelocity ==  motorControler->getTargetVelocity() );
  }

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

void MotorDriverCardTest::testGetStatusWord(){
  mapFile::mapElem mapElement;
  _registerMapping->getRegisterInfo( CONTROLER_STATUS_BITS_ADDRESS_STRING, mapElement );

  unsigned int expectedContent = testWordFromPCIeAddress( mapElement.reg_address );

  BOOST_CHECK( _motorDriverCard->getStatusWord().getDataWord() == expectedContent );
}

void MotorDriverCardTest::testGetDatagramLowWord(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_DATAGRAM_LOW_WORD);
  BOOST_CHECK( _motorDriverCard->getDatagramLowWord() == expectedContent );
}

// just for the fun of it I use AAAA, 5555 and FFFF alternating as test patterns
void MotorDriverCardTest::testSetDatagramLowWord(){
  _motorDriverCard->setDatagramLowWord( 0xAAAAAA );
  BOOST_CHECK( _motorDriverCard->getDatagramLowWord() == 0xAAAAAA );
}

void MotorDriverCardTest::testGetDatagramHighWord(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_DATAGRAM_HIGH_WORD);
  BOOST_CHECK( _motorDriverCard->getDatagramHighWord() == expectedContent );
}

void MotorDriverCardTest::testSetDatagramHighWord(){
  _motorDriverCard->setDatagramHighWord( 0x555555 );
  BOOST_CHECK( _motorDriverCard->getDatagramHighWord() == 0x555555 );
}

void MotorDriverCardTest::testGetCoverPositionAndLength(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_COVER_POSITION_AND_LENGTH);
  BOOST_CHECK( _motorDriverCard->getCoverPositionAndLength() == CoverPositionAndLength(expectedContent) );
}

void MotorDriverCardTest::testSetCoverPositionAndLength(){
  _motorDriverCard->setCoverPositionAndLength( 0xFFFFFF );
  BOOST_CHECK( _motorDriverCard->getCoverPositionAndLength() == CoverPositionAndLength(0xFFFFFF) );
}

void MotorDriverCardTest::testGetCoverDatagram(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_COVER_DATAGRAM);
  BOOST_CHECK( _motorDriverCard->getCoverDatagram() == expectedContent );
}

void MotorDriverCardTest::testSetCoverDatagram(){
  _motorDriverCard->setCoverDatagram( 0xAAAAAA );
  BOOST_CHECK( _motorDriverCard->getCoverDatagram() == 0xAAAAAA );
}

void MotorDriverCardTest::testGetStepperMotorGlobalParameters(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_STEPPER_MOTOR_GLOBAL_PARAMETERS);
  BOOST_CHECK( _motorDriverCard->getStepperMotorGlobalParameters() == StepperMotorGlobalParameters(expectedContent) );
}

void MotorDriverCardTest::testSetStepperMotorGlobalParameters(){
  _motorDriverCard->setStepperMotorGlobalParameters( 0x555555 );
  BOOST_CHECK( _motorDriverCard->getStepperMotorGlobalParameters() == StepperMotorGlobalParameters(0x555555) );
}

void MotorDriverCardTest::testGetInterfaceConfiguration(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_INTERFACE_CONFIGURATION);
  BOOST_CHECK( _motorDriverCard->getInterfaceConfiguration() == InterfaceConfiguration(expectedContent) );
}

void MotorDriverCardTest::testSetInterfaceConfiguration(){
  _motorDriverCard->setInterfaceConfiguration( 0x555555 );
  BOOST_CHECK( _motorDriverCard->getInterfaceConfiguration() == InterfaceConfiguration(0x555555) );
}

void MotorDriverCardTest::testGetPositionCompareWord(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_POSITION_COMPARE);
  BOOST_CHECK( _motorDriverCard->getPositionCompareWord() == expectedContent );
}

void MotorDriverCardTest::testSetPositionCompareWord(){
  _motorDriverCard->setPositionCompareWord( 0xFFFFFF );
  BOOST_CHECK( _motorDriverCard->getPositionCompareWord() == 0xFFFFFF );
}

void MotorDriverCardTest::testGetPositionCompareInterruptData(){
  unsigned int expectedContent = testWordFromSpiAddress( SMDA_COMMON,
							 JDX_POSITION_COMPARE_INTERRUPT);
  BOOST_CHECK( _motorDriverCard->getPositionCompareInterruptData() == PositionCompareInterruptData(expectedContent) );
}

void MotorDriverCardTest::testSetPositionCompareInterruptData(){
  _motorDriverCard->setPositionCompareInterruptData( 0xAAAAAA );
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
    BOOST_CHECK( _motorDriverCard->getMotorControler(i)->getID() == i );
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

}//namespace mtca4u

test_suite*
init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
   framework::master_test_suite().p_name.value = "MotorDriverCard test suite";

   return new mtca4u::MotorDriverCardTestSuite;
}
