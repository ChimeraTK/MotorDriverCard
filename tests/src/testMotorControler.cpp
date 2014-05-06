#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include <sstream>

#include "DFMC_MD22Dummy.h"
#include "MotorDriverCardImpl.h"
#include <MtcaMappedDevice/devMap.h>
#include <MtcaMappedDevice/libmap.h>

#include "DFMC_MD22Constants.h"
#include "testWordFromPCIeAddress.h"
using namespace mtca4u::dfmc_md22;
#include "TMC429DummyConstants.h"
#include "TMC260DummyConstants.h"
#include "testWordFromSpiAddress.h"
using namespace mtca4u::tmc429;
using namespace mtca4u::tmc260;

#include "MotorControlerConfigDefaults.h"
using namespace mtca4u;

#define MAP_FILE_NAME "DFMC_MD22_test.map"

#define DECLARE_GET_SET_TEST( NAME )\
  void testGet ## NAME ();\
  void testSet ## NAME ()

#define ADD_GET_SET_TEST( NAME )\
  test_case* set ## NAME ## TestCase =  BOOST_CLASS_TEST_CASE( &MotorControlerTest::testSet ## NAME , motorControlerTest );\
  test_case* get ## NAME ## TestCase =  BOOST_CLASS_TEST_CASE( &MotorControlerTest::testGet ## NAME , motorControlerTest );\
  set ## NAME ## TestCase->depends_on( get ## NAME ## TestCase );\
  add( get ## NAME ## TestCase );\
  add( set ## NAME ## TestCase )

#define DEFINE_GET_SET_TEST( NAME, IDX, PATTERN )\
void MotorControlerTest::testGet ## NAME (){\
  unsigned int expectedValue = tmc429::testWordFromSpiAddress( _motorControler->getID(),\
                                                       IDX );\
  BOOST_CHECK( _motorControler->get ## NAME () == expectedValue );}\
void MotorControlerTest::testSet ## NAME (){\
  _motorControler->set ## NAME ( PATTERN );\
  BOOST_CHECK( _motorControler->get ## NAME () == (PATTERN & SPI_DATA_MASK) );}

#define DEFINE_SIGNED_GET_SET_TEST( NAME, IDX, NBITS )	\
void MotorControlerTest::testGet ## NAME (){\
  SignedIntConverter converter( NBITS );\
  unsigned int expectedValue = tmc429::testWordFromSpiAddress( _motorControler->getID(),\
                                                       IDX );\
  BOOST_CHECK( _motorControler->get ## NAME () == converter.customToThirtyTwo(expectedValue) ); \
 }\
void MotorControlerTest::testSet ## NAME (){\
  unsigned int positiveMask = (1 << (NBITS-1) ) -1 ;\
  unsigned int negativeBit  = (1 << (NBITS-1) ) ;\
  _motorControler->set ## NAME ( 0xAAAAAAAA & positiveMask );/* something positive */ \
  BOOST_CHECK( _motorControler->get ## NAME () == static_cast<int>(0xAAAAAAAA & positiveMask) );\
  _motorControler->set ## NAME ( (0x55555555 & positiveMask) | negativeBit );/* out of range positive with the custom negative bit active*/ \
  BOOST_CHECK( _motorControler->get ## NAME () == -static_cast<int>( (0xAAAAAAAA +1) & positiveMask) );/* is interpreted as negative*/ \
  _motorControler->set ## NAME ( -1 );/* a normal neagative number */	\
  BOOST_CHECK( _motorControler->get ## NAME () == -1 );\
  _motorControler->set ## NAME ( 0xFFFFFFFF & ~negativeBit );/* something negative with the negative bit of the custom variable not set*/	\
  BOOST_CHECK( _motorControler->get ## NAME () == static_cast<int>(0xFFFFFFFF & positiveMask) );/* is interpreted as positive */ \
}

#define DEFINE_TYPED_GET_SET_TEST( NAME )\
void MotorControlerTest::testGet ## NAME (){\
  testGetTypedData< NAME >( &MotorControler::get ## NAME );}\
void MotorControlerTest::testSet ## NAME (){\
  testSetTypedData< NAME >( &MotorControler::get ## NAME ,\
			    &MotorControler::set ## NAME );}

#define DEFINE_GET_SET_DRIVER_SPI_DATA( NAME, SPI_ADDRESS, CONFIG_DEFAULT, TEST_PATTERN ) \
void MotorControlerTest::testGet ## NAME (){\
  testGetDriverSpiData< NAME >( &MotorControler::get ## NAME , SPI_ADDRESS, CONFIG_DEFAULT );} \
void MotorControlerTest::testSet ## NAME (){\
  testSetDriverSpiData< NAME >( &MotorControler::set ## NAME ,\
                                &MotorControler::get ## NAME ,\
				SPI_ADDRESS,\
                                TEST_PATTERN );}

using namespace mtca4u;

class MotorControlerTest{
public:
  MotorControlerTest(boost::shared_ptr<MotorControler> const & motorControler, 
		     boost::shared_ptr<mapFile> & registerMapping,
		     boost::shared_ptr<DFMC_MD22Dummy> dummyDevice);
  // getID() is tested in the MotorDriver card, where 
  // different ID are known. This test is for just one
  // Motor with one ID, which is now known to be ok.
  DECLARE_GET_SET_TEST( ActualPosition );
  DECLARE_GET_SET_TEST( ActualVelocity );
  DECLARE_GET_SET_TEST( ActualAcceleration );
  DECLARE_GET_SET_TEST( MicroStepCount );

  // for testing direct read from pcie registers (with default values from the dummy)
  void testReadPCIeRegister( unsigned int(MotorControler::* readFunction)(void),
			     std::string const & registerSuffix);

  // for reading registers which do not return an int but a MultiVariableWord
  template<class T>
  void testReadTypedPCIeRegister( T (MotorControler::* readFunction)(void),
				  std::string const & registerSuffix);

  void testSetIsEnabled();

  DECLARE_GET_SET_TEST( DecoderReadoutMode );
  DECLARE_GET_SET_TEST( TargetPosition );
  DECLARE_GET_SET_TEST( MinimumVelocity );
  DECLARE_GET_SET_TEST( MaximumVelocity );
  DECLARE_GET_SET_TEST( TargetVelocity );
  DECLARE_GET_SET_TEST( MaximumAcceleration );
  DECLARE_GET_SET_TEST( PositionTolerance );
  DECLARE_GET_SET_TEST( PositionLatched );

  DECLARE_GET_SET_TEST( AccelerationThresholdData );
  DECLARE_GET_SET_TEST( ProportionalityFactorData );
  DECLARE_GET_SET_TEST( ReferenceConfigAndRampModeData );
  DECLARE_GET_SET_TEST( InterruptData );
  DECLARE_GET_SET_TEST( DividersAndMicroStepResolutionData );

  DECLARE_GET_SET_TEST( DriverControlData );
  DECLARE_GET_SET_TEST( ChopperControlData );
  DECLARE_GET_SET_TEST( CoolStepControlData );
  DECLARE_GET_SET_TEST( StallGuardControlData );
  DECLARE_GET_SET_TEST( DriverConfigData );

  void testGetReferenceSwitchData( boost::shared_ptr<MotorDriverCardExpert> motorDriverCard );
  void testSetReferenceSwitchEnabled();
  
  void testTargetPositionReached();
  void testGetReferenceSwitchBit();

private:
  boost::shared_ptr<MotorControler> _motorControler;
  boost::shared_ptr<mapFile> _registerMapping;
  boost::shared_ptr<DFMC_MD22Dummy> _dummyDevice;

  template<class T>
  void testGetTypedData( T (MotorControler::* getterFunction)() );

  template<class T>
  void testSetTypedData( T (MotorControler::* getterFunction)(),
			       void (MotorControler::* setterFunction)(T const &) );
  template <class T>
  void testGetDriverSpiData( T const & (MotorControler::* getterFunction)() const,
			     unsigned int driverSpiAddress,
			     unsigned int configDefault);
  template <class T>
  void testSetDriverSpiData( void (MotorControler::* setterFunction)(T const &),
			     T const & (MotorControler::* getterFunction)() const,
			     unsigned int driverSpiAddress,
			     unsigned int testPattern);

  unsigned int testWordFromPCIeSuffix(std::string const & registerSuffix);

};

class  MotorControlerTestSuite : public test_suite{
private:
    boost::shared_ptr<MotorDriverCardExpert> _motorDriverCard;
public:
  MotorControlerTestSuite(std::string const & mapFileName) 
    : test_suite(" MotorControler test suite"){

    boost::shared_ptr<DFMC_MD22Dummy> dummyDevice( new DFMC_MD22Dummy );
    dummyDevice->openDev( mapFileName );
 
    mapFileParser fileParser;
    boost::shared_ptr<mapFile> registerMapping = fileParser.parse(mapFileName);

    boost::shared_ptr< devMap<devBase> > mappedDevice(new devMap<devBase>);
    MotorDriverCardConfig  motorDriverCardConfig;
   
    mappedDevice->openDev( dummyDevice, registerMapping );
    // We need the motor driver card as private variable because it has to
    // survive the constructor. Otherwise the MotorControlers passed to
    // the tests will be invalid.
    _motorDriverCard.reset( new MotorDriverCardImpl( mappedDevice,
						     motorDriverCardConfig ) );
    dummyDevice->setRegistersForTesting();
    
    for (unsigned int i = 0; i < N_MOTORS_MAX ; ++i){
      boost::shared_ptr<MotorControlerTest> motorControlerTest( 
	       new MotorControlerTest( _motorDriverCard->getMotorControler( i ) ,
				       registerMapping, 
				       dummyDevice) );
      ADD_GET_SET_TEST( ActualPosition );
      ADD_GET_SET_TEST( ActualVelocity );
      ADD_GET_SET_TEST( ActualAcceleration );
      //      ADD_GET_SET_TEST( AccelerationThreshold );
      ADD_GET_SET_TEST( MicroStepCount );
      
      add( BOOST_TEST_CASE(
	      boost::bind( &MotorControlerTest::testReadPCIeRegister,
			   motorControlerTest,
			   &MotorControler::getStallGuardValue,  STALL_GUARD_VALUE_SUFFIX ) ));
      add( BOOST_TEST_CASE(
	      boost::bind( &MotorControlerTest::testReadPCIeRegister,
			   motorControlerTest,
			   &MotorControler::getCoolStepValue,  COOL_STEP_VALUE_SUFFIX ) ));
      add( BOOST_TEST_CASE(
			   boost::bind( &MotorControlerTest::testReadTypedPCIeRegister<DriverStatusData>,
			   motorControlerTest,
			   &MotorControler::getStatus, STATUS_SUFFIX ) ));

      add( BOOST_CLASS_TEST_CASE( &MotorControlerTest::testSetIsEnabled,
				  motorControlerTest ) );
  
      ADD_GET_SET_TEST( DecoderReadoutMode );

      add( BOOST_TEST_CASE(
	      boost::bind( &MotorControlerTest::testReadPCIeRegister,
			   motorControlerTest,
			   &MotorControler::getDecoderPosition, DECODER_POSITION_SUFFIX ) ));

      ADD_GET_SET_TEST( TargetPosition );
      ADD_GET_SET_TEST( MinimumVelocity );
      ADD_GET_SET_TEST( MaximumVelocity );
      ADD_GET_SET_TEST( TargetVelocity ); 
      ADD_GET_SET_TEST( MaximumAcceleration );
      ADD_GET_SET_TEST( PositionTolerance );
      ADD_GET_SET_TEST( PositionLatched );

      ADD_GET_SET_TEST( AccelerationThresholdData );
      ADD_GET_SET_TEST( ProportionalityFactorData );
      ADD_GET_SET_TEST( ReferenceConfigAndRampModeData );
      ADD_GET_SET_TEST( InterruptData );
      ADD_GET_SET_TEST( DividersAndMicroStepResolutionData );

      ADD_GET_SET_TEST( DriverControlData );
      ADD_GET_SET_TEST( ChopperControlData );
      ADD_GET_SET_TEST( CoolStepControlData );
      ADD_GET_SET_TEST( StallGuardControlData );
      ADD_GET_SET_TEST( DriverConfigData );

      add( BOOST_TEST_CASE( boost::bind( &MotorControlerTest::testGetReferenceSwitchData,
					 motorControlerTest,
					 _motorDriverCard) ));
      add( BOOST_CLASS_TEST_CASE( &MotorControlerTest::testSetReferenceSwitchEnabled,
				  motorControlerTest ) );
      add( BOOST_CLASS_TEST_CASE( &MotorControlerTest::testTargetPositionReached,
				  motorControlerTest ) );
      add( BOOST_CLASS_TEST_CASE( &MotorControlerTest::testGetReferenceSwitchBit,
				  motorControlerTest ) );

   }// for i < N_MOTORS_MAX
  }// constructor
};// test suite

test_suite*
init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
   framework::master_test_suite().p_name.value = "MotorControler test suite";

   return new MotorControlerTestSuite(MAP_FILE_NAME);
}


MotorControlerTest::MotorControlerTest(boost::shared_ptr<MotorControler> const & motorControler,
				       boost::shared_ptr<mapFile> & registerMapping,
				       boost::shared_ptr<DFMC_MD22Dummy> dummyDevice)
  : _motorControler(motorControler), _registerMapping( registerMapping ),
    _dummyDevice(dummyDevice){
}

DEFINE_SIGNED_GET_SET_TEST( ActualPosition, IDX_ACTUAL_POSITION, 24 )
DEFINE_SIGNED_GET_SET_TEST( ActualVelocity, IDX_ACTUAL_VELOCITY, 12  )
DEFINE_GET_SET_TEST( ActualAcceleration, IDX_ACTUAL_ACCELERATION, 0xFFFFFF ) 
DEFINE_GET_SET_TEST( MicroStepCount, IDX_MICRO_STEP_COUNT, 0xAAAAAA ) 

void MotorControlerTest::testReadPCIeRegister( unsigned int(MotorControler::* readFunction)(void),
						std::string const & registerSuffix){
  unsigned int expectedValue = testWordFromPCIeSuffix(registerSuffix);
  std::stringstream message;
  message << "read () " <<  ((*_motorControler).*readFunction)() 
	  << ", expected " << expectedValue << std::endl;
  BOOST_CHECK_MESSAGE( ((*_motorControler).*readFunction)() == expectedValue , message.str());
}

unsigned int  MotorControlerTest::testWordFromPCIeSuffix(std::string const & registerSuffix){
  std::string registerName = createMotorRegisterName( _motorControler->getID(),
						      registerSuffix );
  mapFile::mapElem registerInfo;
  _registerMapping->getRegisterInfo( registerName, registerInfo );
  return testWordFromPCIeAddress( registerInfo.reg_address );
}

template<class T>
void MotorControlerTest::testReadTypedPCIeRegister( T (MotorControler::* readFunction)(void),
						    std::string const & registerSuffix){
  unsigned int expectedValue = testWordFromPCIeSuffix(registerSuffix);
  std::stringstream message;
  message << "read () " <<  ((*_motorControler).*readFunction)().getDataWord()
	  << ", expected " << expectedValue << std::endl;
  BOOST_CHECK_MESSAGE( ((*_motorControler).*readFunction)().getDataWord() == expectedValue , message.str());
}

void MotorControlerTest::testGetDecoderReadoutMode(){
  testReadPCIeRegister( &MotorControler::getDecoderReadoutMode,
			DECODER_READOUT_MODE_SUFFIX );
}

void MotorControlerTest::testSetDecoderReadoutMode(){
  unsigned int readoutMode = _motorControler->getDecoderReadoutMode();
  _motorControler->setDecoderReadoutMode( ++readoutMode );
  BOOST_CHECK( _motorControler->getDecoderReadoutMode() == readoutMode );
}

void MotorControlerTest::testSetIsEnabled(){
  BOOST_CHECK( _motorControler->isEnabled() );
  _motorControler->setEnabled( false );
  BOOST_CHECK( _motorControler->isEnabled()==false );
  _motorControler->setEnabled();
  BOOST_CHECK( _motorControler->isEnabled() );
}

DEFINE_SIGNED_GET_SET_TEST( TargetPosition, IDX_TARGET_POSITION, 24 )
DEFINE_GET_SET_TEST( MinimumVelocity, IDX_MINIMUM_VELOCITY, 0xFFFFFF )
DEFINE_GET_SET_TEST( MaximumVelocity, IDX_MAXIMUM_VELOCITY, 0xAAAAAA )
DEFINE_SIGNED_GET_SET_TEST( TargetVelocity, IDX_TARGET_VELOCITY, 12 )
DEFINE_GET_SET_TEST( MaximumAcceleration, IDX_MAXIMUM_ACCELERATION, 0xFFFFFF )
DEFINE_GET_SET_TEST( PositionTolerance, IDX_DELTA_X_REFERENCE_TOLERANCE, 0xAAAAAA )
DEFINE_GET_SET_TEST( PositionLatched, IDX_POSITION_LATCHED, 0x555555 )

template<class T>
void MotorControlerTest::testGetTypedData( T (MotorControler::* getterFunction)() ){
  T typedWord;
  unsigned int idx = typedWord.getIDX_JDX();
  unsigned int expectedContent = tmc429::testWordFromSpiAddress( _motorControler->getID(),
								 idx );
  // this assignment checks that the delivered data object actually is the right type of object.
  typedWord = ((*_motorControler).*getterFunction)();
  BOOST_CHECK( typedWord.getDATA() == expectedContent );
  BOOST_CHECK( typedWord.getSMDA() == _motorControler->getID() );
}

template<class T>
void MotorControlerTest::testSetTypedData( T (MotorControler::* getterFunction)(),
					   void (MotorControler::* setterFunction)(T const &) ){
  T typedWord = ((*_motorControler).*getterFunction)();
  unsigned int dataContent = typedWord.getDATA();
  typedWord.setDATA(++dataContent);
  ((*_motorControler).*setterFunction)(typedWord);
  BOOST_CHECK( ((*_motorControler).*getterFunction)() == typedWord );    
}

DEFINE_TYPED_GET_SET_TEST( AccelerationThresholdData )
DEFINE_TYPED_GET_SET_TEST( ProportionalityFactorData )
DEFINE_TYPED_GET_SET_TEST( ReferenceConfigAndRampModeData )
DEFINE_TYPED_GET_SET_TEST( InterruptData )
DEFINE_TYPED_GET_SET_TEST( DividersAndMicroStepResolutionData )

template <class T>
void MotorControlerTest::testGetDriverSpiData( T const & (MotorControler::* getterFunction)() const,
					       unsigned int driverSpiAddress,
					       unsigned int configDefault){
  T expectedWord = tmc260::testWordFromSpiAddress(driverSpiAddress, _motorControler->getID());

  BOOST_CHECK( _dummyDevice->readDriverSpiRegister( _motorControler->getID(), driverSpiAddress )==
	       expectedWord.getPayloadData() );
  BOOST_CHECK( ((*_motorControler).*getterFunction)() == T(configDefault) ); // T is uninitialised at the moment
}

template <class T>
void MotorControlerTest::testSetDriverSpiData( void (MotorControler::* setterFunction)(T const &),
					       T const & (MotorControler::* getterFunction)() const,
					       unsigned int driverSpiAddress,
					       unsigned int testPattern){
  T testWord( testPattern );
  ((*_motorControler).* setterFunction)(testWord);

  // the new word must arrive at the spi register in the "hardware" and a copy has to be in the motor controler
  BOOST_CHECK( _dummyDevice->readDriverSpiRegister( _motorControler->getID(), driverSpiAddress )==
	       testWord.getPayloadData() );
  BOOST_CHECK(((*_motorControler).* getterFunction)() == testWord );
}

DEFINE_GET_SET_DRIVER_SPI_DATA( DriverControlData, ADDRESS_DRIVER_CONTROL, DRIVER_CONTROL_DEFAULT , 0x2AAAA)
DEFINE_GET_SET_DRIVER_SPI_DATA( ChopperControlData, ADDRESS_CHOPPER_CONFIG, CHOPPER_CONTROL_DEFAULT , 0x1AAAA)
DEFINE_GET_SET_DRIVER_SPI_DATA( CoolStepControlData, ADDRESS_COOL_STEP_CONFIG, COOL_STEP_CONTROL_DEFAULT , 0x1AAAA)
DEFINE_GET_SET_DRIVER_SPI_DATA( StallGuardControlData, ADDRESS_STALL_GUARD_CONFIG, STALL_GUARD_CONTROL_DEFAULT, 0x1AAAA )
DEFINE_GET_SET_DRIVER_SPI_DATA( DriverConfigData, ADDRESS_DRIVER_CONFIG, DRIVER_CONFIG_DEFAULT , 0x1AAAA )

void MotorControlerTest::testGetReferenceSwitchData( boost::shared_ptr<MotorDriverCardExpert> motorDriverCard ){
  // This approach is intentinally clumsy and manual in order not to use the same algorithm as 
  // in the implementation, but still be felxible if the register content changes.
  unsigned int referenceWord;
  
  switch (_motorControler->getID()){
  case 0 : referenceWord = motorDriverCard->getReferenceSwitchData().getDATA() & 0x3; break;
  case 1 : referenceWord = ((motorDriverCard->getReferenceSwitchData().getDATA() & 0xC) >> 2); break;
  case 2 : referenceWord = ((motorDriverCard->getReferenceSwitchData().getDATA() & 0x30) >> 4); break;
  default:
    BOOST_FAIL("Invalid MotorID. Either there is something wrong in the code or the test has to be adapted.");
  }

  BOOST_CHECK(_motorControler->getReferenceSwitchData().getSwitchesActiveWord() == referenceWord );
  BOOST_CHECK(_motorControler->getReferenceSwitchData().getPositiveSwitchEnabled() ==
	      !_motorControler->getReferenceConfigAndRampModeData().getDISABLE_STOP_R() );
  BOOST_CHECK(_motorControler->getReferenceSwitchData().getNegativeSwitchEnabled() ==
	      !_motorControler->getReferenceConfigAndRampModeData().getDISABLE_STOP_L() );
}

void MotorControlerTest::testSetReferenceSwitchEnabled(){
  MotorReferenceSwitchData originalSetting = _motorControler->getReferenceSwitchData();

  _motorControler->setPositiveReferenceSwitchEnabled(true);
  _motorControler->setNegativeReferenceSwitchEnabled(true);
  BOOST_CHECK( _motorControler->getReferenceSwitchData().getSwitchesEnabledWord() == 0x3 );

  _motorControler->setPositiveReferenceSwitchEnabled(false);
  _motorControler->setNegativeReferenceSwitchEnabled(true);
  BOOST_CHECK( _motorControler->getReferenceSwitchData().getSwitchesEnabledWord() == 0x2 );

  _motorControler->setPositiveReferenceSwitchEnabled(true);
  _motorControler->setNegativeReferenceSwitchEnabled(false);
  BOOST_CHECK( _motorControler->getReferenceSwitchData().getSwitchesEnabledWord() == 0x1 );

  _motorControler->setPositiveReferenceSwitchEnabled(false);
  _motorControler->setNegativeReferenceSwitchEnabled(false);
  BOOST_CHECK( _motorControler->getReferenceSwitchData().getSwitchesEnabledWord() == 0x0 );
  
  _motorControler->setPositiveReferenceSwitchEnabled( originalSetting.getPositiveSwitchEnabled() );
  _motorControler->setNegativeReferenceSwitchEnabled( originalSetting.getNegativeSwitchEnabled() );

}

void MotorControlerTest::testTargetPositionReached(){
  mapFile::mapElem mapElement;
  _registerMapping->getRegisterInfo( CONTROLER_STATUS_BITS_ADDRESS_STRING, mapElement );

  TMC429StatusWord expectedControlerStatus( testWordFromPCIeAddress( mapElement.reg_address ) );

  BOOST_CHECK( expectedControlerStatus.getTargetPositionReached( _motorControler->getID() ) ==
	       _motorControler->targetPositionReached() );
}

void MotorControlerTest::testGetReferenceSwitchBit(){
  mapFile::mapElem mapElement;
  _registerMapping->getRegisterInfo( CONTROLER_STATUS_BITS_ADDRESS_STRING, mapElement );

  TMC429StatusWord expectedControlerStatus( testWordFromPCIeAddress( mapElement.reg_address ) );

  BOOST_CHECK( expectedControlerStatus.getReferenceSwitchBit( _motorControler->getID() ) ==
	       _motorControler->getReferenceSwitchBit() );
}
