#include <sstream>

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "MotorControlerDummy.h"
#include "MtcaMappedDevice/NotImplementedException.h"
using namespace mtca4u;

// A macro to declare tests for an unimplemented get/set pair.
// It is checked that the NotImplementedException is thrown
#define DECLARE_GET_SET_THROW_TEST( NAME )\
  void testGetSet ## NAME (){\
    BOOST_CHECK_THROW( _motorControlerDummy.get ## NAME (), NotImplementedException );\
    BOOST_CHECK_THROW( _motorControlerDummy.set ## NAME (0), NotImplementedException );}

static const unsigned int THE_ID = 17;

class MotorControlerDummyTest
{
 public:
  MotorControlerDummyTest();

  void testGetID();
  void testIsSetEnabled();
  void testGetSetTargetPosition();
  void testGetSetActualPosition();
  void testGetSetActualVelocity();
  void testGetSetActualAcceleration();
  void testGetSetMicroStepCount();

  void testGetStatus();
  void testGetSetDecoderReadoutMode();
  void testGetDecoderPosition();

  //  MotorReferenceSwitchData getReferenceSwitchData();

  //  void setPositiveReferenceSwitchEnabled(bool enableStatus);
  //  void setNegativeReferenceSwitchEnabled(bool enableStatus);

  DECLARE_GET_SET_THROW_TEST(MinimumVelocity)
  DECLARE_GET_SET_THROW_TEST(MaximumVelocity)
  DECLARE_GET_SET_THROW_TEST(TargetVelocity)
  DECLARE_GET_SET_THROW_TEST(MaximumAcceleration)
  DECLARE_GET_SET_THROW_TEST(PositionTolerance)
  DECLARE_GET_SET_THROW_TEST(PositionLatched)

  //  bool targetPositionReached();
  //  unsigned int getReferenceSwitchBit();
  //
  //  void moveTowardsTarget(float fraction);
  

 private:
  MotorControlerDummy _motorControlerDummy;
};

class MotorControlerDummyTestSuite : public test_suite {
public:
  MotorControlerDummyTestSuite(): test_suite("MotorControlerDummy test suite") {
    // create an instance of the test class
    boost::shared_ptr<MotorControlerDummyTest> motorControlerDummyTest( new MotorControlerDummyTest );

    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetID,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testIsSetEnabled,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetSetTargetPosition,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetSetActualPosition,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetSetActualVelocity,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetSetActualAcceleration,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetSetMicroStepCount,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetStatus,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetSetDecoderReadoutMode,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetSetMinimumVelocity,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetSetMaximumVelocity,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetSetTargetVelocity,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetSetMaximumAcceleration,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetSetPositionTolerance,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetSetPositionLatched,
				motorControlerDummyTest) );
    add( BOOST_CLASS_TEST_CASE( &MotorControlerDummyTest::testGetDecoderPosition,
				motorControlerDummyTest) );
  }
};

test_suite*
init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
  framework::master_test_suite().p_name.value = "MotorControlerDummy test suite";
  return new MotorControlerDummyTestSuite;
}

MotorControlerDummyTest::MotorControlerDummyTest()
  : _motorControlerDummy(THE_ID)
{}

void MotorControlerDummyTest::testGetID(){
  BOOST_CHECK( _motorControlerDummy.getID() == THE_ID );
}

void MotorControlerDummyTest::testIsSetEnabled(){
  // motor should be disabled now
  BOOST_CHECK(  _motorControlerDummy.isEnabled() == false );
  // just a small test that the motor does not move
  // move to somewhere way beyond the end swith. The current position should go there.
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 10000 );
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 0 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 0 );
  
  _motorControlerDummy.setTargetPosition(2000000);
  // motor must not be moving
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 0 ); 
  _motorControlerDummy.moveTowardsTarget(1);
  // real position has not changed, target positon reached
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 10000 );
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 2000000 );

  // reset the positons and enable the motor. All other tests are done with motor
  // activated.
  _motorControlerDummy.setTargetPosition(0);
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setEnabled();
  // check that on/of works as expected
  BOOST_CHECK(  _motorControlerDummy.isEnabled() );
  _motorControlerDummy.setEnabled(false);
  BOOST_CHECK(  _motorControlerDummy.isEnabled() == false );
  _motorControlerDummy.setEnabled();
  
}

void MotorControlerDummyTest::testGetSetTargetPosition(){
  // upon initialisation all positions must be 0
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 0);
  _motorControlerDummy.setTargetPosition(1234);
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 1234);
  // set it back to 0 to simplify further tests
  _motorControlerDummy.setTargetPosition(0);  
}

void MotorControlerDummyTest::testGetSetActualPosition(){
  // upon initialisation all positions must be 0
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 0);
  _motorControlerDummy.setActualPosition(1000);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 1000);
}

void MotorControlerDummyTest::testGetSetActualVelocity(){
  // we strongly depent on the order of the execution here. actual position is 1000,
  // while the target position is 0, to we expect a negative velocity
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == -25 );

  _motorControlerDummy.setActualPosition(0);
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 0 );

  _motorControlerDummy.setActualPosition(-1000);
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 25 );
 
  // whitebox test: as the decision in getActualVelocity() directly depends on
  // the private isMoving, this is the ideal case to test it.

  // move to the positive end switch. Although the target positon is not reached the
  // motor must not move any more
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setTargetPosition(200000);
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 25 );
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 0 );

  _motorControlerDummy.setTargetPosition(-200000);
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == -25 );
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 0 );  

  BOOST_CHECK_THROW(_motorControlerDummy.setActualVelocity(0), NotImplementedException);
}

void MotorControlerDummyTest::testGetSetActualAcceleration(){
  BOOST_CHECK( _motorControlerDummy.getActualAcceleration() == 0 );
  BOOST_CHECK_THROW(_motorControlerDummy.setActualAcceleration(0),
		    NotImplementedException);
}

void MotorControlerDummyTest::testGetSetMicroStepCount(){
  // Set the actual position to something positive. The micro step count is
  // positive and identical to the actual count.
  // Does this make sense? Check the data sheet. 
  _motorControlerDummy.setActualPosition(5000);
  BOOST_CHECK( _motorControlerDummy.getMicroStepCount() == 5000 );
  BOOST_CHECK_THROW(_motorControlerDummy.setMicroStepCount(0),
		    NotImplementedException);
}


void MotorControlerDummyTest::testGetStatus(){
  BOOST_CHECK_THROW(_motorControlerDummy.getStatus(), NotImplementedException);
}

void MotorControlerDummyTest::testGetSetDecoderReadoutMode(){
  BOOST_CHECK(_motorControlerDummy.getDecoderReadoutMode() ==
	      MotorControler::DecoderReadoutMode::HEIDENHAIN);
  BOOST_CHECK_THROW(_motorControlerDummy.setDecoderReadoutMode(MotorControler::DecoderReadoutMode::INCREMENTAL),
		    NotImplementedException);
}

void MotorControlerDummyTest::testGetDecoderPosition(){
  // we are still at the negative end switch
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 0 );
  // actual position is 5000, move into the absolute positive range
  _motorControlerDummy.setTargetPosition(17000); // 12000 steps
  _motorControlerDummy.moveTowardsTarget(1);
  
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 12000 );  
}
