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
  //  DECLARE_GET_SET_TEST( AccelerationThreshold );
  DECLARE_GET_SET_TEST( MicroStepCount );

  // for testing direct read from pcie registers (with default values from the dummy)
  void testReadPCIeRegister( unsigned int(MotorControler::* readFunction)(void),
			     std::string const & registerSuffix);

  void testSetIsEnabled();

  DECLARE_GET_SET_TEST( DecoderReadoutMode );
  
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
    }
  }
};

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

void MotorControlerTest::testGetActualPosition(){
  unsigned int expectedPosition = testWordFromSpiAddress( _motorControler.getID(),
							  IDX_ACTUAL_POSITION);  
  BOOST_CHECK( _motorControler.getActualPosition() == expectedPosition );
}

void MotorControlerTest::testSetActualPosition(){
  _motorControler.setActualPosition( 0x55555555 );
  BOOST_CHECK( _motorControler.getActualPosition() == 0x00555555 );
}

void MotorControlerTest::testGetActualVelocity(){
  unsigned int expectedVelocity = testWordFromSpiAddress( _motorControler.getID(),
							  IDX_ACTUAL_VELOCITY);  
  BOOST_CHECK( _motorControler.getActualVelocity() == expectedVelocity );
}

void MotorControlerTest::testSetActualVelocity(){
  _motorControler.setActualVelocity( 0xAAAAAAAA );
  BOOST_CHECK( _motorControler.getActualVelocity() == 0x00AAAAAA );
}

void MotorControlerTest::testGetActualAcceleration(){
  unsigned int expectedAcceleration = testWordFromSpiAddress( _motorControler.getID(),
							  IDX_ACTUAL_ACCELERATION);  
  BOOST_CHECK( _motorControler.getActualAcceleration() == expectedAcceleration );
}

void MotorControlerTest::testSetActualAcceleration(){
  _motorControler.setActualAcceleration( 0xFFFFFFFF );
  BOOST_CHECK( _motorControler.getActualAcceleration() == 0x00FFFFFF );
}

//void MotorControlerTest::testGetAccelerationThreshold(){
//  AccelerationThresholdData thresholdData = testWordFromSpiAddress( 
//                                                  _motorControler.getID(),
//						  IDX_ACCELERATION_THRESHOLD);  
//  BOOST_CHECK( _motorControler.getActualAcceleration() == 
//	       thresholdData.getAccelerationThreshold() );
//}
//
//void MotorControlerTest::testSetAccelerationThreshold(){
//  _motorControler.setAccelerationThreshold( 0x55555555 );
//  BOOST_CHECK( _motorControler.getAccelerationThreshold() == 0x00000555 );
//}

void MotorControlerTest::testGetMicroStepCount(){
  unsigned int expectedCount = testWordFromSpiAddress( _motorControler.getID(),
						       IDX_MICRO_STEP_COUNT);  
  BOOST_CHECK( _motorControler.getMicroStepCount() == expectedCount );
}

void MotorControlerTest::testSetMicroStepCount(){
  _motorControler.setMicroStepCount( 0xAAAAAAAA );
  BOOST_CHECK( _motorControler.getMicroStepCount() == 0x00AAAAAA );
}

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
