
#define BOOST_TEST_MODULE LoggerModuleTest
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test_framework;

#include "Logger.h"

BOOST_AUTO_TEST_SUITE(LoggerTestSuite)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(testFullDetailLevelOfLogger) {
#pragma GCC diagnostic pop

  std::stringstream testStream; // any kind of object which is std::ostream

  mtca4u::Logger logger(mtca4u::Logger::NO_LOGGING,
      testStream); // CREATE OBJECT WIT DEFAULT SETTINGS - logging level to
                   // NO_LOGGING, logging stream set to std::cout
  logger.setLogLevel(mtca4u::Logger::FULL_DETAIL);
  BOOST_CHECK(logger.getLogLevel() == mtca4u::Logger::FULL_DETAIL);

  testStream.str(std::string());
  testStream.clear();
  logger() << " EMPTY FUNCTOR message";
  BOOST_CHECK(testStream.str().compare(" EMPTY FUNCTOR message") == 0); // WILL BE LOGGED

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::ERROR) << " ERROR message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" ERROR message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::WARNING) << " WARNING message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" WARNING message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::INFO) << " INFO message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" INFO message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::DETAIL) << " DETAIL message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" DETAIL message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::FULL_DETAIL) << " FULL_DETAIL message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" FULL_DETAIL message") == 0);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(testDetailLevelOfLogger) {
#pragma GCC diagnostic pop

  std::stringstream testStream; // any kind of object which is std::ostream

  mtca4u::Logger logger(mtca4u::Logger::NO_LOGGING,
      testStream); // CREATE OBJECT WIT DEFAULT SETTINGS - logging level to
                   // NO_LOGGING, logging stream set to std::cout
  logger.setLogLevel(mtca4u::Logger::DETAIL);
  BOOST_CHECK(logger.getLogLevel() == mtca4u::Logger::DETAIL);

  testStream.str(std::string());
  testStream.clear();
  logger() << " EMPTY FUNCTOR message";
  BOOST_CHECK(testStream.str().compare(" EMPTY FUNCTOR message") == 0); // WILL BE LOGGED

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::ERROR) << " ERROR message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" ERROR message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::WARNING) << " WARNING message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" WARNING message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::INFO) << " INFO message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" INFO message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::DETAIL) << " DETAIL message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" DETAIL message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::FULL_DETAIL) << " FULL_DETAIL message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(testInfoLevelOfLogger) {
#pragma GCC diagnostic pop

  std::stringstream testStream; // any kind of object which is std::ostream

  mtca4u::Logger logger(mtca4u::Logger::NO_LOGGING,
      testStream); // CREATE OBJECT WIT DEFAULT SETTINGS - logging level to
                   // NO_LOGGING, logging stream set to std::cout
  logger.setLogLevel(mtca4u::Logger::INFO);
  BOOST_CHECK(logger.getLogLevel() == mtca4u::Logger::INFO);

  testStream.str(std::string());
  testStream.clear();
  logger() << " EMPTY FUNCTOR message";
  BOOST_CHECK(testStream.str().compare(" EMPTY FUNCTOR message") == 0); // WILL BE LOGGED

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::ERROR) << " ERROR message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" ERROR message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::WARNING) << " WARNING message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" WARNING message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::INFO) << " INFO message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" INFO message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::DETAIL) << " DETAIL message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::FULL_DETAIL) << " FULL_DETAIL message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(testWarningLevelOfLogger) {
#pragma GCC diagnostic pop

  std::stringstream testStream; // any kind of object which is std::ostream

  mtca4u::Logger logger(mtca4u::Logger::NO_LOGGING,
      testStream); // CREATE OBJECT WIT DEFAULT SETTINGS - logging level to
                   // NO_LOGGING, logging stream set to std::cout
  logger.setLogLevel(mtca4u::Logger::WARNING);
  BOOST_CHECK(logger.getLogLevel() == mtca4u::Logger::WARNING);

  testStream.str(std::string());
  testStream.clear();
  logger() << " EMPTY FUNCTOR message";
  BOOST_CHECK(testStream.str().compare(" EMPTY FUNCTOR message") == 0); // WILL BE LOGGED

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::ERROR) << " ERROR message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" ERROR message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::WARNING) << " WARNING message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" WARNING message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::INFO) << " INFO message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::DETAIL) << " DETAIL message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::FULL_DETAIL) << " FULL_DETAIL message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(testErrorLevelOfLogger) {
#pragma GCC diagnostic pop

  std::stringstream testStream; // any kind of object which is std::ostream

  mtca4u::Logger logger(mtca4u::Logger::NO_LOGGING,
      testStream); // CREATE OBJECT WIT DEFAULT SETTINGS - logging level to
                   // NO_LOGGING, logging stream set to std::cout
  logger.setLogLevel(mtca4u::Logger::ERROR);
  BOOST_CHECK(logger.getLogLevel() == mtca4u::Logger::ERROR);

  testStream.str(std::string());
  testStream.clear();
  logger() << " EMPTY FUNCTOR message";
  BOOST_CHECK(testStream.str().compare(" EMPTY FUNCTOR message") == 0); // WILL BE LOGGED

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::ERROR) << " ERROR message"; // WILL BE LOGGED
  BOOST_CHECK(testStream.str().compare(" ERROR message") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::WARNING) << " WARNING message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::INFO) << " INFO message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::DETAIL) << " DETAIL message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::FULL_DETAIL) << " FULL_DETAIL message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(testNoLoggingLevelOfLogger) {
#pragma GCC diagnostic pop

  std::stringstream testStream; // any kind of object which is std::ostream

  mtca4u::Logger logger(mtca4u::Logger::NO_LOGGING,
      testStream); // CREATE OBJECT WIT DEFAULT SETTINGS - logging level to
                   // NO_LOGGING, logging stream set to std::cout
  logger.setLogLevel(mtca4u::Logger::NO_LOGGING);
  BOOST_CHECK(logger.getLogLevel() == mtca4u::Logger::NO_LOGGING);

  testStream.str(std::string());
  testStream.clear();
  logger() << " EMPTY FUNCTOR message";
  BOOST_CHECK(testStream.str().compare(" EMPTY FUNCTOR message") == 0); // WILL BE LOGGED

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::NO_LOGGING) << " NO_LOGGING message" << std::endl; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::ERROR) << " ERROR message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::WARNING) << " WARNING message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::INFO) << " INFO message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::DETAIL) << " DETAIL message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);

  testStream.str(std::string());
  testStream.clear();
  logger(mtca4u::Logger::FULL_DETAIL) << " FULL_DETAIL message"; // WILL NOT BE LOGGED
  BOOST_CHECK(testStream.str().compare("") == 0);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(testOperators) {
#pragma GCC diagnostic pop

  std::stringstream testStream; // any kind of object which is std::ostream

  mtca4u::Logger logger(mtca4u::Logger::NO_LOGGING,
      testStream); // CREATE OBJECT WIT DEFAULT SETTINGS - logging level to
                   // NO_LOGGING, logging stream set to std::cout

  logger.setLogLevel(mtca4u::Logger::NO_LOGGING);
  BOOST_CHECK(logger.getLogLevel() == mtca4u::Logger::NO_LOGGING);
  testStream.str(std::string());
  testStream.clear();
  logger() << logger;
  BOOST_CHECK(testStream.str().compare("Current logging level: NO_LOGGING (0)") == 0);

  logger.setLogLevel(mtca4u::Logger::ERROR);
  BOOST_CHECK(logger.getLogLevel() == mtca4u::Logger::ERROR);
  testStream.str(std::string());
  testStream.clear();
  logger() << logger;
  BOOST_CHECK(testStream.str().compare("Current logging level: ERROR (1)") == 0);

  logger.setLogLevel(mtca4u::Logger::WARNING);
  BOOST_CHECK(logger.getLogLevel() == mtca4u::Logger::WARNING);
  testStream.str(std::string());
  testStream.clear();
  logger() << logger;
  BOOST_CHECK(testStream.str().compare("Current logging level: WARNING (2)") == 0);

  logger.setLogLevel(mtca4u::Logger::INFO);
  BOOST_CHECK(logger.getLogLevel() == mtca4u::Logger::INFO);
  testStream.str(std::string());
  testStream.clear();
  logger() << logger;
  BOOST_CHECK(testStream.str().compare("Current logging level: INFO (3)") == 0);

  logger.setLogLevel(mtca4u::Logger::DETAIL);
  BOOST_CHECK(logger.getLogLevel() == mtca4u::Logger::DETAIL);
  testStream.str(std::string());
  testStream.clear();
  logger() << logger;
  BOOST_CHECK(testStream.str().compare("Current logging level: DETAIL (4)") == 0);

  logger.setLogLevel(mtca4u::Logger::FULL_DETAIL);
  BOOST_CHECK(logger.getLogLevel() == mtca4u::Logger::FULL_DETAIL);
  testStream.str(std::string());
  testStream.clear();
  logger() << logger;
  BOOST_CHECK(testStream.str().compare("Current logging level: FULL_DETAIL (5)") == 0);

  BOOST_CHECK((logger.getLogLevel() != mtca4u::Logger::FULL_DETAIL) == false);
  BOOST_CHECK((logger.getLogLevel() == mtca4u::Logger::ERROR) == false);
}

BOOST_AUTO_TEST_SUITE_END()
