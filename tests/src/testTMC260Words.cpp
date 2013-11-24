#define BOOST_TEST_MODULE TMC260WordsTest
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "TMC260Words.h"
using namespace mtca4u;
using namespace mtca4u::tmc260;

/** This function is used to evaluate that the bit pattern given in
 *  ADD_VARIABLE is correct. It takes an empty word and sets all bits described in the setVARIABLE function.
 *  Afterwards it returns the whole data word, which can be compared to the correct bit pattern with only the
 *  bits according to the pattern are on.
 *  By using the bit numbers in the definition and the bit pattern in the evaluation the chance of 
 *  having the same error in these two independent implementations is minimised.
 * 
 *  As setVARIABLE and getVARIABLE are both generated from the same macro, which has been tested independently,
 *  just using the setter is sufficient.
 */
template<class T> unsigned int createDataWordUsingSetterFunction( void (T::* setterFunction )(unsigned int) ){
  T t;
  (t.*setterFunction)(0xFFFFFFFF);
  return t.getDataWord();
}

BOOST_AUTO_TEST_SUITE( TMC260WordsTestSuite )

BOOST_AUTO_TEST_CASE( testDriverControlData ){
  BOOST_CHECK( createDataWordUsingSetterFunction<DriverControlData> ( &DriverControlData::setInterpolation )       == 0x00200 );
  BOOST_CHECK( createDataWordUsingSetterFunction<DriverControlData> ( &DriverControlData::setDoubleEdge )          == 0x00100 );
  BOOST_CHECK( createDataWordUsingSetterFunction<DriverControlData> ( &DriverControlData::setMicroStepResolution ) == 0x0000F );
}

BOOST_AUTO_TEST_CASE( testChopperControlData ){
  BOOST_CHECK( createDataWordUsingSetterFunction<ChopperControlData> ( &ChopperControlData::setBlankingTime )                == 0x98000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ChopperControlData> ( &ChopperControlData::setChopperMode )                 == 0x84000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ChopperControlData> ( &ChopperControlData::setRandomOffTime )               == 0x82000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ChopperControlData> ( &ChopperControlData::setHysteresisDecrementInterval ) == 0x81800 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ChopperControlData> ( &ChopperControlData::setHysteresisEndValue )          == 0x80780 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ChopperControlData> ( &ChopperControlData::setHysteresisStartValue )        == 0x80070 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ChopperControlData> ( &ChopperControlData::setOffTime )                     == 0x8000F );
}

BOOST_AUTO_TEST_SUITE_END()
