#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE MotorDriverCardDummyTest

#include <boost/test/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "MotorControlerDummy.h"
#include "MotorDriverCardDummy.h"

#include <ChimeraTK/Exception.h>

namespace mtca4u {

  class MotorDriverCardDummyTest {
   protected:
    MotorDriverCardDummy _motorDriverCardDummy;
  };

  BOOST_FIXTURE_TEST_CASE(TestMotorControler, MotorDriverCardDummyTest) {
    boost::shared_ptr<MotorControler> controler0 = _motorDriverCardDummy.getMotorControler(0);

    BOOST_CHECK(controler0->getID() == 0);

    // check that we actually get a MotorControlerDummy
    boost::shared_ptr<MotorControlerDummy> controler1 =
        boost::dynamic_pointer_cast<MotorControlerDummy>(_motorDriverCardDummy.getMotorControler(1));

    // this will cause an exception if the cast failed
    BOOST_CHECK(controler1->getID() == 1);

    BOOST_CHECK_THROW(_motorDriverCardDummy.getMotorControler(2), ChimeraTK::logic_error);
  }

  BOOST_FIXTURE_TEST_CASE(TestGetPowerMonitor, MotorDriverCardDummyTest) {
    BOOST_CHECK_THROW(_motorDriverCardDummy.getPowerMonitor(), ChimeraTK::logic_error);
  }

} // namespace mtca4u
