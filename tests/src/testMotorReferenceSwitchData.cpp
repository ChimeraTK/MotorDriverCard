#define BOOST_TEST_MODULE MotorReferenceSwitchDataTest
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "createOutputMaskUsingSetterGetter.h"
#include "MotorReferenceSwitchData.h"
using namespace mtca4u;

BOOST_AUTO_TEST_SUITE( MotorReferenceSwitchDataTestSuite )

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE( testBitValues ){
#pragma GCC diagnostic pop

  BOOST_CHECK(createOutputMaskUsingSetterGetter<MotorReferenceSwitchData> (
		     &MotorReferenceSwitchData::setPositiveSwitchActive,
		     &MotorReferenceSwitchData::getPositiveSwitchActive )
	      == 0x00000001 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<MotorReferenceSwitchData> (
		     &MotorReferenceSwitchData::setNegativeSwitchActive,
		     &MotorReferenceSwitchData::getNegativeSwitchActive )
	      == 0x00000002 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<MotorReferenceSwitchData> (
		     &MotorReferenceSwitchData::setSwitchesActiveWord,
		     &MotorReferenceSwitchData::getSwitchesActiveWord )
	      == 0x00000003 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<MotorReferenceSwitchData> (
		     &MotorReferenceSwitchData::setPositiveSwitchEnabled,
		     &MotorReferenceSwitchData::getPositiveSwitchEnabled )
	      == 0x00010000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<MotorReferenceSwitchData> (
		     &MotorReferenceSwitchData::setNegativeSwitchEnabled,
		     &MotorReferenceSwitchData::getNegativeSwitchEnabled )
	      == 0x00020000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<MotorReferenceSwitchData> (
		     &MotorReferenceSwitchData::setSwitchesEnabledWord,
		     &MotorReferenceSwitchData::getSwitchesEnabledWord )
	      == 0x00030000 );
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE( testBoolOperator ){
#pragma GCC diagnostic pop

  MotorReferenceSwitchData motorReferenceSwitchData;
  BOOST_CHECK( !motorReferenceSwitchData );

  motorReferenceSwitchData.setPositiveSwitchActive(1);
  BOOST_CHECK( motorReferenceSwitchData );
  
  motorReferenceSwitchData.setNegativeSwitchActive(1);
  BOOST_CHECK( motorReferenceSwitchData );
 
  motorReferenceSwitchData.setPositiveSwitchActive(0);
  BOOST_CHECK( motorReferenceSwitchData );
  
  motorReferenceSwitchData.setNegativeSwitchActive(0);
  BOOST_CHECK( !motorReferenceSwitchData );

  // Set some unused bits. They have to be ignored in the status
  MotorReferenceSwitchData motorReferenceSwitchData2(0x4);

  BOOST_CHECK( !motorReferenceSwitchData2 );
 
}


BOOST_AUTO_TEST_SUITE_END()
