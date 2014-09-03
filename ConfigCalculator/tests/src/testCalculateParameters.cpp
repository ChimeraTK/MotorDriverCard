#define BOOST_TEST_MODULE testCalculateParameters
// Only after defining the name include the unit test header.
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include <stdexcept>

#include "ParametersCalculator.h"

// just for debugging the test
void printWarnings( std::list<std::string> const & warnings ){
  std::cout << "Warnings: " <<std::endl;
  for( std::list<std::string>::const_iterator it = warnings.begin();
       it != warnings.end(); ++it){
    std::cout << *it << std::endl;
  }
}

#define CHECK_MICROSTEPS( nMicrosteps,  driverValue,  controllerValue )	\
  physicalParameters.microsteps = nMicrosteps;\
  { ParametersCalculator::TMC429Parameters tmc429Parameters		\
      = ParametersCalculator::calculateParameters( physicalParameters );\
  BOOST_CHECK( tmc429Parameters.driverMicroStepValue == driverValue);\
  BOOST_CHECK( tmc429Parameters.controllerMicroStepValue == controllerValue);}

// ough, a global variable. But it saves tons of tying

// The parameters as used for the VT21 are used to check the calculated
// values as a random sample
const ParametersCalculator::PhysicalParameters 
  vt21Parameters( 32, //MHz system clock
		  16, // microsteps
		  200, //steps per turn
		  200, // maxRPM
		  0.5, // s to vMax
		  0.24); // iMax

BOOST_AUTO_TEST_SUITE( CalculateParametersTestSuite )

// The parameters as used for the VT21 are used to check the calculated
// values as a random sample
BOOST_AUTO_TEST_CASE( testVT21Parameters ){
  ParametersCalculator::TMC429Parameters tmc429Parameters
    = ParametersCalculator::calculateParameters( vt21Parameters );

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
  ParametersCalculator::PhysicalParameters physicalParameters = 
    vt21Parameters;

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
    
BOOST_AUTO_TEST_CASE( testClocks ){
  ParametersCalculator::PhysicalParameters physicalParameters = 
    vt21Parameters;

  physicalParameters.systemClock = 33;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( physicalParameters), 
		     std::invalid_argument );
  
  physicalParameters.systemClock = 0.5;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( physicalParameters), 
		     std::invalid_argument );

    
}

BOOST_AUTO_TEST_CASE( testCurrents ){
  ParametersCalculator::PhysicalParameters physicalParameters = 
    vt21Parameters;

  physicalParameters.iMax = 2;
  ParametersCalculator::TMC429Parameters tmc429Parameters
    = ParametersCalculator::calculateParameters( physicalParameters );
  BOOST_CHECK( tmc429Parameters.currentScale == 31 );
  BOOST_CHECK( tmc429Parameters.warnings.size() == 1 );

  physicalParameters.iMax = 0.001;
  tmc429Parameters = 
    ParametersCalculator::calculateParameters( physicalParameters );
  BOOST_CHECK( tmc429Parameters.currentScale == 0 );
  BOOST_CHECK( tmc429Parameters.warnings.size() == 0 );

  physicalParameters.iMax = 0;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( physicalParameters), 
		     std::invalid_argument );
  
  physicalParameters.iMax = -1;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( physicalParameters), 
		     std::invalid_argument );   
}

// the other input parameters must be positive
BOOST_AUTO_TEST_CASE( testPositive ){
  ParametersCalculator::PhysicalParameters physicalParameters = 
    vt21Parameters;

  physicalParameters.maxRPM = 0;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( physicalParameters), 
		     std::invalid_argument );
  physicalParameters.maxRPM = -1;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( physicalParameters), 
		     std::invalid_argument );   

  // reset to good parameters, only modify one parameter at a time
  physicalParameters = vt21Parameters;
  physicalParameters.nStepsPerTurn = 0;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( physicalParameters), 
		     std::invalid_argument );
  physicalParameters.nStepsPerTurn = -1;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( physicalParameters), 
		     std::invalid_argument );

  physicalParameters = vt21Parameters;
  physicalParameters.timeToVMax = 0;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( physicalParameters), 
		     std::invalid_argument );
  physicalParameters.timeToVMax = -1;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( physicalParameters), 
		     std::invalid_argument );
}

BOOST_AUTO_TEST_CASE( testPluseDivVMaxRanges ){
  ParametersCalculator::PhysicalParameters 
    maxPulseDivParameters( 32, // max system clock
			   1, // min microsteps
			   200, 
			   4, // very low maxRPM
			   0.2, // adjusted so other parameters stay in range
			   0.24); 
  // should lead to pulse div = 16, but the max is 13

  ParametersCalculator::TMC429Parameters tmc429Parameters
    = ParametersCalculator::calculateParameters( maxPulseDivParameters );
  BOOST_CHECK( tmc429Parameters.pulseDiv == 13 );
  BOOST_CHECK( tmc429Parameters.warnings.size() == 1 );
  
  // check that pulseDiv = 13 can be reached without warning
  ParametersCalculator::PhysicalParameters 
    largePulseDivParameters = vt21Parameters;
  largePulseDivParameters.maxRPM=2;
  largePulseDivParameters.timeToVMax=1.8;

  tmc429Parameters
    = ParametersCalculator::calculateParameters( largePulseDivParameters );
  BOOST_CHECK( tmc429Parameters.pulseDiv == 13 );
  BOOST_CHECK( tmc429Parameters.warnings.size() == 0 );

  // check with settings where the not rounded
  // value of pulseDiv is ]0..-1[, so the special case
  // static_cast<int>(pulseDiv) = 0 and pulseDiv is in a bad range
  // so v_max is out of range is covered
  ParametersCalculator::PhysicalParameters 
    minPulseDivParameters( 2, // low system clock
			   16, 
			   200, 
			   2000, // high maxRPM
			   0.5, 
			   0.24); 
  // should lead to pulse div < 0, but the min is 0
  // This leads to a v_max which is out of range (3495) and has to be
  // limited. => desired v_max cannot be reached 
  // Two warnings expected
  

  tmc429Parameters
    = ParametersCalculator::calculateParameters( minPulseDivParameters );
  BOOST_CHECK( tmc429Parameters.pulseDiv == 0 );
  BOOST_CHECK( tmc429Parameters.vMax == 2047 );
  BOOST_CHECK( tmc429Parameters.warnings.size() == 2 );

  // check that pulseDiv = 0 can be reached without warning
  // v_max must be smaller than the maximum
  ParametersCalculator::PhysicalParameters 
    smallPulseDivParameters( 16,
			     16,
			     800, 
			     2000,
			     0.5,
			     0.24); 

  tmc429Parameters
    = ParametersCalculator::calculateParameters( smallPulseDivParameters );
  BOOST_CHECK( tmc429Parameters.pulseDiv == 0 );
  BOOST_CHECK( tmc429Parameters.vMax == 1747);
  BOOST_CHECK( tmc429Parameters.warnings.size() == 0 );

  // If the requested speed is way too slow vMax will be calculated as 0,
  // which will lead to an exception
  ParametersCalculator::PhysicalParameters 
    tooSlowParameters( 32, // max system clock
		       1, // min microsteps
		       1, // very low (probably wrong) step per turn
		       2, // very low maxRPM
		       0.5, 
		       0.24); 
  
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( tooSlowParameters ), std::out_of_range );
}

BOOST_AUTO_TEST_CASE( testRampDivAMaxRanges ){
  ParametersCalculator::PhysicalParameters 
    maxRampDivParameters = vt21Parameters;
  maxRampDivParameters.timeToVMax = 10; // very slow acceleration

  ParametersCalculator::TMC429Parameters tmc429Parameters
    = ParametersCalculator::calculateParameters( maxRampDivParameters );
  BOOST_CHECK( tmc429Parameters.rampDiv == 13 );
  BOOST_CHECK( tmc429Parameters.warnings.size() == 1 );

  // check that rampDiv = 13 can be reached without warning
  ParametersCalculator::PhysicalParameters 
    largeRampDivParameters = vt21Parameters;
  largeRampDivParameters.timeToVMax = 2; // slow acceleration

  tmc429Parameters
    = ParametersCalculator::calculateParameters( largeRampDivParameters );
  BOOST_CHECK( tmc429Parameters.rampDiv == 13 );
  BOOST_CHECK( tmc429Parameters.warnings.size() == 0 );
  
  // Tuned so the other parameters stay in range.
  // Check with settings where the not rounded
  // value of pulseDiv is ]0..-1[, so the special case
  // 'static_cast<int>(pulseDiv) = 0 and pulseDiv is in a bad range
  // so v_max is out of range' is covered
  ParametersCalculator::PhysicalParameters 
    minRampDivParameters( 8, 
			  8, 
			  200, 
			  5000,
			  0.0005, // very fast acceleration
			  0.24); 
  // should lead to rapDiv < 0 , but the min is 0
  // This leads to a a_max which is out of range (2236) and has to be
  // limited. => desired a_max cannot be reached 
  // Two warnings expected
  tmc429Parameters
    = ParametersCalculator::calculateParameters( minRampDivParameters );
  BOOST_CHECK( tmc429Parameters.rampDiv == 0 );
  BOOST_CHECK( tmc429Parameters.aMax == 2047 );
  BOOST_CHECK( tmc429Parameters.warnings.size() == 2 );

  // check that rampDiv = 0 can be reached without warning
  // aMax must not be maximum
  // Tuned so other parameters are in range
  ParametersCalculator::PhysicalParameters 
    smallRampDivParameters( 8, 
			    16, 
			    200, 
			    2000,
			    0.001, // very fast acceleration
			    0.24); 
  tmc429Parameters
    = ParametersCalculator::calculateParameters( smallRampDivParameters );
  BOOST_CHECK( tmc429Parameters.rampDiv == 0 );
  BOOST_CHECK( tmc429Parameters.aMax == 1789 );
  BOOST_CHECK( tmc429Parameters.warnings.size() == 0 );

  ParametersCalculator::PhysicalParameters lowAMaxParameters 
    = vt21Parameters;
  lowAMaxParameters.timeToVMax = 150; // extremely low acceleration
  lowAMaxParameters.maxRPM = 20; // with 200 also pDiv is out of range
  // AMax is below the recommended value of 20, there whould be
  // an additional warning
  tmc429Parameters
    = ParametersCalculator::calculateParameters( lowAMaxParameters );
  BOOST_CHECK( tmc429Parameters.rampDiv == 13 );
  BOOST_CHECK( tmc429Parameters.aMax == 15 );
  BOOST_CHECK( tmc429Parameters.warnings.size() == 2 );

  // If the requested acceleration way too slow aMax will be calculated 
  // as 0, which will lead to an exception
  ParametersCalculator::PhysicalParameters tooSlowParameters 
    = vt21Parameters;
  tooSlowParameters.timeToVMax = 10000; // way too slow acceleration
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( tooSlowParameters ), std::out_of_range );
}

BOOST_AUTO_TEST_CASE( testPDiv ){
  ParametersCalculator::PhysicalParameters 
    physicalParameters = vt21Parameters;

  // pDiv too small
  physicalParameters.timeToVMax = 0.005;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( 
	       physicalParameters), std::out_of_range );
  
  // pDiv too large
  physicalParameters.timeToVMax = 100;
  BOOST_CHECK_THROW( ParametersCalculator::calculateParameters( 
	       physicalParameters), std::out_of_range );

  // check that pDiv = 13 can be reached
  physicalParameters.timeToVMax = 60;
  ParametersCalculator::TMC429Parameters tmc429Parameters
    = ParametersCalculator::calculateParameters( physicalParameters );
  BOOST_CHECK( tmc429Parameters.pDiv == 13 );

  // check that pDiv = 0 can be reached
  physicalParameters.timeToVMax = 0.01;
  tmc429Parameters
    = ParametersCalculator::calculateParameters( physicalParameters );
  BOOST_CHECK( tmc429Parameters.pDiv == 0 );
}

BOOST_AUTO_TEST_SUITE_END()
