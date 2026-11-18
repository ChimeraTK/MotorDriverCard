#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE MotorDriverCardDummyTest

#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "MotorControllerDummy.h"
#include "MotorDriverCardDummy.h"

#include <ChimeraTK/Exception.h>
using namespace ChimeraTK::MotorDriver;

namespace ChimeraTK::MotorDriver {

  class MotorDriverCardDummyTest {
   protected:
    MotorDriverCardDummy _motorDriverCardDummy;
  };

  BOOST_FIXTURE_TEST_CASE(TestMotorController, MotorDriverCardDummyTest) {
    boost::shared_ptr<MotorController> controller0 = _motorDriverCardDummy.getMotorController(0);

    BOOST_CHECK(controller0->getID() == 0);

    // check that we actually get a MotorControllerDummy
    boost::shared_ptr<MotorControllerDummy> controller1 =
        boost::dynamic_pointer_cast<MotorControllerDummy>(_motorDriverCardDummy.getMotorController(1));

    // this will cause an exception if the cast failed
    BOOST_CHECK(controller1->getID() == 1);

    BOOST_CHECK_THROW(_motorDriverCardDummy.getMotorController(2), ChimeraTK::logic_error);
  }

  BOOST_FIXTURE_TEST_CASE(TestGetPowerMonitor, MotorDriverCardDummyTest) {
    BOOST_CHECK_THROW(_motorDriverCardDummy.getPowerMonitor(), ChimeraTK::logic_error);
  }

} // namespace ChimeraTK::MotorDriver
