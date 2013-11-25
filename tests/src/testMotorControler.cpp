#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

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
  unsigned int expectedValue = tmc429::testWordFromSpiAddress( _motorControler.getID(),\
                                                       IDX );\
  BOOST_CHECK( _motorControler.get ## NAME () == expectedValue );}\
void MotorControlerTest::testSet ## NAME (){\
  _motorControler.set ## NAME ( PATTERN );\
  BOOST_CHECK( _motorControler.get ## NAME () == (PATTERN & SPI_DATA_MASK) );}

#define DEFINE_TYPED_GET_SET_TEST( NAME )\
void MotorControlerTest::testGet ## NAME (){\
  testGetTypedData< NAME >( &MotorControler::get ## NAME );}\
void MotorControlerTest::testSet ## NAME (){\
  testSetTypedData< NAME >( &MotorControler::get ## NAME ,\
			    &MotorControler::set ## NAME );}

#define DEFINE_GET_SET_DRIVER_SPI_DATA( NAME, SPI_ADDRESS )\
void MotorControlerTest::testGet ## NAME (){\
  testGetDriverSpiData< NAME >( &MotorControler::get ## NAME , SPI_ADDRESS);} \
void MotorControlerTest::testSet ## NAME (){\
  testSetDriverSpiData< NAME >( &MotorControler::set ## NAME ,\
                                &MotorControler::get ## NAME ,\
				SPI_ADDRESS );}

using namespace mtca4u;

class MotorControlerTest{
public:
  MotorControlerTest(MotorControler & motorControler, 
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
  
private:
  MotorControler & _motorControler;
  boost::shared_ptr<mapFile> _registerMapping;
  boost::shared_ptr<DFMC_MD22Dummy> _dummyDevice;

  template<class T>
  void testGetTypedData( T (MotorControler::* getterFunction)() );

  template<class T>
  void testSetTypedData( T (MotorControler::* getterFunction)(),
			       void (MotorControler::* setterFunction)(T const &) );
  template <class T>
  void testGetDriverSpiData( T const & (MotorControler::* getterFunction)() const,
				 unsigned int driverSpiAddress);
  template <class T>
  void testSetDriverSpiData( void (MotorControler::* setterFunction)(T const &),
				 T const & (MotorControler::* getterFunction)() const,
				 unsigned int driverSpiAddress);
};

class  MotorControlerTestSuite : public test_suite{
private:
    boost::shared_ptr<MotorDriverCard> _motorDriverCard;
public:
  MotorControlerTestSuite(std::string const & mapFileName) 
    : test_suite(" MotorControler test suite"){

    boost::shared_ptr<DFMC_MD22Dummy> dummyDevice( new DFMC_MD22Dummy );
    dummyDevice->openDev( mapFileName );
    dummyDevice->setControlerSpiRegistersForTesting();
 
    mapFileParser fileParser;
    boost::shared_ptr<mapFile> registerMapping = fileParser.parse(mapFileName);

    boost::shared_ptr< devMap<devBase> > mappedDevice(new devMap<devBase>);
    MotorDriverCardImpl::MotorDriverConfiguration  motorDriverConfiguration;
   
    mappedDevice->openDev( dummyDevice, registerMapping );
    // We need the motor driver card as private variable because it has to
    // survive the constructor. Otherwise the MotorControlers passed to
    // the tests will be invalid.
    _motorDriverCard.reset( new MotorDriverCardImpl( mappedDevice,
						     motorDriverConfiguration ) );

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
	      boost::bind( &MotorControlerTest::testReadPCIeRegister,
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
   }// for i < N_MOTORS_MAX
  }// constructor
};// test suite

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
   framework::master_test_suite().p_name.value = "MotorControler test suite";

   return new MotorControlerTestSuite(MAP_FILE_NAME);
}


  MotorControlerTest::MotorControlerTest(MotorControler & motorControler,
					 boost::shared_ptr<mapFile> & registerMapping,
					 boost::shared_ptr<DFMC_MD22Dummy> dummyDevice)
    : _motorControler(motorControler), _registerMapping( registerMapping ),
      _dummyDevice(dummyDevice){
}

DEFINE_GET_SET_TEST( ActualPosition, IDX_ACTUAL_POSITION, 0x55555555 )
DEFINE_GET_SET_TEST( ActualVelocity, IDX_ACTUAL_VELOCITY, 0xAAAAAAAA )
DEFINE_GET_SET_TEST( ActualAcceleration, IDX_ACTUAL_ACCELERATION, 0xFFFFFFFF ) 
DEFINE_GET_SET_TEST( MicroStepCount, IDX_MICRO_STEP_COUNT, 0xAAAAAAAA ) 

void MotorControlerTest::testReadPCIeRegister( unsigned int(MotorControler::* readFunction)(void),
						std::string const & registerSuffix){
  std::string registerName = createMotorRegisterName( _motorControler.getID(),
						      registerSuffix );
  mapFile::mapElem registerInfo;
  _registerMapping->getRegisterInfo( registerName, registerInfo );
  unsigned int expectedValue = testWordFromPCIeAddress( registerInfo.reg_address );
  BOOST_CHECK( (_motorControler.*readFunction)() == expectedValue );
}

void MotorControlerTest::testGetDecoderReadoutMode(){
  testReadPCIeRegister( &MotorControler::getDecoderReadoutMode,
			DECODER_READOUT_MODE_SUFFIX );
}

void MotorControlerTest::testSetDecoderReadoutMode(){
  unsigned int readoutMode = _motorControler.getDecoderReadoutMode();
  _motorControler.setDecoderReadoutMode( ++readoutMode );
  BOOST_CHECK( _motorControler.getDecoderReadoutMode() == readoutMode );
}

void MotorControlerTest::testSetIsEnabled(){
  BOOST_CHECK( _motorControler.isEnabled() );
  _motorControler.setEnabled( false );
  BOOST_CHECK( _motorControler.isEnabled()==false );
  _motorControler.setEnabled();
  BOOST_CHECK( _motorControler.isEnabled() );
}

DEFINE_GET_SET_TEST( TargetPosition, IDX_TARGET_POSITION, 0x55555555 ) 
DEFINE_GET_SET_TEST( MinimumVelocity, IDX_MINIMUM_VELOCITY, 0xFFFFFFFF )
DEFINE_GET_SET_TEST( MaximumVelocity, IDX_MAXIMUM_VELOCITY, 0xAAAAAAAA )
DEFINE_GET_SET_TEST( TargetVelocity, IDX_TARGET_VELOCITY, 0x55555555 )
DEFINE_GET_SET_TEST( MaximumAcceleration, IDX_MAXIMUM_ACCELERATION, 0xFFFFFFFF )
DEFINE_GET_SET_TEST( PositionTolerance, IDX_DELTA_X_REFERENCE_TOLERANCE, 0xAAAAAAAA )
DEFINE_GET_SET_TEST( PositionLatched, IDX_POSITION_LATCHED, 0x55555555 )

template<class T>
void MotorControlerTest::testGetTypedData( T (MotorControler::* getterFunction)() ){
  T typedWord;
  unsigned int idx = typedWord.getIDX_JDX();
  unsigned int expectedContent = tmc429::testWordFromSpiAddress( _motorControler.getID(),
								 idx );
  // this assignment checks that the delivered data object actually is the right type of object.
  typedWord = (_motorControler.*getterFunction)();
  BOOST_CHECK( typedWord.getDATA() == expectedContent );
  BOOST_CHECK( typedWord.getSMDA() == _motorControler.getID() );
}

template<class T>
void MotorControlerTest::testSetTypedData( T (MotorControler::* getterFunction)(),
					   void (MotorControler::* setterFunction)(T const &) ){
  T typedWord = (_motorControler.*getterFunction)();
  unsigned int dataContent = typedWord.getDATA();
  typedWord.setDATA(++dataContent);
  (_motorControler.*setterFunction)(typedWord);
  BOOST_CHECK( (_motorControler.*getterFunction)() == typedWord );    
}

DEFINE_TYPED_GET_SET_TEST( AccelerationThresholdData )
DEFINE_TYPED_GET_SET_TEST( ProportionalityFactorData )
DEFINE_TYPED_GET_SET_TEST( ReferenceConfigAndRampModeData )
DEFINE_TYPED_GET_SET_TEST( InterruptData )
DEFINE_TYPED_GET_SET_TEST( DividersAndMicroStepResolutionData )

template <class T>
void MotorControlerTest::testGetDriverSpiData( T const & (MotorControler::* getterFunction)() const,
						   unsigned int driverSpiAddress){
  T expectedWord = tmc260::testWordFromSpiAddress(driverSpiAddress, _motorControler.getID());

  BOOST_CHECK( _dummyDevice->readDriverSpiRegister( _motorControler.getID(), driverSpiAddress )==
	       expectedWord.getPayloadData() );
  BOOST_CHECK( (_motorControler.*getterFunction)() == T() ); // T is uninitialised at the moment
}

template <class T>
void MotorControlerTest::testSetDriverSpiData( void (MotorControler::* setterFunction)(T const &),
						   T const & (MotorControler::* getterFunction)() const,
						   unsigned int driverSpiAddress){
  T testWord( 0xAAAAAAAA );
  (_motorControler.* setterFunction)(testWord);

  // the new word must arrive at the spi register in the "hardware" and a copy has to be in the motor controler
  BOOST_CHECK( _dummyDevice->readDriverSpiRegister( _motorControler.getID(), driverSpiAddress )==
	       testWord.getPayloadData() );
  BOOST_CHECK((_motorControler.* getterFunction)() == testWord );
}

DEFINE_GET_SET_DRIVER_SPI_DATA( DriverControlData, ADDRESS_DRIVER_CONTROL )
DEFINE_GET_SET_DRIVER_SPI_DATA( ChopperControlData, ADDRESS_CHOPPER_CONFIG )
DEFINE_GET_SET_DRIVER_SPI_DATA( CoolStepControlData, ADDRESS_COOL_STEP_CONFIG )
DEFINE_GET_SET_DRIVER_SPI_DATA( StallGuardControlData, ADDRESS_STALL_GUARD_CONFIG )
DEFINE_GET_SET_DRIVER_SPI_DATA( DriverConfigData, ADDRESS_DRIVER_CONFIG )
