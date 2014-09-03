#define BOOST_TEST_MODULE testCalculateRegisterValues
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "RegisterValuesCalculator.h"

BOOST_AUTO_TEST_SUITE( CalculateRegisterValuesTestSuite )

BOOST_AUTO_TEST_CASE( testCalculateRegisterValues ){
  // Very simple test: check that all values end up in the right 
  // bit patterns.
  // Just use numbers counting up from 1
  ParametersCalculator::TMC429Parameters
    tmc429Parameters( 1, //pulseDiv
		      2, //rampDiv
		      3, //aMax
		      4, //vMax
		      5, //pDiv
		      6, //pMul
		      7, //controllerMicroStepValue
		      8, //driverMicroStepValue
		      9, //currentScale
		      std::list<std::string>() ); // an empty list

  RegisterValuesCalculator::RegisterValues registerValues=
    RegisterValuesCalculator::calculateRegisterValues( tmc429Parameters,
	                          RegisterValuesCalculator::IGNORE_BOTH);

  BOOST_CHECK( registerValues.driverControlData == 8);
  BOOST_CHECK( registerValues.stallGuardControlData == 0x10009);
  BOOST_CHECK( registerValues.referenceConfigAndRampModeData == 
	       RegisterValuesCalculator::IGNORE_BOTH );
  // proportionalityFactorData is 0xXX0Y with XX = pmul and y= pdiv
  BOOST_CHECK( registerValues.proportionalityFactorData == 0x605);
  // dividersAndMicroStepResolutionData is 0xXY0Z with 
  // X=PulseDiv, Y=RampDiv, Z=controllerMicroStepValue
  BOOST_CHECK( registerValues.dividersAndMicroStepResolutionData == 0x1207);
  BOOST_CHECK( registerValues.maximumVelocity == 4);
  BOOST_CHECK( registerValues.maximumAcceleration == 3);
}

BOOST_AUTO_TEST_SUITE_END()
