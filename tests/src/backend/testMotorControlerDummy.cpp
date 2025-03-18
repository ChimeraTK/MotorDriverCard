#include <boost/test/included/unit_test.hpp>

#include <sstream>
using namespace boost::unit_test_framework;

#include "MotorControlerDummy.h"

#include <ChimeraTK/Exception.h>

using namespace mtca4u;

// A macro to declare tests for an unimplemented get/set pair.
// It is checked that the MotorDriverException is thrown
#define DECLARE_GET_SET_THROW_TEST(NAME)                                                                               \
  void testGetSet##NAME() {                                                                                            \
    BOOST_CHECK_THROW(_motorControlerDummy.get##NAME(), ChimeraTK::logic_error);                                       \
    BOOST_CHECK_THROW(_motorControlerDummy.set##NAME(0), ChimeraTK::logic_error);                                      \
  }

static const unsigned int THE_ID = 17;

class MotorControlerDummyTest {
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

  void testReferenceSwitchData();

  DECLARE_GET_SET_THROW_TEST(MinimumVelocity)
  DECLARE_GET_SET_THROW_TEST(MaximumVelocity)
  DECLARE_GET_SET_THROW_TEST(TargetVelocity)
  DECLARE_GET_SET_THROW_TEST(MaximumAcceleration)
  DECLARE_GET_SET_THROW_TEST(PositionTolerance)
  DECLARE_GET_SET_THROW_TEST(PositionLatched)

  void testMoveTowardsTarget();

 private:
  MotorControlerDummy _motorControlerDummy;
};

class MotorControlerDummyTestSuite : public test_suite {
 public:
  MotorControlerDummyTestSuite() : test_suite("MotorControlerDummy test suite") {
    // create an instance of the test class
    boost::shared_ptr<MotorControlerDummyTest> motorControlerDummyTest(new MotorControlerDummyTest);

    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetID, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testIsSetEnabled, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetSetTargetPosition, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetSetActualPosition, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetSetActualVelocity, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetSetActualAcceleration, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetSetMicroStepCount, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetStatus, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetSetDecoderReadoutMode, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetSetMinimumVelocity, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetSetMaximumVelocity, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetSetTargetVelocity, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetSetMaximumAcceleration, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetSetPositionTolerance, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetSetPositionLatched, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testGetDecoderPosition, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testReferenceSwitchData, motorControlerDummyTest));
    add(BOOST_CLASS_TEST_CASE(&MotorControlerDummyTest::testMoveTowardsTarget, motorControlerDummyTest));
  }
};

test_suite* init_unit_test_suite(int /*argc*/, char* /*argv*/[]) {
  framework::master_test_suite().p_name.value = "MotorControlerDummy test suite";
  return new MotorControlerDummyTestSuite;
}

MotorControlerDummyTest::MotorControlerDummyTest() : _motorControlerDummy(THE_ID) {}

void MotorControlerDummyTest::testGetID() {
  BOOST_CHECK(_motorControlerDummy.getID() == THE_ID);
}

void MotorControlerDummyTest::testIsSetEnabled() {
  // motor should be disabled now
  BOOST_CHECK(_motorControlerDummy.isEnabled() == false);
  // just a small test that the motor does not move
  // move to somewhere way beyond the end swith. The current position should go
  // there.
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 10000);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 0);

  _motorControlerDummy.setTargetPosition(2000000);
  // motor must not be moving
  BOOST_CHECK(_motorControlerDummy.getActualVelocity() == 0);
  _motorControlerDummy.moveTowardsTarget(1);
  // real position has not changed, target positon reached
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 10000);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 2000000);

  // reset the positons and enable the motor. All other tests are done with
  // motor activated.
  _motorControlerDummy.setTargetPosition(0);
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setEnabled(true);
  // check that on/of works as expected
  BOOST_CHECK(_motorControlerDummy.isEnabled());
  _motorControlerDummy.setEnabled(false);
  BOOST_CHECK(_motorControlerDummy.isEnabled() == false);
  _motorControlerDummy.setEnabled(true);
}

void MotorControlerDummyTest::testGetSetTargetPosition() {
  // upon initialisation all positions must be 0
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 0);
  BOOST_CHECK(_motorControlerDummy.targetPositionReached());
  _motorControlerDummy.setTargetPosition(1234);
  BOOST_CHECK(_motorControlerDummy.targetPositionReached() == false);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 1234);
  // set it back to 0 to simplify further tests
  _motorControlerDummy.setTargetPosition(0);
  BOOST_CHECK(_motorControlerDummy.targetPositionReached());
}

void MotorControlerDummyTest::testGetSetActualPosition() {
  // upon initialisation all positions must be 0
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 0);
  _motorControlerDummy.setActualPosition(1000);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 1000);
}

void MotorControlerDummyTest::testGetSetActualVelocity() {
  // we strongly depent on the order of the execution here. actual position is
  // 1000, while the target position is 0, to we expect a negative velocity
  BOOST_CHECK(_motorControlerDummy.getActualVelocity() == -25);

  _motorControlerDummy.setActualPosition(0);
  BOOST_CHECK(_motorControlerDummy.getActualVelocity() == 0);

  _motorControlerDummy.setActualPosition(-1000);
  BOOST_CHECK(_motorControlerDummy.getActualVelocity() == 25);

  // whitebox test: as the decision in getActualVelocity() directly depends on
  // the private isMoving, this is the ideal case to test it.

  // move to the positive end switch. Although the target positon is not reached
  // the motor must not move any more
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setTargetPosition(200000);
  BOOST_CHECK(_motorControlerDummy.getActualVelocity() == 25);
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControlerDummy.getActualVelocity() == 0);

  _motorControlerDummy.setTargetPosition(-200000);
  BOOST_CHECK(_motorControlerDummy.getActualVelocity() == -25);
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControlerDummy.getActualVelocity() == 0);

  BOOST_CHECK_THROW(_motorControlerDummy.setActualVelocity(0), ChimeraTK::logic_error);
}

void MotorControlerDummyTest::testGetSetActualAcceleration() {
  BOOST_CHECK(_motorControlerDummy.getActualAcceleration() == 0);
  BOOST_CHECK_THROW(_motorControlerDummy.setActualAcceleration(0), ChimeraTK::logic_error);
}

void MotorControlerDummyTest::testGetSetMicroStepCount() {
  // Set the actual position to something positive. The micro step count is
  // positive and identical to the actual count.
  // Does this make sense? Check the data sheet.
  _motorControlerDummy.setActualPosition(5000);
  BOOST_CHECK(_motorControlerDummy.getMicroStepCount() == 543);
  BOOST_CHECK_THROW(_motorControlerDummy.setMicroStepCount(0), ChimeraTK::logic_error);
}

void MotorControlerDummyTest::testGetStatus() {
  DriverStatusData referenceStatusData;
  referenceStatusData.setStandstillIndicator(1); // reference when motor is not moving

  // we are at the negative end switch. The motor should not be moving.
  BOOST_CHECK(_motorControlerDummy.getStatus() == referenceStatusData);

  // the actual position is 5000. Move away from it
  _motorControlerDummy.setTargetPosition(6000);
  // motor is moving, now all bits should be 0
  BOOST_CHECK(_motorControlerDummy.getStatus() == 0);
}

void MotorControlerDummyTest::testGetSetDecoderReadoutMode() {
  BOOST_CHECK(_motorControlerDummy.getDecoderReadoutMode() == MotorControler::DecoderReadoutMode::HEIDENHAIN);
  BOOST_CHECK_THROW(_motorControlerDummy.setDecoderReadoutMode(MotorControler::DecoderReadoutMode::INCREMENTAL),
      ChimeraTK::logic_error);
}

void MotorControlerDummyTest::testGetDecoderPosition() {
  // we are still at the negative end switch
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 0);
  // actual position is 5000, move into the absolute positive range
  _motorControlerDummy.setTargetPosition(17000); // 12000 steps
  _motorControlerDummy.moveTowardsTarget(1);

  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 12000);
}

void MotorControlerDummyTest::testReferenceSwitchData() {
  MotorReferenceSwitchData motorReferenceSwitchData = _motorControlerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() == false);
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchBit() == 0);

  // move to the positive end switch and recheck
  _motorControlerDummy.setTargetPosition(_motorControlerDummy.getActualPosition() + 2000000);
  _motorControlerDummy.moveTowardsTarget(1);
  motorReferenceSwitchData = _motorControlerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() == false);
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchBit() == 1);

  // deactivate the positive end switch. Although it is still active in
  // hardware, this should not be reported
  _motorControlerDummy.setPositiveReferenceSwitchEnabled(false);
  motorReferenceSwitchData = _motorControlerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() == false);
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchBit() == 0);

  // reenable the switch and to the same for the negative switch
  _motorControlerDummy.setPositiveReferenceSwitchEnabled(true);
  _motorControlerDummy.setTargetPosition(_motorControlerDummy.getActualPosition() - 2000000);
  _motorControlerDummy.moveTowardsTarget(1);
  motorReferenceSwitchData = _motorControlerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive());
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchBit() == 1);

  // deactivate the positive end switch. Although it is still active in
  // hardware, this should not be reported
  _motorControlerDummy.setNegativeReferenceSwitchEnabled(false);
  motorReferenceSwitchData = _motorControlerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled() == false);
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() == false);
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchBit() == 0);

  // reenable the switch and to the same for the negative switch
  _motorControlerDummy.setNegativeReferenceSwitchEnabled(true);
}

void MotorControlerDummyTest::testMoveTowardsTarget() {
  // ok, we are at the negative end switch. reset target and actual position to
  // 0 for a clean test environment.
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setTargetPosition(0);

  // move to a target position in the range in multiple steps
  _motorControlerDummy.setTargetPosition(4000);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 4000);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 0);

  _motorControlerDummy.moveTowardsTarget(0.25);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 1000);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 4000);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 1000);

  _motorControlerDummy.moveTowardsTarget(0.5);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 2500);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 4000);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 2500);

  // moving backwards works differently, this does nothing
  _motorControlerDummy.moveTowardsTarget(-0.25);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 2500);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 4000);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 2500);

  // moving further than the target position also does not work
  _motorControlerDummy.moveTowardsTarget(2);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 4000);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 4000);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 4000);

  // we are already there. Moving again should not do anything
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 4000);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 4000);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 4000);

  // ok, let's recalibrate and do the stuff backwards
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setTargetPosition(-400); // we intentionally do not move all the
  // way back to the end switch
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == -400);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 4000);

  // this moves backwards because the target position is smaller than the actual
  // one
  _motorControlerDummy.moveTowardsTarget(0.25);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == -100);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == -400);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 3900);

  _motorControlerDummy.moveTowardsTarget(0.5);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == -250);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == -400);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 3750);

  // moving backwards works differently, this does nothing
  _motorControlerDummy.moveTowardsTarget(-0.25);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == -250);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == -400);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 3750);

  // moving further than the target position also does not work
  _motorControlerDummy.moveTowardsTarget(2);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == -400);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == -400);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 3600);

  // now check that the end switches are handled correctly
  // move to the middle and recalibrate for easier readability

  _motorControlerDummy.setTargetPosition(6000);
  _motorControlerDummy.moveTowardsTarget(1);
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setTargetPosition(0);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 0);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 10000);

  // set a target position outside the end switches and see what happes
  _motorControlerDummy.setTargetPosition(11000);
  // only move half the way, should be fine
  _motorControlerDummy.moveTowardsTarget(0.5);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 5500);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 11000);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 15500);
  // move the rest, it should stop at the end switch and not be moving any more
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 10000);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 11000);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 20000);
  BOOST_CHECK(_motorControlerDummy.getActualVelocity() == 0);
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchData().getPositiveSwitchActive());

  // Move back, deactivate the end switch and try again
  // (Don't try this at home. It will break your hardware!)
  _motorControlerDummy.setTargetPosition(5500);
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 5500);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 5500);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 15500);

  _motorControlerDummy.setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy.setTargetPosition(11000);
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 11000);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 11000);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 21000);

  // now the same for the negative end switch
  // set a target position outside the end switches and see what happes
  _motorControlerDummy.setTargetPosition(-11000);
  // only move half the way, should be fine
  _motorControlerDummy.moveTowardsTarget(0.5);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == -11000);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 10000);
  // move the rest, it should stop at the end switch and not be moving any more
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == -10000);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == -11000);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 0);
  BOOST_CHECK(_motorControlerDummy.getActualVelocity() == 0);
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchData().getNegativeSwitchActive());

  // Move back, deactivate the end switch and try again
  // (Don't try this at home. It will break your hardware!)
  _motorControlerDummy.setTargetPosition(-5500);
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == -5500);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == -5500);
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == 4500);

  _motorControlerDummy.setNegativeReferenceSwitchEnabled(false);
  _motorControlerDummy.setTargetPosition(-11000);
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == -11000);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == -11000);
  // the decoder delivers a huge value because it is unsigned. We broke the
  // hardware anyway ;-) Negative positons are not possible.
  BOOST_CHECK(_motorControlerDummy.getDecoderPosition() == static_cast<unsigned int>(-1000));

  // test blocking of movement
  // reset positions and disable end switches
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setTargetPosition(0);
  _motorControlerDummy.setNegativeReferenceSwitchEnabled(false);
  _motorControlerDummy.setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy.setEnabled(true);
  // try to move, but enable blocking
  _motorControlerDummy.setTargetPosition(1000);
  _motorControlerDummy.moveTowardsTarget(1, true);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 1000);
  BOOST_CHECK(_motorControlerDummy.getStatus().getStandstillIndicator() == 1);

  // try to move, but enable blocking
  _motorControlerDummy.setTargetPosition(-1000);
  _motorControlerDummy.moveTowardsTarget(1, true);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == -1000);
  BOOST_CHECK(_motorControlerDummy.getStatus().getStandstillIndicator() == 1);

  // try to move, but disable blocking
  _motorControlerDummy.setTargetPosition(1000);
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControlerDummy.getActualPosition() == 1000);
  BOOST_CHECK(_motorControlerDummy.getTargetPosition() == 1000);
  _motorControlerDummy.setEnabled(false);
}
