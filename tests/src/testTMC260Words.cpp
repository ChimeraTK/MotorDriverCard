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
  // as the output "mask" also contains the content of the address field, these tests automatically
  // check that the address is set correctly (bits 17:19)
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

BOOST_AUTO_TEST_CASE( testCoolStepControlData ){
  BOOST_CHECK( createOutputMaskUsingSetterGetter<CoolStepControlData> ( &CoolStepControlData::setPayloadData,
									&CoolStepControlData::getPayloadData )
               == 0xBFFFF );
}

BOOST_AUTO_TEST_CASE( testStallGuardControlData ){
  BOOST_CHECK( createOutputMaskUsingSetterGetter<StallGuardControlData> ( &StallGuardControlData::setPayloadData,
									  &StallGuardControlData::getPayloadData )
               == 0xDFFFF );
}

BOOST_AUTO_TEST_CASE( testDriverConfigData ){
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverConfigData> ( &DriverConfigData::setPayloadData,
								     &DriverConfigData::getPayloadData )
               == 0xFFFFF );
}

BOOST_AUTO_TEST_CASE( testDriverStatusData ){
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverStatusData> ( &DriverStatusData::setStallGuardStatus,
								     &DriverStatusData::getStallGuardStatus )
               == 0x01 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverStatusData> ( &DriverStatusData::setOvertemperatureShutdown,
								     &DriverStatusData::getOvertemperatureShutdown )
               == 0x02 );

  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverStatusData> ( &DriverStatusData::setOvertemperatureWarning,
								     &DriverStatusData::getOvertemperatureWarning )
               == 0x04 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverStatusData> ( &DriverStatusData::setShortToGroundCoilA,
								     &DriverStatusData::getShortToGroundCoilA )
               == 0x08 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverStatusData> ( &DriverStatusData::setShortToGroundCoilB,
								     &DriverStatusData::getShortToGroundCoilB )
               == 0x10 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverStatusData> ( &DriverStatusData::setShortToGroundIndicators,
								     &DriverStatusData::getShortToGroundIndicators )
               == 0x18 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverStatusData> ( &DriverStatusData::setOpenLoadCoilA,
								     &DriverStatusData::getOpenLoadCoilA )
               == 0x20 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverStatusData> ( &DriverStatusData::setOpenLoadCoilB,
								     &DriverStatusData::getOpenLoadCoilB )
               == 0x40 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverStatusData> ( &DriverStatusData::setOpenLoadIndicators,
								     &DriverStatusData::getOpenLoadIndicators )
               == 0x60 );
  BOOST_CHECK( createOutputMaskUsingSetterGetter<DriverStatusData> ( &DriverStatusData::setStandstillIndicator,
								     &DriverStatusData::getStandstillIndicator )
               == 0x80 );
}



BOOST_AUTO_TEST_SUITE_END()
