#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "MotorDriverCardImpl.h"
using namespace mtca4u;

class MotorDriverCardTest{
public:
  MotorDriverCardTest(){}
};

class  MotorDriverCardTestSuite : public test_suite{
 public:
  MotorDriverCardTestSuite() : test_suite(" MultiVariableWord test suite"){
    boost::shared_ptr<MotorDriverCardTest> motorDriverCardTest( new MotorDriverCardTest );
    
    
  }
};

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
   framework::master_test_suite().p_name.value = "MotorDriverCard test suite";

  return new MotorDriverCardTestSuite;
}
