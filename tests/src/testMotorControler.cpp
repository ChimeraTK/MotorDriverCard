#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "DFMC_MD22Dummy.h"
#include "MotorDriverCardImpl.h"
#include <MtcaMappedDevice/devMap.h>
#include <MtcaMappedDevice/libmap.h>

#include "DFMC_MD22Constants.h"
#include "testWordFromPCIeAddress.h"
using namespace mtca4u::dfmc_md22;
#include "testWordFromSpiAddress.h"
using namespace mtca4u::tmc429;

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
  unsigned int expectedValue = testWordFromSpiAddress( _motorControler.getID(),\
                                                       IDX );\
  BOOST_CHECK( _motorControler.get ## NAME () == expectedValue );}\
void MotorControlerTest::testSet ## NAME (){\
  _motorControler.set ## NAME ( PATTERN );\
  BOOST_CHECK( _motorControler.get ## NAME () == (PATTERN & spiDataMask) );}

using namespace mtca4u;

class MotorControlerTest{
public:
  MotorControlerTest(MotorControler & motorControler, 
		     boost::shared_ptr<mapFile> & registerMapping);
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

  DECLARE_GET_SET_TEST( AccelerationThresholdRegister );
  
private:
  MotorControler & _motorControler;
  boost::shared_ptr<mapFile> _registerMapping;
  static unsigned int const spiDataMask = 0xFFFFFF;
};

class  MotorControlerTestSuite : public test_suite{
private:
    boost::shared_ptr<MotorDriverCard> _motorDriverCard;
public:
  MotorControlerTestSuite(std::string const & mapFileName) 
    : test_suite(" MotorControler test suite"){

    boost::shared_ptr<DFMC_MD22Dummy> dummyDevice( new DFMC_MD22Dummy );
    dummyDevice->openDev( mapFileName );
    dummyDevice->setSPIRegistersForTesting();
 
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
				       registerMapping) );
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

      ADD_GET_SET_TEST( AccelerationThresholdRegister );
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
					 boost::shared_ptr<mapFile> & registerMapping ) 
  : _motorControler(motorControler), _registerMapping( registerMapping ){
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

void MotorControlerTest::testGetAccelerationThresholdRegister(){
  unsigned int expectedContent = testWordFromSpiAddress( _motorControler.getID(),
							 IDX_ACCELERATION_THRESHOLD);
  // this assignment checks that the delivered data object actually is an AccelerationThresholdData object.
  AccelerationThresholdData threshodData = _motorControler.readAccelerationThresholdRegister();
  BOOST_CHECK( threshodData.getDATA() == expectedContent );
  BOOST_CHECK( threshodData.getSMDA() == _motorControler.getID() );
  std::cout << std::hex << "expectedContent " << expectedContent << std::endl;
}

void MotorControlerTest::testSetAccelerationThresholdRegister(){
  
  AccelerationThresholdData thresholdData = _motorControler.readAccelerationThresholdRegister();
  unsigned int dataContent = thresholdData.getDATA();
  thresholdData.setDATA(++dataContent);
  _motorControler.writeAccelerationThresholdRegister(thresholdData);
  BOOST_CHECK( _motorControler.readAccelerationThresholdRegister()
	       == thresholdData );
  std::cout << std::hex << "read " << _motorControler.readAccelerationThresholdRegister().getDataWord() 
	    << ", written " << thresholdData.getDataWord() << std::endl;
    
}
