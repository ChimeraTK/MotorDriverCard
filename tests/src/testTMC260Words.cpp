#define BOOST_TEST_MODULE TMC260WordsTest
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "createOutputMaskUsingSetterGetter.h"
#include "TMC260Words.h"
using namespace mtca4u;
using namespace mtca4u::tmc260;

BOOST_AUTO_TEST_SUITE( TMC260WordsTestSuite )

BOOST_AUTO_TEST_CASE( testDriverControlData ){
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverControlData> ( &DriverControlData::setInterpolation,
								      &DriverControlData::getInterpolation )
               == 0x00200 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverControlData> ( &DriverControlData::setDoubleEdge,
								      &DriverControlData::getDoubleEdge )
	       == 0x00100 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverControlData> ( &DriverControlData::setMicroStepResolution,
								      &DriverControlData::getMicroStepResolution )
	       == 0x0000F );
}

BOOST_AUTO_TEST_CASE( testChopperControlData ){
  BOOST_CHECK( createOutputMaskUsingSetterGetter<ChopperControlData> ( &ChopperControlData::setBlankingTime,
								       &ChopperControlData::getBlankingTime )
	       == 0x98000 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<ChopperControlData> ( &ChopperControlData::setChopperMode,
								       &ChopperControlData::getChopperMode )
	       == 0x84000 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<ChopperControlData> ( &ChopperControlData::setRandomOffTime,
								       &ChopperControlData::getRandomOffTime )
	       == 0x82000 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<ChopperControlData> ( &ChopperControlData::setHysteresisDecrementInterval,
								       &ChopperControlData::getHysteresisDecrementInterval )
	       == 0x81800 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<ChopperControlData> ( &ChopperControlData::setHysteresisEndValue,
								       &ChopperControlData::getHysteresisEndValue )
	       == 0x80780 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<ChopperControlData> ( &ChopperControlData::setHysteresisStartValue,
								       &ChopperControlData::getHysteresisStartValue )
	       == 0x80070 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<ChopperControlData> ( &ChopperControlData::setOffTime,
								       &ChopperControlData::getOffTime )
	       == 0x8000F );
}

BOOST_AUTO_TEST_SUITE_END()
