#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE MotorControllerDummyTest

#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "MotorControllerDummy.h"

#include <ChimeraTK/Exception.h>
using namespace ChimeraTK::MotorDriver;

using namespace ChimeraTK;

// A macro to declare tests for an unimplemented get/set pair.
// It is checked that the MotorDriverException is thrown
#define DECLARE_GET_SET_THROW_TEST(NAME)                                                                               \
  BOOST_FIXTURE_TEST_CASE(TestGetSet##NAME, MotorControllerDummyTest) {                                                \
    BOOST_CHECK_THROW(_motorControllerDummy.get##NAME(), ChimeraTK::logic_error);                                      \
    BOOST_CHECK_THROW(_motorControllerDummy.set##NAME(0), ChimeraTK::logic_error);                                     \
  }

static const unsigned int THE_ID = 17;

class MotorControllerDummyTest {
 protected:
  static MotorControllerDummy _motorControllerDummy;
};

MotorControllerDummy MotorControllerDummyTest::_motorControllerDummy(THE_ID);

BOOST_FIXTURE_TEST_CASE(TestGetId, MotorControllerDummyTest) {
  BOOST_CHECK(_motorControllerDummy.getID() == THE_ID);
}

BOOST_FIXTURE_TEST_CASE(TestIsSetEnabled, MotorControllerDummyTest) {
  // motor should be disabled now
  BOOST_CHECK(_motorControllerDummy.isEnabled() == false);
  // just a small test that the motor does not move
  // move to somewhere way beyond the end swith. The current position should go
  // there.
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 10000);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 0);

  _motorControllerDummy.setTargetPosition(2000000);
  // motor must not be moving
  BOOST_CHECK(_motorControllerDummy.getActualVelocity() == 0);
  _motorControllerDummy.moveTowardsTarget(1);
  // real position has not changed, target position reached
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 10000);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 2000000);

  // reset the positons and enable the motor. All other tests are done with
  // motor activated.
  _motorControllerDummy.setTargetPosition(0);
  _motorControllerDummy.setActualPosition(0);
  _motorControllerDummy.setEnabled(true);
  // check that on/of works as expected
  BOOST_CHECK(_motorControllerDummy.isEnabled());
  _motorControllerDummy.setEnabled(false);
  BOOST_CHECK(_motorControllerDummy.isEnabled() == false);
  _motorControllerDummy.setEnabled(true);
}

BOOST_FIXTURE_TEST_CASE(TestGetSetTargetPosition, MotorControllerDummyTest) {
  // upon initialisation all positions must be 0
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 0);
  BOOST_CHECK(_motorControllerDummy.targetPositionReached());
  _motorControllerDummy.setTargetPosition(1234);
  BOOST_CHECK(_motorControllerDummy.targetPositionReached() == false);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 1234);
  // set it back to 0 to simplify further tests
  _motorControllerDummy.setTargetPosition(0);
  BOOST_CHECK(_motorControllerDummy.targetPositionReached());
}

BOOST_FIXTURE_TEST_CASE(TestGetSetActualPosition, MotorControllerDummyTest) {
  // upon initialisation all positions must be 0
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 0);
  _motorControllerDummy.setActualPosition(1000);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 1000);
}

BOOST_FIXTURE_TEST_CASE(TestGetSetActualVelocity, MotorControllerDummyTest) {
  // we strongly depent on the order of the execution here. actual position is
  // 1000, while the target position is 0, to we expect a negative velocity
  BOOST_CHECK(_motorControllerDummy.getActualVelocity() == -25);

  _motorControllerDummy.setActualPosition(0);
  BOOST_CHECK(_motorControllerDummy.getActualVelocity() == 0);

  _motorControllerDummy.setActualPosition(-1000);
  BOOST_CHECK(_motorControllerDummy.getActualVelocity() == 25);

  // whitebox test: as the decision in getActualVelocity() directly depends on
  // the private isMoving, this is the ideal case to test it.

  // move to the positive end switch. Although the target position is not reached
  // the motor must not move any more
  _motorControllerDummy.setActualPosition(0);
  _motorControllerDummy.setTargetPosition(200000);
  BOOST_CHECK(_motorControllerDummy.getActualVelocity() == 25);
  _motorControllerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControllerDummy.getActualVelocity() == 0);

  _motorControllerDummy.setTargetPosition(-200000);
  BOOST_CHECK(_motorControllerDummy.getActualVelocity() == -25);
  _motorControllerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControllerDummy.getActualVelocity() == 0);

  BOOST_CHECK_THROW(_motorControllerDummy.setActualVelocity(0), ChimeraTK::logic_error);
}

BOOST_FIXTURE_TEST_CASE(TestGetSetActualAcceleration, MotorControllerDummyTest) {
  BOOST_CHECK(_motorControllerDummy.getActualAcceleration() == 0);
  BOOST_CHECK_THROW(_motorControllerDummy.setActualAcceleration(0), ChimeraTK::logic_error);
}

BOOST_FIXTURE_TEST_CASE(TestGetSetMicroStepCount, MotorControllerDummyTest) {
  // Set the actual position to something positive. The micro step count is
  // positive and identical to the actual count.
  // Does this make sense? Check the data sheet.
  _motorControllerDummy.setActualPosition(5000);
  BOOST_CHECK(_motorControllerDummy.getMicroStepCount() == 543);
  BOOST_CHECK_THROW(_motorControllerDummy.setMicroStepCount(0), ChimeraTK::logic_error);
}

BOOST_FIXTURE_TEST_CASE(TestGetStatus, MotorControllerDummyTest) {
  DriverStatusData referenceStatusData;
  referenceStatusData.setStandstillIndicator(1); // reference when motor is not moving

  // we are at the negative end switch. The motor should not be moving.
  BOOST_CHECK(_motorControllerDummy.getStatus() == referenceStatusData);

  // the actual position is 5000. Move away from it
  _motorControllerDummy.setTargetPosition(6000);
  // motor is moving, now all bits should be 0
  BOOST_CHECK(_motorControllerDummy.getStatus() == 0);
}

BOOST_FIXTURE_TEST_CASE(TestGetSetDecoderReadoutMode, MotorControllerDummyTest) {
  BOOST_CHECK(_motorControllerDummy.getDecoderReadoutMode() == MotorController::DecoderReadoutMode::HEIDENHAIN);
  BOOST_CHECK_THROW(_motorControllerDummy.setDecoderReadoutMode(MotorController::DecoderReadoutMode::INCREMENTAL),
      ChimeraTK::logic_error);
}

BOOST_FIXTURE_TEST_CASE(TestGetDecoderPosition, MotorControllerDummyTest) {
  // we are still at the negative end switch
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 0);
  // actual position is 5000, move into the absolute positive range
  _motorControllerDummy.setTargetPosition(17000); // 12000 steps
  _motorControllerDummy.moveTowardsTarget(1);

  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 12000);
}

BOOST_FIXTURE_TEST_CASE(TestReferenceSwitchData, MotorControllerDummyTest) {
  MotorReferenceSwitchData motorReferenceSwitchData = _motorControllerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() == false);
  BOOST_CHECK(_motorControllerDummy.getReferenceSwitchBit() == 0);

  // move to the positive end switch and recheck
  _motorControllerDummy.setTargetPosition(_motorControllerDummy.getActualPosition() + 2000000);
  _motorControllerDummy.moveTowardsTarget(1);
  motorReferenceSwitchData = _motorControllerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() == false);
  BOOST_CHECK(_motorControllerDummy.getReferenceSwitchBit() == 1);

  // deactivate the positive end switch. Although it is still active in
  // hardware, this should not be reported
  _motorControllerDummy.setPositiveReferenceSwitchEnabled(false);
  motorReferenceSwitchData = _motorControllerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() == false);
  BOOST_CHECK(_motorControllerDummy.getReferenceSwitchBit() == 0);

  // reenable the switch and to the same for the negative switch
  _motorControllerDummy.setPositiveReferenceSwitchEnabled(true);
  _motorControllerDummy.setTargetPosition(_motorControllerDummy.getActualPosition() - 2000000);
  _motorControllerDummy.moveTowardsTarget(1);
  motorReferenceSwitchData = _motorControllerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive());
  BOOST_CHECK(_motorControllerDummy.getReferenceSwitchBit() == 1);

  // deactivate the positive end switch. Although it is still active in
  // hardware, this should not be reported
  _motorControllerDummy.setNegativeReferenceSwitchEnabled(false);
  motorReferenceSwitchData = _motorControllerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled() == false);
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() == false);
  BOOST_CHECK(_motorControllerDummy.getReferenceSwitchBit() == 0);

  // reenable the switch and to the same for the negative switch
  _motorControllerDummy.setNegativeReferenceSwitchEnabled(true);
}

BOOST_FIXTURE_TEST_CASE(TestMoveTowardsTarget, MotorControllerDummyTest) {
  // ok, we are at the negative end switch. reset target and actual position to
  // 0 for a clean test environment.
  _motorControllerDummy.setActualPosition(0);
  _motorControllerDummy.setTargetPosition(0);

  // move to a target position in the range in multiple steps
  _motorControllerDummy.setTargetPosition(4000);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 4000);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 0);

  _motorControllerDummy.moveTowardsTarget(0.25);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 1000);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 4000);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 1000);

  _motorControllerDummy.moveTowardsTarget(0.5);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 2500);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 4000);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 2500);

  // moving backwards works differently, this does nothing
  _motorControllerDummy.moveTowardsTarget(-0.25);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 2500);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 4000);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 2500);

  // moving further than the target position also does not work
  _motorControllerDummy.moveTowardsTarget(2);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 4000);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 4000);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 4000);

  // we are already there. Moving again should not do anything
  _motorControllerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 4000);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 4000);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 4000);

  // ok, let's recalibrate and do the stuff backwards
  _motorControllerDummy.setActualPosition(0);
  _motorControllerDummy.setTargetPosition(-400); // we intentionally do not move all the
  // way back to the end switch
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == -400);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 4000);

  // this moves backwards because the target position is smaller than the actual
  // one
  _motorControllerDummy.moveTowardsTarget(0.25);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == -100);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == -400);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 3900);

  _motorControllerDummy.moveTowardsTarget(0.5);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == -250);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == -400);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 3750);

  // moving backwards works differently, this does nothing
  _motorControllerDummy.moveTowardsTarget(-0.25);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == -250);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == -400);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 3750);

  // moving further than the target position also does not work
  _motorControllerDummy.moveTowardsTarget(2);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == -400);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == -400);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 3600);

  // now check that the end switches are handled correctly
  // move to the middle and recalibrate for easier readability

  _motorControllerDummy.setTargetPosition(6000);
  _motorControllerDummy.moveTowardsTarget(1);
  _motorControllerDummy.setActualPosition(0);
  _motorControllerDummy.setTargetPosition(0);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 0);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 10000);

  // set a target position outside the end switches and see what happes
  _motorControllerDummy.setTargetPosition(11000);
  // only move half the way, should be fine
  _motorControllerDummy.moveTowardsTarget(0.5);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 5500);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 11000);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 15500);
  // move the rest, it should stop at the end switch and not be moving any more
  _motorControllerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 10000);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 11000);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 20000);
  BOOST_CHECK(_motorControllerDummy.getActualVelocity() == 0);
  BOOST_CHECK(_motorControllerDummy.getReferenceSwitchData().getPositiveSwitchActive());

  // Move back, deactivate the end switch and try again
  // (Don't try this at home. It will break your hardware!)
  _motorControllerDummy.setTargetPosition(5500);
  _motorControllerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 5500);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 5500);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 15500);

  _motorControllerDummy.setPositiveReferenceSwitchEnabled(false);
  _motorControllerDummy.setTargetPosition(11000);
  _motorControllerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 11000);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 11000);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 21000);

  // now the same for the negative end switch
  // set a target position outside the end switches and see what happes
  _motorControllerDummy.setTargetPosition(-11000);
  // only move half the way, should be fine
  _motorControllerDummy.moveTowardsTarget(0.5);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == -11000);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 10000);
  // move the rest, it should stop at the end switch and not be moving any more
  _motorControllerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == -10000);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == -11000);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 0);
  BOOST_CHECK(_motorControllerDummy.getActualVelocity() == 0);
  BOOST_CHECK(_motorControllerDummy.getReferenceSwitchData().getNegativeSwitchActive());

  // Move back, deactivate the end switch and try again
  // (Don't try this at home. It will break your hardware!)
  _motorControllerDummy.setTargetPosition(-5500);
  _motorControllerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == -5500);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == -5500);
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == 4500);

  _motorControllerDummy.setNegativeReferenceSwitchEnabled(false);
  _motorControllerDummy.setTargetPosition(-11000);
  _motorControllerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == -11000);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == -11000);
  // the decoder delivers a huge value because it is unsigned. We broke the
  // hardware anyway ;-) Negative positons are not possible.
  BOOST_CHECK(_motorControllerDummy.getDecoderPosition() == static_cast<unsigned int>(-1000));

  // test blocking of movement
  // reset positions and disable end switches
  _motorControllerDummy.setActualPosition(0);
  _motorControllerDummy.setTargetPosition(0);
  _motorControllerDummy.setNegativeReferenceSwitchEnabled(false);
  _motorControllerDummy.setPositiveReferenceSwitchEnabled(false);
  _motorControllerDummy.setEnabled(true);
  // try to move, but enable blocking
  _motorControllerDummy.setTargetPosition(1000);
  _motorControllerDummy.moveTowardsTarget(1, true);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 1000);
  BOOST_CHECK(_motorControllerDummy.getStatus().getStandstillIndicator() == 1);

  // try to move, but enable blocking
  _motorControllerDummy.setTargetPosition(-1000);
  _motorControllerDummy.moveTowardsTarget(1, true);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 0);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == -1000);
  BOOST_CHECK(_motorControllerDummy.getStatus().getStandstillIndicator() == 1);

  // try to move, but disable blocking
  _motorControllerDummy.setTargetPosition(1000);
  _motorControllerDummy.moveTowardsTarget(1);
  BOOST_CHECK(_motorControllerDummy.getActualPosition() == 1000);
  BOOST_CHECK(_motorControllerDummy.getTargetPosition() == 1000);
  _motorControllerDummy.setEnabled(false);
}

DECLARE_GET_SET_THROW_TEST(MinimumVelocity)
DECLARE_GET_SET_THROW_TEST(MaximumVelocity)
DECLARE_GET_SET_THROW_TEST(TargetVelocity)
DECLARE_GET_SET_THROW_TEST(MaximumAcceleration)
DECLARE_GET_SET_THROW_TEST(PositionTolerance)
DECLARE_GET_SET_THROW_TEST(PositionLatched)
