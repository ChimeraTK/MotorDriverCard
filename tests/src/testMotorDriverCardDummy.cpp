#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "MotorDriverCardDummy.h"
#include "MotorControlerDummy.h"
#include "MotorDriverException.h"

namespace mtca4u{

class MotorDriverCardDummyTest
{
 public:
  void testGetMotorControler();
  void testGetPowerMonitor();

  MotorDriverCardDummyTest() :  _motorDriverCardDummy(){}

 private:
  MotorDriverCardDummy _motorDriverCardDummy;
};

class MotorDriverCardDummyTestSuite : public test_suite {
public:
  MotorDriverCardDummyTestSuite(): test_suite("MotorDriverCardDummy test suite") {
    // create an instance of the test class
    boost::shared_ptr<MotorDriverCardDummyTest> motorDriverCardTest( new MotorDriverCardDummyTest );

    add( BOOST_CLASS_TEST_CASE( &MotorDriverCardDummyTest::testGetMotorControler,
				motorDriverCardTest ) );

    add( BOOST_CLASS_TEST_CASE( &MotorDriverCardDummyTest::testGetPowerMonitor,
				motorDriverCardTest ) );
  }
};

void MotorDriverCardDummyTest::testGetMotorControler(){
  boost::shared_ptr<MotorControler> controler0 = _motorDriverCardDummy.getMotorControler(0);
  
  BOOST_CHECK( controler0->getID() == 0 );

  // check that we actually get a MotorControlerDummy
  boost::shared_ptr<MotorControlerDummy> controler1 = 
    boost::dynamic_pointer_cast<MotorControlerDummy>(_motorDriverCardDummy.getMotorControler(1));
 
  // this will cause an exception if the cast failed
  BOOST_CHECK( controler1->getID() == 1 );

  BOOST_CHECK_THROW( _motorDriverCardDummy.getMotorControler(2), MotorDriverException);
  
}

void MotorDriverCardDummyTest::testGetPowerMonitor(){
  BOOST_CHECK_THROW( _motorDriverCardDummy.getPowerMonitor(), MotorDriverException);
}

} //namespace mtca4u

test_suite*
init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
  framework::master_test_suite().p_name.value = "MotorDriverCardDummy test suite";
  return new mtca4u::MotorDriverCardDummyTestSuite;
}
