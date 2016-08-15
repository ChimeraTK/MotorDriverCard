
#include <sstream>

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "StepperMotor.h"
#include "MotorControlerDummy.h"
#include "MotorDriverCardFactory.h"
#include "MotorDriverException.h"
#include "TMC429Constants.h"
#include "DFMC_MD22Dummy.h"
#include <mtca4u/DMapFilesParser.h>
#include <mtca4u/Utilities.h>

#include <boost/thread.hpp>

using namespace mtca4u;

//static const unsigned int THE_ID = 17;

static const std::string stepperMotorDeviceName("STEPPER-MOTOR-DUMMY");
static const std::string stepperMotorDeviceConfigFile("VT21-MotorDriverCardConfig.xml");
static const std::string dmapPath(".");
static const std::string moduleName("");
static TMC429OutputWord readDFMCDummyMotor0VMaxRegister(boost::shared_ptr<DFMC_MD22Dummy>& dfmc_md22);
static StallGuardControlData readDFMCDummyMotor0CurrentScale(boost::shared_ptr<DFMC_MD22Dummy>& dfmc_md2);

class TestUnitConveter : public StepperMotorUnitsConverter {
public:

    TestUnitConveter() {
    };

    virtual float stepsToUnits(int steps) {
        return float(steps/10.0);
    };
    virtual int unitsToSteps(float units) {
        return int(10.0*units);
    };
};


class StepperMotorTest
{
 public:
  StepperMotorTest();

  void testSoftLimits();
  void testAutostart();
  void testStart();
  void testStop();
  void testEmergencyStop();
  void testEnabledDisabled();
  void testDebugLevel();
  void testCalibration();
  void testSetPositionAs();
  void testSetGetSpeed();
  
  void testMoveToPosition();  
  void thread(int i);
  
  void testUnitConverter();
  void testGetSetSpeedLimit();
  void testSetGetCurrent();
 
 private:
  boost::shared_ptr<StepperMotor> _stepperMotor;
  boost::shared_ptr<MotorControlerDummy> _motorControlerDummy;
  
  boost::shared_ptr<TestUnitConveter> _testUnitConveter;
  
};

class StepperMotorTestSuite : public test_suite {
public:

    StepperMotorTestSuite() : test_suite("StepperMotor suite") {
        // create an instance of the test class
        boost::shared_ptr<StepperMotorTest> stepperMotorTest(new StepperMotorTest);

        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testSoftLimits,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testAutostart,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testStart,
                stepperMotorTest));
        
        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testEnabledDisabled,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testDebugLevel,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testCalibration,
                stepperMotorTest));          
           
        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testStop,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testEmergencyStop,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testSetPositionAs,
                stepperMotorTest));     

        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testSetGetSpeed,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testMoveToPosition,
                stepperMotorTest));
        
        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testUnitConverter,
                stepperMotorTest)); 

        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testGetSetSpeedLimit,
                stepperMotorTest));

        add(BOOST_CLASS_TEST_CASE(&StepperMotorTest::testSetGetCurrent,
                stepperMotorTest));
      }
};

test_suite*
init_unit_test_suite( int /*argc*/, char* /*argv*/ [] )
{
  framework::master_test_suite().p_name.value = "StepperMotor test suite";
  return new StepperMotorTestSuite;
}

StepperMotorTest::StepperMotorTest() 
  : _stepperMotor(), _motorControlerDummy(), _testUnitConveter()
{
    
    std::string deviceFileName(DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).deviceName);
    std::string mapFileName(DMapFilesParser(dmapPath).getdMapFileElem(stepperMotorDeviceName).mapFileName);

    _testUnitConveter.reset(new TestUnitConveter);

    //_motorControlerDummy = boost::dynamic_pointer_cast<MotorControlerDummy>(MotorDriverCardFactory::instance().createMotorDriverCard(deviceFileName, mapFileName, moduleName, stepperMotorDeviceConfigFile)->getMotorControler(0));
    MotorDriverCardFactory::instance().setDummyMode();
    _motorControlerDummy = boost::dynamic_pointer_cast<MotorControlerDummy>(MotorDriverCardFactory::instance().createMotorDriverCard(deviceFileName,  moduleName, stepperMotorDeviceConfigFile)->getMotorControler(0));
    
    //_stepperMotor.reset(new StepperMotor(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile, dmapPath));
    _stepperMotor.reset(new StepperMotor(stepperMotorDeviceName, moduleName, 0, stepperMotorDeviceConfigFile));
    
    
    //!!!! CHANGE THIS FOR LINEAR STEPER MOTOR TESTS
    _motorControlerDummy->setPositiveReferenceSwitchEnabled(false);
    _motorControlerDummy->setNegativeReferenceSwitchEnabled(false);
}


void StepperMotorTest::testSoftLimits(){
    _stepperMotor->setEnabled(true);

    _stepperMotor->setSoftwareLimitsEnabled(true);
    BOOST_CHECK( _stepperMotor->getSoftwareLimitsEnabled() == true);
    
    _stepperMotor->setSoftwareLimitsEnabled(false);
    BOOST_CHECK( _stepperMotor->getSoftwareLimitsEnabled() == false);  
    
    
    _stepperMotor->setMaxPositionLimit(150);
    _stepperMotor->setMinPositionLimit(50);
    
    BOOST_CHECK( _stepperMotor->getMaxPositionLimit() == 150);  
    BOOST_CHECK( _stepperMotor->getMinPositionLimit() == 50);  
    
    
    _stepperMotor->setSoftwareLimitsEnabled(true);
    
    _stepperMotor->setTargetPosition(200);
    BOOST_CHECK( _stepperMotor->getTargetPosition() == 150 );
    
    _stepperMotor->setTargetPosition(0);
    BOOST_CHECK( _stepperMotor->getTargetPosition() == 50 );
    
    _stepperMotor->setTargetPosition(100);
    BOOST_CHECK( _stepperMotor->getTargetPosition() == 100 );
    
    _stepperMotor->setSoftwareLimitsEnabled(false);
    
    _stepperMotor->setTargetPosition(200);
    BOOST_CHECK( _stepperMotor->getTargetPosition() == 200 );
    
    _stepperMotor->setTargetPosition(0);
    BOOST_CHECK( _stepperMotor->getTargetPosition() == 0 );
    
    _stepperMotor->setTargetPosition(100);
    BOOST_CHECK( _stepperMotor->getTargetPosition() == 100 );    
    
    
    //should trigger error state - target position should not be changed
    _stepperMotor->setMaxPositionLimit(50);
    _stepperMotor->setMinPositionLimit(150);
    
    _stepperMotor->setSoftwareLimitsEnabled(true);

    _stepperMotor->setTargetPosition(200);
    BOOST_CHECK( _stepperMotor->getTargetPosition() == 100 );
    
    _stepperMotor->setTargetPosition(0);
    BOOST_CHECK( _stepperMotor->getTargetPosition() == 100 );
    
    _stepperMotor->setTargetPosition(100);
    BOOST_CHECK( _stepperMotor->getTargetPosition() == 100 );
    
    _stepperMotor->setSoftwareLimitsEnabled(false);
    
    _stepperMotor->setTargetPosition(200);
    BOOST_CHECK( _stepperMotor->getTargetPosition() == 200 );
    
    _stepperMotor->setTargetPosition(0);
    BOOST_CHECK( _stepperMotor->getTargetPosition() == 0 );
    
    _stepperMotor->setTargetPosition(100);
    BOOST_CHECK( _stepperMotor->getTargetPosition() == 100 );
    

    _stepperMotor->setMaxPositionLimit(150);
    _stepperMotor->setMinPositionLimit(50);
    _stepperMotor->setTargetPosition(200);
    _stepperMotor->setSoftwareLimitsEnabled(true);
    
    BOOST_CHECK ( (_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON) == true);
    
    _stepperMotor->setSoftwareLimitsEnabled(false);
    _stepperMotor->setTargetPosition(0);
    _stepperMotor->setSoftwareLimitsEnabled(true);
    
    BOOST_CHECK ( (_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_SOFT_NEGATIVE_END_SWITCHED_ON) == true);
    
    _stepperMotor->setSoftwareLimitsEnabled(false);
    _stepperMotor->setEnabled(false);
}

void StepperMotorTest::testAutostart() {
    _stepperMotor->setEnabled(true);
    
    _stepperMotor->setAutostart(true);
     BOOST_CHECK ( _stepperMotor->getAutostart() == true);
    
    _stepperMotor->setAutostart(false);
     BOOST_CHECK ( _stepperMotor->getAutostart() == false);
     
    
    int initTargetPosition = _motorControlerDummy->getTargetPosition();
    _stepperMotor->setAutostart(false);
    _stepperMotor->setTargetPosition(1000);
    BOOST_CHECK( _motorControlerDummy->getTargetPosition() == initTargetPosition );
    
    
    _stepperMotor->setAutostart(true);
    BOOST_CHECK( _motorControlerDummy->getTargetPosition() == 1000 );
     
    
    _stepperMotor->setTargetPosition(2000);
    BOOST_CHECK( _motorControlerDummy->getTargetPosition() == 2000 );
    
    _stepperMotor->setTargetPosition(-2000);
    BOOST_CHECK( _motorControlerDummy->getTargetPosition() == -2000 );
    
    _stepperMotor->setAutostart(false);
}


void StepperMotorTest::testStart() {

    
    _motorControlerDummy->setTargetPosition(0);
    _motorControlerDummy->setActualPosition(0);
    
    
    _stepperMotor->setEnabled(true);
    _stepperMotor->setAutostart(false);

    //test start function
    _stepperMotor->setTargetPosition(5000);
    BOOST_CHECK(_motorControlerDummy->getTargetPosition() == 0);
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_NOT_IN_POSITION);
    
    
    _stepperMotor->start();
    BOOST_CHECK(_motorControlerDummy->getTargetPosition() == (5000));
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_IN_MOVE);    
    _motorControlerDummy->moveTowardsTarget(0.5);
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_IN_MOVE);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == (5000*0.5));
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    

    _motorControlerDummy->moveTowardsTarget(1);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == (5000));
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_OK);

    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_OK);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getTargetPosition()); 

}


void StepperMotorTest::testStop() {
    _motorControlerDummy->setTargetPosition(0);
    _motorControlerDummy->setActualPosition(0);

    _stepperMotor->setEnabled(true);
    
    _stepperMotor->setAutostart(false);

    //test start function
    _stepperMotor->setTargetPosition(5000);
    BOOST_CHECK( _stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_NOT_IN_POSITION);
    _stepperMotor->start();
    BOOST_CHECK(_motorControlerDummy->getTargetPosition() == (5000));
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_IN_MOVE);
    _motorControlerDummy->moveTowardsTarget(0.5);
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_IN_MOVE);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == (5000 * 0.5));
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    
    _motorControlerDummy->moveTowardsTarget(0.5);
    _stepperMotor->stop();
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_OK);   
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == (5000 * 0.75));
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getTargetPosition());      

}

void StepperMotorTest::testEmergencyStop() {
    _motorControlerDummy->setTargetPosition(0);
    _motorControlerDummy->setActualPosition(0);

    _stepperMotor->setEnabled(true);
    
    _stepperMotor->setAutostart(false);

    //test start function
    _stepperMotor->setTargetPosition(5000);
    BOOST_CHECK ( _stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_NOT_IN_POSITION);
    _stepperMotor->start();
    BOOST_CHECK(_motorControlerDummy->getTargetPosition() == (5000));
    BOOST_CHECK ( _stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_IN_MOVE);
    _motorControlerDummy->moveTowardsTarget(0.5);
    BOOST_CHECK ( _stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_IN_MOVE);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == (5000 * 0.5));
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    
    _motorControlerDummy->moveTowardsTarget(0.5);
    _stepperMotor->emergencyStop();
    BOOST_CHECK ( _stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_DISABLED);   
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == (5000 * 0.75));
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getTargetPosition());
    BOOST_CHECK(_motorControlerDummy->getTargetPosition() == _motorControlerDummy->getActualPosition());

}


void StepperMotorTest::testEnabledDisabled() {
    _stepperMotor->setEnabled(true);
    
    _stepperMotor->setSoftwareLimitsEnabled(false);
    _stepperMotor->setTargetPosition(_motorControlerDummy->getActualPosition());
    
    BOOST_CHECK(_motorControlerDummy->isEnabled() == true);
    BOOST_CHECK(_stepperMotor->getEnabled() == true);
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_OK);
    
    _stepperMotor->setEnabled(false);
    
    BOOST_CHECK(_motorControlerDummy->isEnabled() == false);
    BOOST_CHECK(_stepperMotor->getEnabled() == false);
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_DISABLED);
    
}


void StepperMotorTest::testDebugLevel() {
    _stepperMotor->setLogLevel(Logger::NO_LOGGING);
    BOOST_CHECK( _stepperMotor->getLogLevel() == Logger::NO_LOGGING );

    _stepperMotor->setLogLevel(Logger::ERROR);
    BOOST_CHECK(_stepperMotor->getLogLevel() == Logger::ERROR);

    _stepperMotor->setLogLevel(Logger::WARNING);
    BOOST_CHECK(_stepperMotor->getLogLevel() == Logger::WARNING);

    _stepperMotor->setLogLevel(Logger::INFO);
    BOOST_CHECK(_stepperMotor->getLogLevel() == Logger::INFO);

    _stepperMotor->setLogLevel(Logger::DETAIL);
    BOOST_CHECK(_stepperMotor->getLogLevel() == Logger::DETAIL);

    _stepperMotor->setLogLevel(Logger::FULL_DETAIL);
    BOOST_CHECK(_stepperMotor->getLogLevel() == Logger::FULL_DETAIL);
    
    _stepperMotor->setLogLevel(Logger::NO_LOGGING);
    
}

void StepperMotorTest::testCalibration() {
    BOOST_CHECK(_stepperMotor->getCalibrationStatus() == StepperMotorCalibrationStatusType::M_NOT_CALIBRATED);
    _stepperMotor->calibrateMotor();
    BOOST_CHECK(_stepperMotor->getCalibrationStatus() == StepperMotorCalibrationStatusType::M_CALIBRATION_NOT_AVAILABLE);
}

void StepperMotorTest::testSetPositionAs() {
    _motorControlerDummy->setTargetPosition(0);
    _motorControlerDummy->setActualPosition(0);

    _stepperMotor->setAutostart(false);

    _stepperMotor->setEnabled(true);
    _stepperMotor->setCurrentPositionAs(5000);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == 5000);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getTargetPosition());
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_OK);
    

    _stepperMotor->setEnabled(false);
    _stepperMotor->setCurrentPositionAs(-2000);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == -2000);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getActualPosition());
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == _motorControlerDummy->getTargetPosition());
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_DISABLED);
}

void StepperMotorTest::testSetGetSpeed() {
    BOOST_CHECK_THROW(_stepperMotor->setUserSpeedLimit(100), mtca4u::MotorDriverException);
    BOOST_CHECK_THROW(_stepperMotor->getUserSpeedLimit(), mtca4u::MotorDriverException);
}

void StepperMotorTest::testMoveToPosition() {
    _motorControlerDummy->resetInternalStateToDefaults();

    _stepperMotor->setAutostart(false);
    _stepperMotor->setEnabled(true);
    
    //set motor in error state - configuration error end test routine
    auto currentPosition = _stepperMotor->getCurrentPosition();
    _stepperMotor->setMaxPositionLimit(50);
    _stepperMotor->setMinPositionLimit(100);
    _stepperMotor->setSoftwareLimitsEnabled(true);
    BOOST_CHECK(_stepperMotor->getStatusAndError().status == StepperMotorStatusTypes::M_ERROR);
    BOOST_CHECK(_stepperMotor->getStatusAndError().error== StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX);
    auto statusAndError =  _stepperMotor->moveToPosition(0);
    BOOST_CHECK( statusAndError.status == StepperMotorStatusTypes::M_ERROR);
    BOOST_CHECK( statusAndError.error == StepperMotorErrorTypes::M_CONFIG_ERROR_MIN_POS_GRATER_EQUAL_TO_MAX);
    BOOST_CHECK(_motorControlerDummy->getTargetPosition() == currentPosition);
    _stepperMotor->setSoftwareLimitsEnabled(false);
    
    

    currentPosition = _stepperMotor->getCurrentPosition();
    _stepperMotor->setMaxPositionLimit(currentPosition - 100);
    _stepperMotor->setMinPositionLimit(currentPosition - 200);
    _stepperMotor->setSoftwareLimitsEnabled(true);
    boost::thread workerThread1(&StepperMotorTest::thread, this, 1);
    statusAndError = _stepperMotor->moveToPosition(currentPosition + 100);
    workerThread1.join();
    BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_SOFT_POSITIVE_END_SWITCHED_ON);
    BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_ERROR);
    BOOST_CHECK(_motorControlerDummy->getTargetPosition() == currentPosition - 100);
    _stepperMotor->setSoftwareLimitsEnabled(false);

    
    //test stop blocking function
    boost::thread workedThread2(&StepperMotorTest::thread, this, 2);
    float targerPosition = _motorControlerDummy->getActualPosition() + 3000;
    statusAndError = _stepperMotor->moveToPosition(targerPosition);
    BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_OK);
    BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_ERROR);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() != targerPosition);
    workedThread2.join();    
    
    
    //test when we will succesfully finish movement
    targerPosition = _motorControlerDummy->getActualPosition() + 3000;
    boost::thread workedThread3_1(&StepperMotorTest::thread, this, 1);
    statusAndError = _stepperMotor->moveToPosition(targerPosition);
    BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_OK);
    BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_ERROR);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == targerPosition);
    workedThread3_1.join(); 
    
    

    //test when we will succesfully finish movement but with autostart ON
    _stepperMotor->setAutostart(true);
    targerPosition = _motorControlerDummy->getActualPosition() + 3000;
    boost::thread workedThread3_2(&StepperMotorTest::thread, this, 1);
    statusAndError = _stepperMotor->moveToPosition(targerPosition);
    BOOST_CHECK(statusAndError.status == StepperMotorStatusTypes::M_OK);
    BOOST_CHECK(statusAndError.error == StepperMotorErrorTypes::M_NO_ERROR);
    BOOST_CHECK(_stepperMotor->getCurrentPosition() == targerPosition);
    _stepperMotor->setAutostart(false);
    workedThread3_2.join(); 
    
}

void StepperMotorTest::thread(int testCase) {

  if (testCase == 1) {
    
    // do not proceed till the parent thread has asked the moter to move
    if (_stepperMotor->isMoving() == false) {
      usleep(10000);
    }
    // simulate the movement
    _motorControlerDummy->moveTowardsTarget(1);
  }

  if (testCase == 2) {
    // do not proceed till the parent thread has asked the moter to move
    if (_stepperMotor->isMoving() == false) {
      usleep(10000);
    }
    // simulate the movement
    _motorControlerDummy->moveTowardsTarget(0.4);
    _stepperMotor->stop();
    _motorControlerDummy->moveTowardsTarget(1);
  }
}

void StepperMotorTest::testUnitConverter() {

    
    _stepperMotor->setStepperMotorUnitsConverter(_testUnitConveter);
    
    BOOST_CHECK(_stepperMotor->recalculateUnitsToSteps(10) == 100);
    BOOST_CHECK(_stepperMotor->recalculateUnitsToSteps(-10) == -100);
    BOOST_CHECK(_stepperMotor->recalculateUnitsToSteps(10.5) == 105);
    BOOST_CHECK(_stepperMotor->recalculateUnitsToSteps(-10.5) == -105);
    
    BOOST_CHECK(_stepperMotor->recalculateStepsToUnits(100) == 10);
    BOOST_CHECK(_stepperMotor->recalculateStepsToUnits(-100) == -10);
    BOOST_CHECK(_stepperMotor->recalculateStepsToUnits(105) == 10.5);
    BOOST_CHECK(_stepperMotor->recalculateStepsToUnits(-105) == -10.5);
    
    
    _testUnitConveter.reset();
    _stepperMotor->setStepperMotorUnitsConverter(_testUnitConveter);
    
    BOOST_CHECK(_stepperMotor->recalculateUnitsToSteps(10) == 10);
    BOOST_CHECK(_stepperMotor->recalculateUnitsToSteps(-10) == -10);
    BOOST_CHECK(_stepperMotor->recalculateUnitsToSteps(10.5) == 10);
    BOOST_CHECK(_stepperMotor->recalculateUnitsToSteps(-10.5) == -10);
    
    BOOST_CHECK(_stepperMotor->recalculateStepsToUnits(100) == 100);
    BOOST_CHECK(_stepperMotor->recalculateStepsToUnits(-100) == -100);
    BOOST_CHECK(_stepperMotor->recalculateStepsToUnits(105) == 105);
    BOOST_CHECK(_stepperMotor->recalculateStepsToUnits(-105) == -105);
}

void StepperMotorTest::testGetSetSpeedLimit() {
  // setup
  auto dmapFile = mtca4u::getDMapFilePath();
  mtca4u::setDMapFilePath("./dummies.dmap");
  auto dummyModeStatus = MotorDriverCardFactory::instance().getDummyMode();
  MotorDriverCardFactory::instance().setDummyMode(false);
  StepperMotor motor("DFMC_MD22_PERSISTENT_BACKEND", "MD22_0", 0, "custom_speed_and_curruent_limits.xml");


  // Expected max speed  capability in this case:[from custom_speed_and_curruent_limits.xml]
  //   Vmax = 1398 (0x576)
  //   fclk = 32000000
  //   pulse_div = 6
  // expectedMaxSpeedLimitInUstepsPerSecond = Vmax * fclk/((2^pulse_div) * 2048 * 32)
  //
  auto expectedMaxSpeedLimit = (1398.0 * 32000000.0) / (exp2(6) * 2048 * 32);

  // expectedMinimumSpeedLimitInUstepsPerSec = Vmin * fclk/((2^pulse_div) * 2048 * 32)
  auto expectedMinimumSpeed = (1.0 * 32000000.0) / // VMin = 1 (1.0)
                              (exp2(6) * 2048 * 32);

  //verify getMaxSpeedCapability
  BOOST_CHECK(motor.getMaxSpeedCapability() == expectedMaxSpeedLimit);

  // set speed > MaxAllowedSpeed
  BOOST_CHECK(motor.setUserSpeedLimit(expectedMaxSpeedLimit) == expectedMaxSpeedLimit);
  BOOST_CHECK(motor.getUserSpeedLimit() == expectedMaxSpeedLimit);
  BOOST_CHECK(motor.setUserSpeedLimit(expectedMaxSpeedLimit + 10) == expectedMaxSpeedLimit);
  BOOST_CHECK(motor.getUserSpeedLimit() == expectedMaxSpeedLimit);

  // set speed < MinAllowedSpeed
  BOOST_CHECK(motor.setUserSpeedLimit(expectedMinimumSpeed) == expectedMinimumSpeed);
  BOOST_CHECK(motor.getUserSpeedLimit() == expectedMinimumSpeed);
  BOOST_CHECK(motor.setUserSpeedLimit(expectedMinimumSpeed - 10) == expectedMinimumSpeed);
  BOOST_CHECK(motor.getUserSpeedLimit() == expectedMinimumSpeed);
  BOOST_CHECK(motor.setUserSpeedLimit(-100) == expectedMinimumSpeed);
  BOOST_CHECK(motor.getUserSpeedLimit() == expectedMinimumSpeed);

  // set speed in valid range [Vmin, Vmax]
  // speed = 8000 microsteps per second -> results in Vmax = 1048.576
  // Vmax 1048.576  floors to 1048 => which gives returned/actual set speed as
  // 7995.6054... microsteps per second
  auto expectedSetSpeed = (1048.0 * 32000000.0) / (exp2(6) * 2048 * 32);
  BOOST_CHECK(motor.setUserSpeedLimit(8000) == expectedSetSpeed);
  BOOST_CHECK(motor.getUserSpeedLimit() == expectedSetSpeed);

  // Verify that Vmax is 1048 in the internal register (which is the
  // VMax corresponding to expectedSetSpeed)
  auto md22_instance = boost::dynamic_pointer_cast<DFMC_MD22Dummy>(
      BackendFactory::getInstance().createBackend(
          "DFMC_MD22_PERSISTENT_BACKEND"));
  BOOST_ASSERT(md22_instance != NULL);
  TMC429OutputWord vMaxRegister = readDFMCDummyMotor0VMaxRegister(md22_instance);
 BOOST_CHECK(vMaxRegister.getDATA() == 1048);

  //teardown
    MotorDriverCardFactory::instance().setDummyMode(dummyModeStatus);
    mtca4u::setDMapFilePath(dmapFile);
}

void StepperMotorTest::testSetGetCurrent() {
  auto dmapFile = mtca4u::getDMapFilePath();
  mtca4u::setDMapFilePath("./dummies.dmap");
  auto dummyModeStatus = MotorDriverCardFactory::instance().getDummyMode();
  MotorDriverCardFactory::instance().setDummyMode(false);
  StepperMotor motor("DFMC_MD22_PERSISTENT_BACKEND", "MD22_0", 0, "custom_speed_and_curruent_limits.xml");

  // The motor has been configured for a maximum current of .24 Amps, with 1.8A
  // being the maximum the driver ic can provide. This translates to a current
  // scale according to the formula:
  // (.24 * 32)/1.8 - 1 == 3.266
  // the 32 in the above formula comes from the 32 current scale levels for the
  // tmc260.
  // The tmc260 current scale register takes a whole number. So floor(3.266)
  // goes into the current scale register. Because of this we get an effective
  // current limit of:
  // 1.8 * (3 + 1)/32 == .225 A

  auto expectedSafeCurrentLimitValue = 1.8 * (4.0)/32.0;
  auto safeCurrentLimit = motor.getSafeCurrentLimit();
  BOOST_CHECK(safeCurrentLimit == expectedSafeCurrentLimitValue);

  // set Current to Beyond the safe limit
  BOOST_CHECK(motor.setUserCurrentLimit(safeCurrentLimit + 10) == safeCurrentLimit);
  BOOST_CHECK(motor.getUserCurrentLimit() == safeCurrentLimit);
  BOOST_CHECK(motor.setUserCurrentLimit(safeCurrentLimit) == safeCurrentLimit);
  BOOST_CHECK(motor.getUserCurrentLimit() == safeCurrentLimit);

  // set a current below minimum value.
  // Minimum value of current (when current scale is 0):
  // 1.8 * (0 + 1)/32 == .05625A
  auto minimumCurrent = 1.8 * 1.0/32.0;
  BOOST_CHECK(motor.setUserCurrentLimit(0) == minimumCurrent);
  BOOST_CHECK(motor.getUserCurrentLimit() == minimumCurrent);
  BOOST_CHECK(motor.setUserCurrentLimit(-10) == minimumCurrent);
  BOOST_CHECK(motor.getUserCurrentLimit() == minimumCurrent);

  // Current in a valid range: 0 - .24A
  // I = .2A
  // CS = (.20 * 32)/1.8 - 1 = floor(2.556) = 2
  // For CS 2: Expected current = (1.8 * (2 + 1))/32 = .16875A
  auto expectedCurrent = (1.8 * (2 + 1))/32.0;
  BOOST_CHECK(motor.setUserCurrentLimit(.2) == expectedCurrent);
  BOOST_CHECK(motor.getUserCurrentLimit() == expectedCurrent);
  // Verify that CS is 2 in the internal register ()
  auto md22_instance = boost::dynamic_pointer_cast<DFMC_MD22Dummy>(
      BackendFactory::getInstance().createBackend("DFMC_MD22_PERSISTENT_BACKEND"));
  BOOST_ASSERT(md22_instance != NULL);
  StallGuardControlData stallGuard = readDFMCDummyMotor0CurrentScale(md22_instance);
  BOOST_CHECK(stallGuard.getCurrentScale() == 2);

  //teardown
    MotorDriverCardFactory::instance().setDummyMode(dummyModeStatus);
    mtca4u::setDMapFilePath(dmapFile);
}


/*
void MotorControlerDummyTest::testGetID(){
  BOOST_CHECK( _motorControlerDummy.getID() == THE_ID );
}

void MotorControlerDummyTest::testIsSetEnabled(){
  // motor should be disabled now
  BOOST_CHECK(  _motorControlerDummy.isEnabled() == false );
  // just a small test that the motor does not move
  // move to somewhere way beyond the end swith. The current position should go
there.
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 10000 );
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 0 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 0 );

  _motorControlerDummy.setTargetPosition(2000000);
  // motor must not be moving
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 0 );
  _motorControlerDummy.moveTowardsTarget(1);
  // real position has not changed, target positon reached
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 10000 );
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 2000000 );

  // reset the positons and enable the motor. All other tests are done with
motor
  // activated.
  _motorControlerDummy.setTargetPosition(0);
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setEnabled();
  // check that on/of works as expected
  BOOST_CHECK(  _motorControlerDummy.isEnabled() );
  _motorControlerDummy.setEnabled(false);
  BOOST_CHECK(  _motorControlerDummy.isEnabled() == false );
  _motorControlerDummy.setEnabled();

}

void MotorControlerDummyTest::testGetSetTargetPosition(){
  // upon initialisation all positions must be 0
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 0);
  BOOST_CHECK( _motorControlerDummy.targetPositionReached() );
  _motorControlerDummy.setTargetPosition(1234);
  BOOST_CHECK( _motorControlerDummy.targetPositionReached() == false );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 1234);
  // set it back to 0 to simplify further tests
  _motorControlerDummy.setTargetPosition(0);
  BOOST_CHECK( _motorControlerDummy.targetPositionReached() );
}

void MotorControlerDummyTest::testGetSetActualPosition(){
  // upon initialisation all positions must be 0
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 0);
  _motorControlerDummy.setActualPosition(1000);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 1000);
}

void MotorControlerDummyTest::testGetSetActualVelocity(){
  // we strongly depent on the order of the execution here. actual position is
1000,
  // while the target position is 0, to we expect a negative velocity
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == -25 );

  _motorControlerDummy.setActualPosition(0);
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 0 );

  _motorControlerDummy.setActualPosition(-1000);
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 25 );

  // whitebox test: as the decision in getActualVelocity() directly depends on
  // the private isMoving, this is the ideal case to test it.

  // move to the positive end switch. Although the target positon is not reached
the
  // motor must not move any more
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setTargetPosition(200000);
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 25 );
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 0 );

  _motorControlerDummy.setTargetPosition(-200000);
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == -25 );
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 0 );

  BOOST_CHECK_THROW(_motorControlerDummy.setActualVelocity(0),
MotorDriverException);
}

void MotorControlerDummyTest::testGetSetActualAcceleration(){
  BOOST_CHECK( _motorControlerDummy.getActualAcceleration() == 0 );
  BOOST_CHECK_THROW(_motorControlerDummy.setActualAcceleration(0),
                    MotorDriverException);
}

void MotorControlerDummyTest::testGetSetMicroStepCount(){
  // Set the actual position to something positive. The micro step count is
  // positive and identical to the actual count.
  // Does this make sense? Check the data sheet.
  _motorControlerDummy.setActualPosition(5000);
  BOOST_CHECK( _motorControlerDummy.getMicroStepCount() == 5000 );
  BOOST_CHECK_THROW(_motorControlerDummy.setMicroStepCount(0),
                    MotorDriverException);
}


void MotorControlerDummyTest::testGetStatus(){
  BOOST_CHECK_THROW(_motorControlerDummy.getStatus(), MotorDriverException);
}

void MotorControlerDummyTest::testGetSetDecoderReadoutMode(){
  BOOST_CHECK(_motorControlerDummy.getDecoderReadoutMode() ==
              MotorControler::DecoderReadoutMode::HEIDENHAIN);
  BOOST_CHECK_THROW(_motorControlerDummy.setDecoderReadoutMode(MotorControler::DecoderReadoutMode::INCREMENTAL),
                    MotorDriverException);
}

void MotorControlerDummyTest::testGetDecoderPosition(){
  // we are still at the negative end switch
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 0 );
  // actual position is 5000, move into the absolute positive range
  _motorControlerDummy.setTargetPosition(17000); // 12000 steps
  _motorControlerDummy.moveTowardsTarget(1);

  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 12000 );
}

void MotorControlerDummyTest::testReferenceSwitchData(){
  MotorReferenceSwitchData motorReferenceSwitchData =
    _motorControlerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() == false);
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchBit() == 0 );

  // move to the positive end switch and recheck
  _motorControlerDummy.setTargetPosition(
_motorControlerDummy.getActualPosition()
                                          + 2000000);
  _motorControlerDummy.moveTowardsTarget(1);
  motorReferenceSwitchData = _motorControlerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() );
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() == false);
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchBit() == 1 );

  // deactivate the positive end switch. Although it is still active in
hardware,
  // this should not be reported
  _motorControlerDummy.setPositiveReferenceSwitchEnabled(false);
  motorReferenceSwitchData = _motorControlerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() == false );
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() == false);
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchBit() == 0 );

  // reenable the switch and to the same for the negative switch
  _motorControlerDummy.setPositiveReferenceSwitchEnabled(true);
  _motorControlerDummy.setTargetPosition(
_motorControlerDummy.getActualPosition()
                                          - 2000000);
  _motorControlerDummy.moveTowardsTarget(1);
  motorReferenceSwitchData = _motorControlerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled());
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() == false);
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() );
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchBit() == 1 );

  // deactivate the positive end switch. Although it is still active in
hardware,
  // this should not be reported
  _motorControlerDummy.setNegativeReferenceSwitchEnabled(false);
  motorReferenceSwitchData = _motorControlerDummy.getReferenceSwitchData();
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchEnabled() );
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchEnabled() == false );
  BOOST_CHECK(motorReferenceSwitchData.getPositiveSwitchActive() == false );
  BOOST_CHECK(motorReferenceSwitchData.getNegativeSwitchActive() == false);
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchBit() == 0 );

  // reenable the switch and to the same for the negative switch
  _motorControlerDummy.setNegativeReferenceSwitchEnabled(true);
}

void MotorControlerDummyTest::testMoveTowardsTarget(){
  // ok, we are at the negative end switch. reset target and actual position to
0
  // for a clean test environment.
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setTargetPosition(0);

  // move to a target position in the range in multiple steps
  _motorControlerDummy.setTargetPosition(4000);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 0 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 4000 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 0 );

  _motorControlerDummy.moveTowardsTarget(0.25);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 1000 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 4000 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 1000 );

  _motorControlerDummy.moveTowardsTarget(0.5);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 2500 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 4000 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 2500 );

  // moving backwards works differently, this does nothing
  _motorControlerDummy.moveTowardsTarget(-0.25);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 2500 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 4000 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 2500 );

  // moving further than the target position also does not work
  _motorControlerDummy.moveTowardsTarget(2);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 4000 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 4000 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 4000 );

  // ok, let's recalibrate and do the stuff backwards
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setTargetPosition(-400); // we intentionally do not move
all the
  // way back to the end switch
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 0 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == -400 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 4000 );

  // this moves backwards because the target position is smaller than the actual
one
  _motorControlerDummy.moveTowardsTarget(0.25);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == -100 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == -400 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 3900 );

  _motorControlerDummy.moveTowardsTarget(0.5);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == -250 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == -400 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 3750 );

  // moving backwards works differently, this does nothing
  _motorControlerDummy.moveTowardsTarget(-0.25);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == -250 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == -400 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 3750 );

  // moving further than the target position also does not work
  _motorControlerDummy.moveTowardsTarget(2);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == -400 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == -400 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 3600 );

  // now check that the end switches are handled correctly
  // move to the middle and recalibrate for easier readability

  _motorControlerDummy.setTargetPosition(6000);
  _motorControlerDummy.moveTowardsTarget(1);
  _motorControlerDummy.setActualPosition(0);
  _motorControlerDummy.setTargetPosition(0);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 0 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 0 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 10000 );

  // set a target position outside the end switches and see what happes
  _motorControlerDummy.setTargetPosition(11000);
  // only move half the way, should be fine
  _motorControlerDummy.moveTowardsTarget(0.5);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 5500 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 11000 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 15500 );
  // move the rest, it should stop at the end switch and not be moving any more
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 10000 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 11000 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 20000 );
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 0 );
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchData().getPositiveSwitchActive()
);

  // Move back, deactivate the end switch and try again
  // (Don't try this at home. It will break your hardware!)
  _motorControlerDummy.setTargetPosition(5500);
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 5500 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 5500 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 15500 );

  _motorControlerDummy.setPositiveReferenceSwitchEnabled(false);
  _motorControlerDummy.setTargetPosition(11000);
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 11000 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == 11000 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 21000 );

  // now the same for the negative end switch
  // set a target position outside the end switches and see what happes
  _motorControlerDummy.setTargetPosition(-11000);
  // only move half the way, should be fine
  _motorControlerDummy.moveTowardsTarget(0.5);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == 0 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == -11000 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 10000 );
  // move the rest, it should stop at the end switch and not be moving any more
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == -10000 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == -11000 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 0 );
  BOOST_CHECK( _motorControlerDummy.getActualVelocity() == 0 );
  BOOST_CHECK(_motorControlerDummy.getReferenceSwitchData().getNegativeSwitchActive()
);

  // Move back, deactivate the end switch and try again
  // (Don't try this at home. It will break your hardware!)
  _motorControlerDummy.setTargetPosition(-5500);
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == -5500 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == -5500 );
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition() == 4500 );

  _motorControlerDummy.setNegativeReferenceSwitchEnabled(false);
  _motorControlerDummy.setTargetPosition(-11000);
  _motorControlerDummy.moveTowardsTarget(1);
  BOOST_CHECK( _motorControlerDummy.getActualPosition() == -11000 );
  BOOST_CHECK( _motorControlerDummy.getTargetPosition() == -11000 );
  // the decoder delivers a huge value because it is unsigned. We broke the
hardware
  // anyway ;-) Negative positons are not possible.
  BOOST_CHECK( _motorControlerDummy.getDecoderPosition()
               == static_cast<unsigned int>(-1000) );
}
*/


TMC429OutputWord readDFMCDummyMotor0VMaxRegister(boost::shared_ptr<DFMC_MD22Dummy>& dfmc_md22){
  TMC429InputWord readVmaxRegister;
  readVmaxRegister.setSMDA(0);
  readVmaxRegister.setIDX_JDX(tmc429::IDX_MAXIMUM_VELOCITY);
  readVmaxRegister.setRW(tmc429::RW_READ);
  return TMC429OutputWord(
      dfmc_md22->readTMC429Register(readVmaxRegister.getDataWord()));
}


StallGuardControlData readDFMCDummyMotor0CurrentScale(boost::shared_ptr<DFMC_MD22Dummy>& dfmc_md2){
  auto motorId = 0;
  return StallGuardControlData(dfmc_md2->readTMC260Register(
      motorId, DFMC_MD22Dummy::TMC260Register::STALLGUARD2_CONTROL_REGISTER));
}
