#define BOOST_TEST_MODULE MotorDriverCardConfigXMLTest

#include "MotorDriverCardConfigXML.h"
#include "XMLException.h"
using namespace mtca4u;

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

BOOST_AUTO_TEST_SUITE( MotorDriverCardConfigXMLTestSuite )

BOOST_AUTO_TEST_CASE( testInvalidInput ){
  BOOST_CHECK_THROW( MotorDriverCardConfigXML::read("invalid.xml"), 
		     XMLException );
}

BOOST_AUTO_TEST_SUITE_END()
