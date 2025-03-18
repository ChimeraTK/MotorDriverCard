// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#define BOOST_TEST_MODULE GeneralStatusTest
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test_framework;

#include "GeneralStatus.h"

bool checkOperatorForDefaultConstructedObject(ChimeraTK::GeneralStatus& status) {
  switch(status) {
    case 0:
      return true;
      break;
    default:
      return false;
  }
}

BOOST_AUTO_TEST_SUITE(GeneralStatusTestSuite)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(GeneralStatusTestCase) {
#pragma GCC diagnostic pop

  std::stringstream testStream; // any kind of object which is std::ostream

  // test: dafault constuctor
  ChimeraTK::GeneralStatus status0;
  // test: parameter constuctor
  ChimeraTK::GeneralStatus status1(1);
  // test: copy constructor
  ChimeraTK::GeneralStatus status0Copy1(status0);
  // test: operator=
  ChimeraTK::GeneralStatus status0Copy2;
  status0Copy2 = status0;
  // status0 = status0;

  // test: operator int() const
  BOOST_CHECK(checkOperatorForDefaultConstructedObject(status0));

  // test: getId
  BOOST_CHECK(status0.getId() == 0);
  BOOST_CHECK(status0Copy1.getId() == 0);
  BOOST_CHECK(status0Copy2.getId() == 0);
  BOOST_CHECK(status1.getId() == 1);

  // test: operator==
  BOOST_CHECK(status0 == status0Copy1);
  BOOST_CHECK(status0 == status0Copy2);
  BOOST_CHECK(status0Copy1 == status0Copy2);

  // test: operator==
  BOOST_CHECK((status0 == status1) == false);

  // test: operator!=
  BOOST_CHECK(status0 != status1);

  BOOST_CHECK((status0 != status0Copy1) == false);

  // test: operator<<
  testStream << status0;
  BOOST_CHECK(testStream.str().compare("No name available in GeneralStatus object. Status id: (0)") == 0);

  testStream.str(std::string());
  testStream.clear();
  testStream << status0Copy1;
  BOOST_CHECK(testStream.str().compare("No name available in GeneralStatus object. Status id: (0)") == 0);

  testStream.str(std::string());
  testStream.clear();
  testStream << status0Copy2;
  BOOST_CHECK(testStream.str().compare("No name available in GeneralStatus object. Status id: (0)") == 0);

  testStream.str(std::string());
  testStream.clear();
  testStream << status1;
  BOOST_CHECK(testStream.str().compare("No name available in GeneralStatus object. Status id: (1)") == 0);

  // fail test
  // BOOST_CHECK( (status0 != status0Copy1) != false);
}

BOOST_AUTO_TEST_SUITE_END()
