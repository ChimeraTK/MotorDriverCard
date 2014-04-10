#define BOOST_TEST_MODULE MotorReferenceSwitchDataTest
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "createOutputMaskUsingSetterGetter.h"
#include "MotorReferenceSwitchData.h"
using namespace mtca4u;

BOOST_AUTO_TEST_SUITE( MotorReferenceSwitchDataTestSuite )

BOOST_AUTO_TEST_CASE( testBitValues ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<MotorReferenceSwitchData> (
		     &MotorReferenceSwitchData::setPositiveSwitch,
		     &MotorReferenceSwitchData::getPositiveSwitch )
	      == 0x00000001 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<MotorReferenceSwitchData> (
		     &MotorReferenceSwitchData::setNegativeSwitch,
		     &MotorReferenceSwitchData::getNegativeSwitch )
	      == 0x00000002 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<MotorReferenceSwitchData> (
		     &MotorReferenceSwitchData::setSwitchStatuses,
		     &MotorReferenceSwitchData::getSwitchStatuses )
	      == 0x00000003 );
}

BOOST_AUTO_TEST_CASE( testBoolOperator ){

  MotorReferenceSwitchData motorReferenceSwitchData;
  BOOST_CHECK( !motorReferenceSwitchData );

  motorReferenceSwitchData.setPositiveSwitch(1);
  BOOST_CHECK( motorReferenceSwitchData );
  
  motorReferenceSwitchData.setNegativeSwitch(1);
  BOOST_CHECK( motorReferenceSwitchData );
 
  motorReferenceSwitchData.setPositiveSwitch(0);
  BOOST_CHECK( motorReferenceSwitchData );
  
  motorReferenceSwitchData.setNegativeSwitch(0);
  BOOST_CHECK( !motorReferenceSwitchData );

  // Set some unused bits. They have to be ignored in the status
  MotorReferenceSwitchData motorReferenceSwitchData2(0x4);

  BOOST_CHECK( !motorReferenceSwitchData2 );
 
}


BOOST_AUTO_TEST_SUITE_END()
