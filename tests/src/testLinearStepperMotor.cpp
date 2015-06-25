
#include <sstream>

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "LinearStepperMotor.h"
#include "MotorControlerDummy.h"
#include "MotorDriverCardFactory.h"


#include <boost/thread.hpp>

using namespace mtca4u;

// A macro to declare tests for an unimplemented get/set pair.
// It is checked that the NotImplementedException is thrown


static const unsigned int THE_ID = 17;

static const std::string stepperMotorDeviceName("DFMC-MD22-DUMMY");
static const std::string stepperMotorDeviceConfigFile("VT21-MotorDriverCardConfig.xml");
static const std::string dmapPath(".");
static const std::string moduleName("");

class TestUnitConveter : public StepperMotorUnitsConverter {
public:

    TestUnitConveter() {
    };

    virtual float stepsToUnits(int steps) {
        return float(steps / 10.0);
    };

    virtual int unitsToSteps(float units) {
        return int(10.0 * units);
    };
};

class LinearStepperMotorTest {
public:
    LinearStepperMotorTest();

    void testSoftLimits();
    void testStop();
    void testEmergencyStop();
    void testCalibration();
    void threadCalibration(int i);
    void testSetPositionAs();

    void testMoveToPosition();
    void threadMoveToPostion(int i);

private:
    boost::shared_ptr<LinearStepperMotor> _stepperMotor;
    boost::shared_ptr<MotorControlerDummy> _motorControlerDummy;

    boost::shared_ptr<TestUnitConveter> _testUnitConveter;

};

class LinearStepperMotorTestSuite : public test_suite {
public:

    LinearStepperMotorTestSuite() : test_suite("LinearStepperMotor suite") {
        // create an instance of the test class
        boost::shared_ptr<LinearStepperMotorTest> stepperMotorTest(new LinearStepperMotorTest);

        add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testSoftLimits,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testStop,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testEmergencyStop,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testSetPositionAs,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testMoveToPosition,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&LinearStepperMotorTest::testCalibration,
                stepperMotorTest));
    }
};

test_suite*
init_unit_test_suite(int /*argc*/, char* /*argv*/ []) {
    framework::master_test_suite().p_name.value = "LinearStepperMotor test suite";
    return new LinearStepperMotorTestSuite;
}

LinearStepperMotorTest::LinearStepperMotorTest() {

    std::string deviceFileName(dmapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).dev_file);
    std::string mapFileName(dmapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).map_file_name);

    _testUnitConveter.reset(new TestUnitConveter);

    _motorControlerDummy = boost::dynamic_pointer_cast<MotorControlerDummy>(MotorDriverCardFactory::instance().createMotorDriverCard(deviceFileName, mapFileName, moduleName, stepperMotorDeviceConfigFile)->getMotorControler(0));

    _stepperMotor.reset(new LinearStepperMotor(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile, dmapPath));



    //!!!! CHANGE THIS FOR LINEAR STEPER MOTOR TESTS
    _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
    _motorControlerDummy->setNegativeReferenceSwitchEnabled(true);
}

void LinearStepperMotorTest::testSoftLimits() {
    std::cout << "testSoftLimits()" << std::endl;

    _stepperMotor->setEnabled(true);

    _stepperMotor->setSoftwareLimitsEnabled(true);
    BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() == true);

    _stepperMotor->setSoftwareLimitsEnabled(false);
    BOOST_CHECK(_stepperMotor->getSoftwareLimitsEnabled() == false);


    _stepperMotor->setMaxPositionLimit(150);
    _stepperMotor->setMinPositionLimit(50);

    BOOST_CHECK(_stepperMotor->getMaxPositionLimit() == 150);
    BOOST_CHECK(_stepperMotor->getMinPositionLimit() == 50);


    _stepperMotor->setSoftwareLimitsEnabled(true);

    _stepperMotor->setTargetPosition(200);
    BOOST_CHECK(_stepperMotor->getTargetPosition() == 150);

    _stepperMotor->setTargetPosition(0);
    BOOST_CHECK(_stepperMotor->getTargetPosition() == 50);

    _stepperMotor->setTargetPosition(100);
    BOOST_CHECK(_stepperMotor->getTargetPosition() == 100);

    _stepperMotor->setSoftwareLimitsEnabled(false);

    _stepperMotor->setTargetPosition(200);
    BOOST_CHECK(_stepperMotor->getTargetPosition() == 200);

    _stepperMotor->setTargetPosition(0);
    BOOST_CHECK(_stepperMotor->getTargetPosition() == 0);

    _stepperMotor->setTargetPosition(100);
    BOOST_CHECK(_stepperMotor->getTargetPosition() == 100);


    //should trigger error state - target position should not be changed
    _stepperMotor->setMaxPositionLimit(50);
    _stepperMotor->setMinPositionLimit(150);

    _stepperMotor->setSoftwareLimitsEnabled(true);

    _stepperMotor->setTargetPosition(200);
    BOOST_CHECK(_stepperMotor->getTargetPosition() == 100);

    _stepperMotor->setTargetPosition(0);
    BOOST_CHECK(_stepperMotor->getTargetPosition() == 100);

    _stepperMotor->setTargetPosition(100);
    BOOST_CHECK(_stepperMotor->getTargetPosition() == 100);

    _stepperMotor->setSoftwareLimitsEnabled(false);

    _stepperMotor->setTargetPosition(200);
    BOOST_CHECK(_stepperMotor->getTargetPosition() == 200);

    _stepperMotor->setTargetPosition(0);
    BOOST_CHECK(_stepperMotor->getTargetPosition() == 0);

    _stepperMotor->setTargetPosition(100);
    BOOST_CHECK(_stepperMotor->getTargetPosition() == 100);


    _stepperMotor->setMaxPositionLimit(150);
    _stepperMotor->setMinPositionLimit(50);
    _stepperMotor->setTargetPosition(200);
    _stepperMotor->setSoftwareLimitsEnabled(true);

    BOOST_CHECK((_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON) == true);

    _stepperMotor->setSoftwareLimitsEnabled(false);
    _stepperMotor->setTargetPosition(0);
    _stepperMotor->setSoftwareLimitsEnabled(true);

    BOOST_CHECK((_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON) == true);

    _stepperMotor->setSoftwareLimitsEnabled(false);
    _stepperMotor->setEnabled(false);
}

void LinearStepperMotorTest::testStop() {
    std::cout << "testStop()" << std::endl;

    _motorControlerDummy->setTargetPosition(0);
    _motorControlerDummy->setActualPosition(0);

    _stepperMotor->setEnabled(true);

    _stepperMotor->setAutostart(false);

    //test start function
    _stepperMotor->setTargetPosition(-500);
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_NOT_IN_POSITION);
    _stepperMotor->start();
    BOOST_CHECK(_motorControlerDummy->getTargetPosition() == (-500));
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_IN_MOVE);
    _motorControlerDummy->moveTowardsTarget(0.5);
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_IN_MOVE);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == (-500 * 0.5));
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());

    _motorControlerDummy->moveTowardsTarget(0.5);
    _stepperMotor->stop();
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_OK);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == (-500 * 0.75));
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getTargetPosition());

}

void LinearStepperMotorTest::testEmergencyStop() {
    std::cout << "testEmergencyStop()" << std::endl;

    _motorControlerDummy->setTargetPosition(0);
    _motorControlerDummy->setActualPosition(0);

    _stepperMotor->setEnabled(true);

    _stepperMotor->setAutostart(false);

    //test start function
    _stepperMotor->setTargetPosition(500);
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_NOT_IN_POSITION);
    _stepperMotor->start();
    BOOST_CHECK(_motorControlerDummy->getTargetPosition() == (500));
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_IN_MOVE);
    _motorControlerDummy->moveTowardsTarget(0.5);

    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_IN_MOVE);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == (500 * 0.5));
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());

    _motorControlerDummy->moveTowardsTarget(0.5);
    _stepperMotor->emergencyStop();
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_DISABLED);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == (500 * 0.75));
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getTargetPosition());
    BOOST_CHECK(_motorControlerDummy->getTargetPosition() == _motorControlerDummy->getActualPosition());

}

void LinearStepperMotorTest::testSetPositionAs() {
    std::cout << "testSetPositionAs()" << std::endl;

    _motorControlerDummy->setTargetPosition(0);
    _motorControlerDummy->setActualPosition(0);

    _stepperMotor->setAutostart(false);

    _stepperMotor->setEnabled(true);
    _stepperMotor->setCurrentPositionAs(500);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == 500);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getTargetPosition());
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_OK);


    _stepperMotor->setEnabled(false);
    _stepperMotor->setCurrentPositionAs(-200);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == -200);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getTargetPosition());
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_DISABLED);
}

void LinearStepperMotorTest::testMoveToPosition() {
    std::cout << "testMoveToPosition()" << std::endl;

    _motorControlerDummy->setTargetPosition(0);
    _motorControlerDummy->setActualPosition(0);

    _stepperMotor->setAutostart(false);
    _stepperMotor->setEnabled(true);

    // move toward but emulate the not react on command error
    _motorControlerDummy->moveTowardsTarget(1, true);
    LinearStepperMotorStatusAndError statusAndError = _stepperMotor->moveToPosition(500);
    BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_ERROR);
    BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_REACTION_ON_COMMAND);

    // set position to the wanted one and later try to send motor to the same position
    _motorControlerDummy->moveTowardsTarget(1, false);
    statusAndError = _stepperMotor->moveToPosition(500);
    BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_OK);
    BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_ERROR);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == 500);
    //std::cout << " Status: " << statusAndError.status << "  Error: " << statusAndError.error << std::endl;


    //set motor in error state - configuration error end test routine
    _stepperMotor->setMaxPositionLimit(50);
    _stepperMotor->setMinPositionLimit(100);
    _stepperMotor->setSoftwareLimitsEnabled(true);
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_ERROR);
    BOOST_CHECK(_stepperMotor->getStatusAndError().error == StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX);
    statusAndError = _stepperMotor->moveToPosition(0);
    BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_ERROR);
    BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX);
    BOOST_CHECK(_motorControlerDummy->getTargetPosition() == 500);
    _stepperMotor->setSoftwareLimitsEnabled(false);


    //trigger internal error during movement
    //std::cout << "Status main 1 " << _stepperMotor->getStatusAndError().status << " Error main " << _stepperMotor->getStatusAndError().error << " POS: "<<_motorControlerDummy->getActualPosition()  << std::endl;
    //BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_ERROR);
    boost::thread workedThread1(&LinearStepperMotorTest::threadMoveToPostion, this, 1);
    statusAndError = _stepperMotor->moveToPosition(0);
    //std::cout << "Status main 2 " << statusAndError.status  << " Error main " << statusAndError.error << " POS: "<<_motorControlerDummy->getActualPosition()  << std::endl;
    BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_ERROR);
    BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX);
    workedThread1.join();

    //test stop blocking function
    boost::thread workedThread2(&LinearStepperMotorTest::threadMoveToPostion, this, 2);
    float targerPosition = _motorControlerDummy->getActualPosition() + 300;
    statusAndError = _stepperMotor->moveToPosition(targerPosition);
    BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_OK);
    BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_ERROR);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() != targerPosition);
    //std::cout << "Status main 2 " << statusAndError.status  << " Error main " << statusAndError.error << " POS: "<<_motorControlerDummy->getActualPosition()  << std::endl;
    workedThread2.join();


    //test when we will succesfuly finish movement
    targerPosition = _motorControlerDummy->getActualPosition() + 300;
    boost::thread workedThread3_1(&LinearStepperMotorTest::threadMoveToPostion, this, 3);
    statusAndError = _stepperMotor->moveToPosition(targerPosition);
    BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_OK);
    BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_ERROR);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == targerPosition);
    //std::cout << "Status main 2 " << statusAndError.status  << " Error main " << statusAndError.error << " POS: "<<_motorControlerDummy->getActualPosition()  << std::endl;
    workedThread3_1.join();



    //test when we will succesfully finish movement but with autostart ON
    _stepperMotor->setAutostart(true);
    targerPosition = _motorControlerDummy->getActualPosition() + 300;
    boost::thread workedThread3_2(&LinearStepperMotorTest::threadMoveToPostion, this, 3);
    statusAndError = _stepperMotor->moveToPosition(targerPosition);
    BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_OK);
    BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_ERROR);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == targerPosition);
    _stepperMotor->setAutostart(false);
    //std::cout << "Status main 2 " << statusAndError.status  << " Error main " << statusAndError.error << " POS: "<<_motorControlerDummy->getActualPosition()  << std::endl;
    workedThread3_2.join();


    //assume we are in the negative end switch
    _motorControlerDummy->setActualPosition(0);
    _stepperMotor->setTargetPosition(10000000);
    _stepperMotor->start();
    _motorControlerDummy->moveTowardsTarget(1, false);
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON);


    //try to move more in positive - should be timeout
    int currentPosition = _stepperMotor->getCurrentPosition();
    boost::thread workedThread4_1(&LinearStepperMotorTest::threadMoveToPostion, this, 4);
    statusAndError = _stepperMotor->moveToPosition(currentPosition + 100);
    workedThread4_1.join();
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == LinearStepperMotorStatusTypes::M_POSITIVE_END_SWITCHED_ON);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == currentPosition);

    boost::thread workedThread4_2(&LinearStepperMotorTest::threadMoveToPostion, this, 4);
    statusAndError = _stepperMotor->moveToPosition(currentPosition - 100);
    workedThread4_2.join();
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == LinearStepperMotorStatusTypes::M_OK);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == (currentPosition - 100));

    //now the same for negative end  switch
    _stepperMotor->setTargetPosition(-10000000);
    _stepperMotor->start();
    _motorControlerDummy->moveTowardsTarget(1, false);
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON);


    //try to move more in negative - should be timeout
    currentPosition = _stepperMotor->getCurrentPosition();
    boost::thread workedThread4_3(&LinearStepperMotorTest::threadMoveToPostion, this, 4);
    statusAndError = _stepperMotor->moveToPosition(currentPosition - 100);
    workedThread4_3.join();
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == LinearStepperMotorStatusTypes::M_NEGATIVE_END_SWITCHED_ON);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == currentPosition);

    boost::thread workedThread4_4(&LinearStepperMotorTest::threadMoveToPostion, this, 4);
    statusAndError = _stepperMotor->moveToPosition(currentPosition + 100);
    workedThread4_4.join();
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == LinearStepperMotorStatusTypes::M_OK);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == (currentPosition + 100));



    //statusAndError = _stepperMotor->moveToPosition(_stepperMotor->getCurrentPosition() + 1000);

    //std::cout << "Status main 3 " << statusAndError.status << " Error main " << statusAndError.error << " POS: "<<_motorControlerDummy->getActualPosition()  << std::endl;
    //std::cout << "Status main 4 " << _stepperMotor->getStatusAndError().status << " Error main " << _stepperMotor->getStatusAndError().error << " POS: "<<_motorControlerDummy->getActualPosition()  << std::endl;
    //std::cout << "Status main 5 " << _stepperMotor->getStatusAndError().status << " Error main " << _stepperMotor->getStatusAndError().error << " POS: " << _motorControlerDummy->getActualPosition() << std::endl;


}

void LinearStepperMotorTest::threadMoveToPostion(int testCase) {


    if (testCase == 1) {
        //std::cout << "Status " << _stepperMotor->getStatusAndError().status << " Error " << _stepperMotor->getStatusAndError().error << std::endl;
        //sleep between threadMoveToPostion start and first command to give time for run moveToPosition
        usleep(10000);
        // simulate the movement
        _motorControlerDummy->moveTowardsTarget(0.4, false);
        //std::cout << "Status " << _stepperMotor->getStatusAndError().status << " Error " << _stepperMotor->getStatusAndError().error << std::endl;
        usleep(10000);
        //change condition to simulate error
        _stepperMotor->setSoftwareLimitsEnabled(true);
        //std::cout << "Status " << _stepperMotor->getStatusAndError().status << " Error " << _stepperMotor->getStatusAndError().error << std::endl;
        //sleep to give time to react on error
        usleep(50000);
        //std::cout << "Status " << _stepperMotor->getStatusAndError().status << " Error " << _stepperMotor->getStatusAndError().error << std::endl;
        //disable the configuration error
        _stepperMotor->setSoftwareLimitsEnabled(false);
        //std::cout << "Status " << _stepperMotor->getStatusAndError().status << " Error " << _stepperMotor->getStatusAndError().error << std::endl;
    }

    if (testCase == 2) {
        //sleep between threadMoveToPostion start and first command to give time for run moveToPosition
        usleep(1000);
        // simulate the movement
        _motorControlerDummy->moveTowardsTarget(0.4, false);
        usleep(10000);
        _stepperMotor->stop();
        usleep(50000);
    }

    if (testCase == 3) {
        //sleep between threadMoveToPostion start and first command to give time for run moveToPosition
        usleep(10000);
        // simulate the movement
        _motorControlerDummy->moveTowardsTarget(0.2, false);
        usleep(10000);
        _motorControlerDummy->moveTowardsTarget(0.4, false);
        usleep(10000);
        _motorControlerDummy->moveTowardsTarget(0.6, false);
        usleep(10000);
        _motorControlerDummy->moveTowardsTarget(0.8, false);
        usleep(10000);
        _motorControlerDummy->moveTowardsTarget(1, false);
        //std::cout << "Status " << _stepperMotor->getStatusAndError().status << " Error " << _stepperMotor->getStatusAndError().error << std::endl;
        usleep(10000);
    }

    if (testCase == 4) {
        //sleep between threadMoveToPostion start and first command to give time for run moveToPosition
        usleep(10000);
        _motorControlerDummy->moveTowardsTarget(1, false);
        usleep(10000);
    }


}

void LinearStepperMotorTest::testCalibration() {
    std::cout << "testCalibration()" << std::endl;

    //check calibration status 
    StepperMotorCalibrationStatus calibrationStatus = _stepperMotor->getCalibrationStatus();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_UNKNOWN);
    _stepperMotor->setEnabled(false);

    _motorControlerDummy->moveTowardsTarget(0.1, true); // enable timeout
    calibrationStatus = _stepperMotor->calibrateMotor();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED);

    //should time out so calibration failed
    _stepperMotor->setEnabled(true);
    calibrationStatus = _stepperMotor->calibrateMotor();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED);

    // try to run second calibration when first is already running
    //in this tread we will try to run second calibration and check its status
    calibrationStatus = _stepperMotor->calibrateMotor();
    boost::thread workedThread1(&LinearStepperMotorTest::threadCalibration, this, 1);
    calibrationStatus = _stepperMotor->calibrateMotor();
    workedThread1.join();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATED);

    //test situation when end switches are not available
    _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
    calibrationStatus = _stepperMotor->calibrateMotor();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE);
    _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
    _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);
    calibrationStatus = _stepperMotor->calibrateMotor();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE);
    _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
    calibrationStatus = _stepperMotor->calibrateMotor();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE);

    //test situation when we have error when both end-switches are ON
    _motorControlerDummy->moveTowardsTarget(0.1, false, true); // in this case both end switches should get active in the same time
    calibrationStatus = _stepperMotor->calibrateMotor();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED);


    //test stop calibration
    _motorControlerDummy->moveTowardsTarget(0); // just to reset flags - _blockMotor and _bothEndSwitchesAlwaysOn
    _motorControlerDummy->setPositiveReferenceSwitchEnabled(true);
    _motorControlerDummy->setNegativeReferenceSwitchEnabled(true);
    boost::thread workedThread2(&LinearStepperMotorTest::threadCalibration, this, 2);
    calibrationStatus = _stepperMotor->calibrateMotor();
    workedThread2.join();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER);

    //successful calibration
    boost::thread workedThread3(&LinearStepperMotorTest::threadCalibration, this, 3);
    calibrationStatus = _stepperMotor->calibrateMotor();
    workedThread3.join();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATED);
    _stepperMotor->setCurrentPositionAs(0);
    BOOST_CHECK((_stepperMotor->getNegativeEndSwitchPosition() * -1) == _stepperMotor->getPositiveEndSwitchPosition());

    //test how emergency stop changes the status of object
    _stepperMotor->emergencyStop();
    BOOST_CHECK(_stepperMotor->getCalibrationStatus() == StepperMotorCalibrationStatusType::M_NOT_CALIBRATED);
    BOOST_CHECK(_stepperMotor->getEnabled() == false);

    //now stop calibration after negative end switch reached
    _stepperMotor->setEnabled(true);
    boost::thread workedThread4(&LinearStepperMotorTest::threadCalibration, this, 4);
    calibrationStatus = _stepperMotor->calibrateMotor();
    workedThread4.join();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER);

    //now stop calibration after negative end switch reached and positive end switch reached
    _motorControlerDummy->moveTowardsTarget(0, false, false, true);
    boost::thread workedThread5(&LinearStepperMotorTest::threadCalibration, this, 5);
    calibrationStatus = _stepperMotor->calibrateMotor();
    workedThread5.join();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_STOPPED_BY_USER);

    //now stop calibration after negative end switch reached and positive end switch reached, but with error trigger - so motor is not calibrated
    boost::thread workedThread6(&LinearStepperMotorTest::threadCalibration, this, 6);
    calibrationStatus = _stepperMotor->calibrateMotor();
    workedThread6.join();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED);


    //now stop calibration after negative end switch reached and positive end switch reached, but with error trigger - so motor is not calibrated
    _motorControlerDummy->moveTowardsTarget(0, false, false, true);
    boost::thread workedThread7(&LinearStepperMotorTest::threadCalibration, this, 7);
    calibrationStatus = _stepperMotor->calibrateMotor();
    workedThread7.join();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_FAILED);


    //do one more time correct calibration
    _motorControlerDummy->moveTowardsTarget(0, false, false, true);
    boost::thread workedThread3a(&LinearStepperMotorTest::threadCalibration, this, 3);
    calibrationStatus = _stepperMotor->calibrateMotor();
    workedThread3a.join();
    BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATED);
    _stepperMotor->setCurrentPositionAs(0);
    BOOST_CHECK((_stepperMotor->getNegativeEndSwitchPosition() * -1) == _stepperMotor->getPositiveEndSwitchPosition());

}

void LinearStepperMotorTest::threadCalibration(int testCase) {
    if (testCase == 1) {
        usleep(1000);
        StepperMotorCalibrationStatus calibrationStatus = _stepperMotor->calibrateMotor();
        BOOST_CHECK(calibrationStatus == StepperMotorCalibrationStatusType::M_CALIBRATION_IN_PROGRESS);
        //now simulate correct calibration
        usleep(1000);
        _motorControlerDummy->moveTowardsTarget(1.0); // go to negative end switch
        usleep(2000);
        _motorControlerDummy->moveTowardsTarget(1.0); // go to positive end switch
        usleep(2000);
        _motorControlerDummy->moveTowardsTarget(1.0); // go to middle of range between end switches
    }

    if (testCase == 2) {
        usleep(1000);
        _stepperMotor->stop(); // this should stop moveToPosition and calibration
    }

    if (testCase == 3) {
        usleep(1000);
        _motorControlerDummy->moveTowardsTarget(1.0); // go to negative end switch
        usleep(2000);
        _motorControlerDummy->moveTowardsTarget(1.0); // go to positive end switch
        usleep(2000);
        _motorControlerDummy->moveTowardsTarget(1.0); // go to middle of range between end switches
    }

    if (testCase == 4) {
        usleep(1000);
        _motorControlerDummy->moveTowardsTarget(1.0); // go to negative end switch
        usleep(2000);
        _stepperMotor->stop();
    }

    if (testCase == 5) {
        usleep(1000);
        _motorControlerDummy->moveTowardsTarget(1.0); // go to negative end switch
        usleep(2000);
        _motorControlerDummy->moveTowardsTarget(1.0); // go to positive end switch
        usleep(1000);
        _stepperMotor->stop();
    }


    if (testCase == 6) {
        usleep(1000);
        _motorControlerDummy->moveTowardsTarget(1.0); // go to negative end switch
        usleep(2000);
        _motorControlerDummy->moveTowardsTarget(1.0); // go to positive end switch
        usleep(2000);
        _motorControlerDummy->moveTowardsTarget(0, false, true); // trigger both end switches error
    }

    if (testCase == 7) {
        usleep(1000);
        _motorControlerDummy->moveTowardsTarget(1.0); // go to negative end switch
        usleep(2000);
        _motorControlerDummy->moveTowardsTarget(0, false, true); // trigger both end switches error
    }

    //_motorControlerDummy->moveTowardsTarget(1, false);
}
