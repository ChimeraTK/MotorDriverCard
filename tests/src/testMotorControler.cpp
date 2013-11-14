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
  MotorControlerTest(MotorControler & motorControler);
  // getID() is tested in the MotorDriver card, where 
  // different ID are known. This test is for just one
  // Motor with one ID, which is now known to be ok.
  DECLARE_GET_SET_TEST( ActualPosition );
  DECLARE_GET_SET_TEST( ActualVelocity );
  DECLARE_GET_SET_TEST( ActualAcceleration );
  //  DECLARE_GET_SET_TEST( AccelerationThreshold );
  DECLARE_GET_SET_TEST( MicroStepCount );
  void testGetStallGuardValue(); // read only register
  void testGetCoolStepValue(); // read only register
  void testGetStauts(); // read only register
  
  void testIsEnabled();
  void testSetEnabled();

  DECLARE_GET_SET_TEST( DecoderReadoutMode );

  void testGetDecoderPosition();
  
private:
  MotorControler & _motorControler;
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
	       new MotorControlerTest( _motorDriverCard->getMotorControler( i ) ) );
      ADD_GET_SET_TEST( ActualPosition );
      ADD_GET_SET_TEST( ActualVelocity );
      ADD_GET_SET_TEST( ActualAcceleration );
      //      ADD_GET_SET_TEST( AccelerationThreshold );
      ADD_GET_SET_TEST( MicroStepCount );
    }
  }
};

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
   framework::master_test_suite().p_name.value = "MotorControler test suite";

   return new MotorControlerTestSuite(MAP_FILE_NAME);
}


MotorControlerTest::MotorControlerTest(MotorControler & motorControler) 
  : _motorControler(motorControler){
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

