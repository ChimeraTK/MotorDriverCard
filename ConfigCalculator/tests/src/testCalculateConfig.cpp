#define BOOST_TEST_MODULE testCalculateConfig
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "ConfigCalculator.h"

#include <stdexcept>

BOOST_AUTO_TEST_SUITE( CalculateConfigTestSuite )

BOOST_AUTO_TEST_CASE( testCalculateConfig ){
  // Very simple test: check that all values end up in the right 
  // bit patterns.
  // Just use numbers counting up from 1
  ParametersCalculator::TMC429Parameters
    tmc429Parameters( 1, //pulseDiv
		      2, //rampDiv
		      3, //aMax
		      4, //vMax
		      5, //pDiv
		      0x86, //pMul has to be between 128 and 255
		      7, //controllerMicroStepValue
		      8, //driverMicroStepValue
		      9, //currentScale
		      std::list<std::string>() ); // an empty list

  mtca4u::MotorControlerConfig controllerConfig =
    ConfigCalculator::calculateConfig( tmc429Parameters,
				       ConfigCalculator::IGNORE_BOTH);

  BOOST_CHECK(controllerConfig.driverControlData.getPayloadData() == 8);
  BOOST_CHECK(controllerConfig.stallGuardControlData.getPayloadData()
	      == 0x10009);
  BOOST_CHECK(controllerConfig.referenceConfigAndRampModeData.getDATA() == 
	      ConfigCalculator::IGNORE_BOTH );
  // proportionalityFactorData is 0xXX0Y with XX = pmul and y= pdiv
  BOOST_CHECK(controllerConfig.proportionalityFactorData.getDATA()
	      == 0x8605);
  std::cout << std::hex << ".proportionalityFactorData 0x"
	    << controllerConfig.proportionalityFactorData.getDATA()
	    << std::dec << std::endl;
  // dividersAndMicroStepResolutionData is 0xXY0Z with 
  // X=PulseDiv, Y=RampDiv, Z=controllerMicroStepValue
  BOOST_CHECK(controllerConfig.dividersAndMicroStepResolutionData.getDATA()
	      == 0x1207);
  BOOST_CHECK(controllerConfig.maximumVelocity == 4);
  BOOST_CHECK(controllerConfig.maximumAcceleration == 3);
}

BOOST_AUTO_TEST_CASE( testeEndSwitchConfig ){
  // Test that an exception is thrown if you write something invalid to
  // the end switch config enum via static_cast
  // Just use the same parameters as above
  ParametersCalculator::TMC429Parameters
    tmc429Parameters( 1, //pulseDiv
		      2, //rampDiv
		      3, //aMax
		      4, //vMax
		      5, //pDiv
		      0x86, //pMul has to be between 128 and 255
		      7, //controllerMicroStepValue
		      8, //driverMicroStepValue
		      9, //currentScale
		      std::list<std::string>() ); // an empty list

  BOOST_CHECK_NO_THROW(
    ConfigCalculator::calculateConfig( tmc429Parameters,
				       ConfigCalculator::IGNORE_BOTH) );
  BOOST_CHECK_NO_THROW(
    ConfigCalculator::calculateConfig( tmc429Parameters,
				       ConfigCalculator::IGNORE_POSITIVE) );
  BOOST_CHECK_NO_THROW(
    ConfigCalculator::calculateConfig( tmc429Parameters,
				       ConfigCalculator::IGNORE_NEGATIVE) );
  BOOST_CHECK_NO_THROW(
    ConfigCalculator::calculateConfig( tmc429Parameters,
				       ConfigCalculator::IGNORE_BOTH) );
  BOOST_CHECK_NO_THROW(
    ConfigCalculator::calculateConfig( tmc429Parameters,
				       ConfigCalculator::USE_BOTH) );

  BOOST_CHECK_THROW(
    ConfigCalculator::calculateConfig( tmc429Parameters,
      static_cast<ConfigCalculator::EndSwitchConfig>(5)),
    std::invalid_argument );
}

BOOST_AUTO_TEST_SUITE_END()
