#define BOOST_TEST_MODULE testCalculateParameters
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include <stdexcept>

#include "ParametersCalculator.h"

#define CHECK_MICROSTEPS( nMicrosteps,  driverValue,  controllerValue )	\
  physicalParameters.microsteps = nMicrosteps;\
  { ParametersCalculator::TMC429Parameters tmc429Parameters		\
      = ParametersCalculator::calculateParameters( physicalParameters );\
  BOOST_CHECK( tmc429Parameters.driverMicroStepValue == driverValue);\
  BOOST_CHECK( tmc429Parameters.controllerMicroStepValue == controllerValue);}


BOOST_AUTO_TEST_SUITE( CalculateParametersTestSuite )

// The parameters as used for the VT21 are used to check the calculated
// values as a random sample
BOOST_AUTO_TEST_CASE( testVT21Parameters ){
  ParametersCalculator::PhysicalParameters 
    physicalParameters( 32, //MHz system clock
			16, // microsteps
			200, //steps per turn
			200, // maxRPM
			0.5, // s to vMax
			0.24); // iMax

  ParametersCalculator::TMC429Parameters tmc429Parameters
    = ParametersCalculator::calculateParameters( physicalParameters );

  BOOST_CHECK( tmc429Parameters.pulseDiv == 6);
  BOOST_CHECK( tmc429Parameters.rampDiv == 11);
  BOOST_CHECK( tmc429Parameters.vMax == 1398);
  BOOST_CHECK( tmc429Parameters.aMax == 1466);
  BOOST_CHECK( tmc429Parameters.pDiv == 6);
  BOOST_CHECK( tmc429Parameters.pMul == 174);
  BOOST_CHECK( tmc429Parameters.controllerMicroStepValue == 4);
  BOOST_CHECK( tmc429Parameters.driverMicroStepValue == 4);
  BOOST_CHECK( tmc429Parameters.currentScale == 4);
}

BOOST_AUTO_TEST_CASE( testMirosteps ){
  ParametersCalculator::PhysicalParameters 
    physicalParameters( 32, //MHz system clock
			1, // 1 microsteps = fullstep
			200, //steps per turn
			200, // maxRPM
			0.5, // s to vMax
			0.24); // iMax

  CHECK_MICROSTEPS( 1, 8, 0 );
  CHECK_MICROSTEPS( 2, 7, 1 );
  CHECK_MICROSTEPS( 4, 6, 2 );
  CHECK_MICROSTEPS( 8, 5, 3 );
  CHECK_MICROSTEPS( 16, 4, 4 );
  CHECK_MICROSTEPS( 32, 3, 5 );
  CHECK_MICROSTEPS( 64, 2, 6 );

  physicalParameters.microsteps = 12.5;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( physicalParameters), 
		     std::invalid_argument );

  physicalParameters.microsteps = 12;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( physicalParameters), 
		     std::invalid_argument );
 
}
    

BOOST_AUTO_TEST_SUITE_END()
