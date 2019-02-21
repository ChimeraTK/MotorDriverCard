#define BOOST_TEST_MODULE StepperMotorCalibrationStatusTest
#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test_framework;

#include "StepperMotorCalibrationStatus.h"

bool checkOperatorForDefaultConstructedObject(
    mtca4u::StepperMotorCalibrationStatus &error) {
  switch (error) {
  case 1:
    return true;
    break;
  default:
    return false;
  }
}

BOOST_AUTO_TEST_SUITE(StepperMotorCalibrationStastusTestSuite)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
BOOST_AUTO_TEST_CASE(StepperMotorCalibrationStastusTestCase) {
#pragma GCC diagnostic pop

  std::stringstream testStream; // any kind of object which is std::ostream

  // test: dafault constuctor
  mtca4u::StepperMotorCalibrationStatus defaultCalibrationStatus;
  // test: parameter constuctor
  mtca4u::StepperMotorCalibrationStatus calibrationStatus2(2);
  // test: copy constructor
  mtca4u::StepperMotorCalibrationStatus defaultCalibrationStatusCopy1(
      defaultCalibrationStatus);
  // test: operator=
  mtca4u::StepperMotorCalibrationStatus defaultCalibrationStatusCopy2;
  defaultCalibrationStatusCopy2 = defaultCalibrationStatus;
  defaultCalibrationStatus = defaultCalibrationStatus;

  // test: operator int() const
  BOOST_CHECK(
      checkOperatorForDefaultConstructedObject(defaultCalibrationStatus));
  BOOST_CHECK(
      checkOperatorForDefaultConstructedObject(defaultCalibrationStatusCopy1));
  BOOST_CHECK(
      checkOperatorForDefaultConstructedObject(defaultCalibrationStatusCopy2));

  // test: getId
  BOOST_CHECK(defaultCalibrationStatus.getId() == 1);
  BOOST_CHECK(defaultCalibrationStatusCopy1.getId() == 1);
  BOOST_CHECK(defaultCalibrationStatusCopy2.getId() == 1);
  BOOST_CHECK(calibrationStatus2.getId() == 2);

  // test: operator==
  BOOST_CHECK(defaultCalibrationStatus == defaultCalibrationStatusCopy1);
  BOOST_CHECK(defaultCalibrationStatus == defaultCalibrationStatusCopy2);
  BOOST_CHECK(defaultCalibrationStatusCopy1 == defaultCalibrationStatusCopy2);

  // test: operator==
  BOOST_CHECK((defaultCalibrationStatus == calibrationStatus2) == false);

  // test: operator!=
  BOOST_CHECK(defaultCalibrationStatus != calibrationStatus2);
  BOOST_CHECK((defaultCalibrationStatus != defaultCalibrationStatusCopy1) ==
              false);

  // test: operator<<
  mtca4u::StepperMotorCalibrationStatus statusUNKNOWN(
      mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATION_UNKNOWN);
  testStream << statusUNKNOWN;
  BOOST_CHECK(testStream.str().compare("Calibration status UNKNOWN (1)") == 0);

  testStream.str(std::string());
  testStream.clear();
  mtca4u::StepperMotorCalibrationStatus statusCALIBRATED(
      mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATED);
  testStream << statusCALIBRATED;
  BOOST_CHECK(testStream.str().compare("Motor is CALIBRATED (2)") == 0);

  testStream.str(std::string());
  testStream.clear();
  mtca4u::StepperMotorCalibrationStatus statusNOT_CALIBTRATED(
      mtca4u::StepperMotorCalibrationStatusType::M_NOT_CALIBRATED);
  testStream << statusNOT_CALIBTRATED;
  BOOST_CHECK(testStream.str().compare("Motor is NOT CALIBRATED (4)") == 0);

  testStream.str(std::string());
  testStream.clear();
  mtca4u::StepperMotorCalibrationStatus statusCALIBRATION_FAILED(
      mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED);
  testStream << statusCALIBRATION_FAILED;
  BOOST_CHECK(testStream.str().compare("Motor calibration FAILED (8)") == 0);

  testStream.str(std::string());
  testStream.clear();
  mtca4u::StepperMotorCalibrationStatus statusCALIBRATION_IN_PROGRESS(
      mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS);
  testStream << statusCALIBRATION_IN_PROGRESS;
  BOOST_CHECK(
      testStream.str().compare("Motor calibration is IN PROGRESS (16)") == 0);

  testStream.str(std::string());
  testStream.clear();
  mtca4u::StepperMotorCalibrationStatus statusCALIBRATION_STOPPED_BY_USER(
      mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER);
  testStream << statusCALIBRATION_STOPPED_BY_USER;
  BOOST_CHECK(
      testStream.str().compare("Motor calibration STOPPED BY USER (32)") == 0);

  testStream.str(std::string());
  testStream.clear();
  mtca4u::StepperMotorCalibrationStatus statusCALIBRATION_NOT_AVAILABLE(
      mtca4u::StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE);
  testStream << statusCALIBRATION_NOT_AVAILABLE;
  BOOST_CHECK(
      testStream.str().compare("Motor calibration NOT AVAILABLE (64)") == 0);

  testStream.str(std::string());
  testStream.clear();
  mtca4u::StepperMotorCalibrationStatus errorRandom(123456);
  testStream << errorRandom;
  BOOST_CHECK(
      testStream.str().compare(
          "No name available in GeneralStatus object. Status id: (123456)") ==
      0);
}

BOOST_AUTO_TEST_SUITE_END()
