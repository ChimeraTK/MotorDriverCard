#include <iostream>

#include "MotorDriverCardFactory.h"
#include "MotorDriverCardImpl.h"
#include "MotorControlerExpert.h"
#include <mtca4u/DMapFileParser.h>
#include <mtca4u/BackendFactory.h>
#include <unistd.h>
#include <ctime>

using namespace mtca4u;

// We use a macro so we can print the failing condition.
// Counts up the error counter if TEST_CONDITION is false and prints an error message
#define PRINT_AND_COUNT_ERRORS( TEST_CONDITION , MOTOR_ID )	\
  if ( !(TEST_CONDITION) ){ \
  ++errorCounter; \
  std::cout << "\n An error occured evaluating " << #TEST_CONDITION \
  << " with motorID = " << MOTOR_ID << " in test cycle " << runCounter <<"!"<<  std::endl; \
  }

#define MASK24( INPUT ) ( (INPUT) & 0xFFFFFF )

int main( int argc, char* argv[] )
{
  if (argc !=4){
    std::cout <<  argv[0] << "This is a long term test which performs SPI communication to the controler"
	      << " and driver chips by writing to the corresponding PCIe regsiters.\n"
	      << "It performs evaluation by reading back the written values.\n"
	      << "The programme uses the first device listed in the given DMap file.\n\n"
	      << "The test runs forever. Terminate it with <Ctrl><c>\n\n"
	      << "Usage: " << argv[0] << " dmapFile moduleName motorConfig.xml" << std::endl;
    return -1;
  }

  // get the first device alias from the dmap file and tell the factory to use this dmap file
  std::string dmapFileName = argv[1];
  std::string deviceAlias = DMapFileParser().parse( dmapFileName )->begin()->dev_name;
  BackendFactory::getInstance().setDMapFilePath( dmapFileName );

  std::string moduleName = argv[2];
  std::string motorConfig = argv[3];
  boost::shared_ptr<mtca4u::MotorDriverCard> motorDriverCard
    = mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard( 
    		deviceAlias,
                moduleName,
		motorConfig); // default motor config

  boost::shared_ptr<mtca4u::MotorControlerExpert> motor0 
    = boost::dynamic_pointer_cast<mtca4u::MotorControlerExpert>
                                               (motorDriverCard->getMotorControler(0));
  boost::shared_ptr<mtca4u::MotorControlerExpert> motor1
    = boost::dynamic_pointer_cast<mtca4u::MotorControlerExpert>
                                               (motorDriverCard->getMotorControler(1));
  
  uint64_t runCounter = 0;
  uint64_t errorCounter = 0;
  time_t startingTime = time(NULL);
  time_t lastPrintTime = startingTime;

  int initialTargetPosition[2] = {motor0->getTargetPosition(), motor1->getTargetPosition()};
  unsigned int initialMinVelocity[2] = {motor0->getMinimumVelocity(), motor1->getMinimumVelocity()};
  unsigned int initialMaxVelocity[2] = {motor0->getMaximumVelocity(), motor1->getMaximumVelocity()};
  unsigned int initialMaxAcceleration[2] = {motor0->getMaximumAcceleration(), motor1->getMaximumAcceleration()};
  unsigned int initialPositionTolerance[2] = {motor0->getPositionTolerance(), motor1->getPositionTolerance()};

  while(1){
    // controler registers (only they can be read back)
    for (unsigned int motorID = 0; motorID <= 1;  ++motorID){
        boost::shared_ptr<mtca4u::MotorControler> motor = motorDriverCard->getMotorControler(motorID);

	// test that writing values in a row work
	// mask then in case the initial values are at the 24 bit data word boundary (readback error)
	motor->setTargetPosition( MASK24(initialTargetPosition[motorID] + 1000) );
	motor->setMinimumVelocity( MASK24(initialMinVelocity[motorID] + 1) );
	motor->setMaximumVelocity( MASK24(initialMaxVelocity[motorID] + 2) );
 	motor->setMaximumAcceleration( MASK24(initialMaxAcceleration[motorID] + 3) );
	motor->setPositionTolerance( MASK24(initialPositionTolerance[motorID] + 4) );
 
	PRINT_AND_COUNT_ERRORS( motor->getTargetPosition() == MASK24(initialTargetPosition[motorID] + 1000), motorID );
	PRINT_AND_COUNT_ERRORS( motor->getMinimumVelocity() == MASK24(initialMinVelocity[motorID] + 1), motorID );
	PRINT_AND_COUNT_ERRORS( motor->getMaximumVelocity() == MASK24(initialMaxVelocity[motorID] + 2), motorID );
	PRINT_AND_COUNT_ERRORS( motor->getMaximumAcceleration() == MASK24(initialMaxAcceleration[motorID] + 3), motorID );
	PRINT_AND_COUNT_ERRORS( motor->getPositionTolerance() == MASK24(initialPositionTolerance[motorID] + 4), motorID );

	// set back to the initial values (another test plus needed for the next test round because we have to 
	// alternate the register content.
	// This time we write in reverse order, just for the fun of it
	motor->setPositionTolerance(initialPositionTolerance[motorID]);
 	motor->setMaximumAcceleration(initialMaxAcceleration[motorID]);
	motor->setMaximumVelocity(initialMaxVelocity[motorID]);
	motor->setMinimumVelocity(initialMinVelocity[motorID]);
	motor->setTargetPosition(initialTargetPosition[motorID]);
 
	PRINT_AND_COUNT_ERRORS( motor->getTargetPosition() == initialTargetPosition[motorID], motorID );
	PRINT_AND_COUNT_ERRORS( motor->getMinimumVelocity() == initialMinVelocity[motorID], motorID );
	PRINT_AND_COUNT_ERRORS( motor->getMaximumVelocity() == initialMaxVelocity[motorID], motorID );
	PRINT_AND_COUNT_ERRORS( motor->getMaximumAcceleration() == initialMaxAcceleration[motorID], motorID );	
	PRINT_AND_COUNT_ERRORS( motor->getPositionTolerance() ==  initialPositionTolerance[motorID], motorID );
    }

    // patterns for both motors, including writing to controler and driver registers, where only controler
    // registers can be checked. Nevertheless there might be interference.
    motor0->setTargetPosition(0xAAAAAA);
    motor1->setTargetPosition(0x555555);
    motor0->setChopperControlData( mtca4u::ChopperControlData( 0x15555 ) );
    motor1->setPositionTolerance( 0xAAA );
    motor1->setChopperControlData( mtca4u::ChopperControlData( 0x0AAAA ) );
    motor0->setPositionTolerance( 0x555 );
    
    // although 0xAAAAAA has been written, the return value is 0xFFAAAAAA.
    // This is because the value is interpreted as signed 24 bit, and the return value is the
    // corresponding signed 32 bit integer.
    PRINT_AND_COUNT_ERRORS( motor0->getTargetPosition() == static_cast<int32_t>(0xFFAAAAAA), 0 );
    PRINT_AND_COUNT_ERRORS( motor1->getTargetPosition() == static_cast<int32_t>(0x555555), 1 );
    PRINT_AND_COUNT_ERRORS( motor0->getPositionTolerance() ==  0x555, 0 );
    PRINT_AND_COUNT_ERRORS( motor1->getPositionTolerance() ==  0xAAA, 1 );

    // shuffle the order when setting back to defaults
    motor0->setTargetPosition(initialTargetPosition[0]);
    motor0->setChopperControlData( mtca4u::ChopperControlData( 0x0 ) );
    motor1->setChopperControlData( mtca4u::ChopperControlData( 0x0 ) );
    motor1->setTargetPosition(initialTargetPosition[1]);
    motor1->setPositionTolerance( initialPositionTolerance[1] );
    motor0->setPositionTolerance( initialPositionTolerance[0] );
    
    PRINT_AND_COUNT_ERRORS( motor0->getTargetPosition() == initialTargetPosition[0], 0 );
    PRINT_AND_COUNT_ERRORS( motor1->getTargetPosition() == initialTargetPosition[1], 1 );
    PRINT_AND_COUNT_ERRORS( motor0->getPositionTolerance() == initialPositionTolerance[1], 1 );
    PRINT_AND_COUNT_ERRORS( motor1->getPositionTolerance() == initialPositionTolerance[0], 0 );

    // footer: count up the run count and print a status every second.
    runCounter++;

    time_t currentTime = time(NULL);
    if (currentTime > lastPrintTime){
      std::cout << "\r" << currentTime - startingTime << " second" <<  (currentTime - startingTime!=1?"s":"") 
		<< " elapsed, " << runCounter << " test cycles completed, " <<
	errorCounter << " errors detected so far." << std::flush;
      lastPrintTime = currentTime;
    }
  }
  
}
