#include <sstream>

#include <boost/test/included/unit_test.hpp>
#include "testConfigConstants.h"
#include "LinearStepperMotor.h"
#include "MotorDriverCardFactory.h"

using namespace boost::unit_test_framework;

static const std::string stepperMotorDeviceName("DFMC_MD22");
static const std::string dmapFile("./dummies.dmap");

class FunctionalTests
{
 public:

  void testMotorObjCreation();


 private:

};

class FunctionalTestSuite : public test_suite {
public:
  FunctionalTestSuite() : test_suite("functional test suite") {
    mtca4u::MotorDriverCardFactory::setDeviceaccessDMapFilePath(dmapFile);
    // create an instance of the test class
    boost::shared_ptr<FunctionalTests> functionalTests(new FunctionalTests);
    add(BOOST_CLASS_TEST_CASE(&FunctionalTests::testMotorObjCreation, functionalTests));
  }
};

test_suite*
init_unit_test_suite(int /*argc*/, char* /*argv*/ []) {
    framework::master_test_suite().p_name.value = "functional test suite";
    return new FunctionalTestSuite;
}

void FunctionalTests::testMotorObjCreation() {

  mtca4u::StepperMotor Motor1(stepperMotorDeviceName, MODULE_NAME_0, 0,CONFIG_FILE) ;
  mtca4u::StepperMotor Motor2(stepperMotorDeviceName, MODULE_NAME_1, 0,CONFIG_FILE);

  float initialTargetPosition = Motor1.getTargetPosition();
  Motor1.setTargetPosition(200);
  BOOST_CHECK(Motor1.getTargetPosition() == initialTargetPosition + 200);

  initialTargetPosition = Motor2.getTargetPosition();
  Motor1.setTargetPosition(500);
  BOOST_CHECK(Motor1.getTargetPosition() == initialTargetPosition + 500);
}
