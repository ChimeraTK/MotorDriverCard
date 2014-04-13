#define BOOST_TEST_MODULE SignedIntConverterTest
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "SignedIntConverter.h"
using namespace mtca4u;

BOOST_AUTO_TEST_SUITE( SignedIntConverterTestSuite )

BOOST_AUTO_TEST_CASE( from24bits ){
  SignedIntConverter converter24( 24 );
  BOOST_CHECK( converter24.customToThirtyTwo( 0xFFFFFF ) == -1 );
  BOOST_CHECK( converter24.customToThirtyTwo( 0xAAAAAA ) == -5592406 );
  BOOST_CHECK( converter24.customToThirtyTwo( 25 ) == 25 );
  BOOST_CHECK( converter24.customToThirtyTwo( 0 ) == 0 );
  BOOST_CHECK( converter24.customToThirtyTwo( 0x7FFFFF ) == 0x7FFFFF );
}

BOOST_AUTO_TEST_CASE( to24bits ){
  SignedIntConverter converter24( 24 );
  BOOST_CHECK( converter24.thrityTwoToCustom( -1 ) == 0xFFFFFF );
  BOOST_CHECK( converter24.thrityTwoToCustom( -16 ) == 0xFFFFF0 );
  BOOST_CHECK( converter24.thrityTwoToCustom( 25 ) == 25 );
  BOOST_CHECK( converter24.thrityTwoToCustom( 0 ) == 0 );
  BOOST_CHECK( converter24.thrityTwoToCustom( 0x7FFFFF ) == 0x7FFFFF );
}




BOOST_AUTO_TEST_SUITE_END()
