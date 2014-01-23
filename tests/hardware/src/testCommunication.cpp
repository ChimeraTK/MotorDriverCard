#include <iostream>

#include "MotorDriverCardImpl.h"
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

int main( int argc, char* argv[] )
{
  if (argc !=2){
    std::cout <<  argv[0] << "This is a long term test which performs SPI communication to the controler"
	      << " and driver chips by writing to the corresponding PCIe regsiters.\n"
	      << "It performs evaluation by reading back the written values.\n\n"
	      << "The test runs forever. Terminate it with <Ctrl><c>\n\n"
	      << "Usage: " << argv[0] << " dmapFile" << std::endl;
    return -1;
  }

  std::pair<std::string, std::string> deviceFileAndMapFileName =
    dmapFileParser().parse( argv[1] )->begin()->getDeviceFileAndMapFileName();

  boost::shared_ptr< devPCIE > ioDevice( new devPCIE );
  ioDevice->openDev( deviceFileAndMapFileName.first );

  boost::shared_ptr< mapFile > registerMapping =  mapFileParser().parse(deviceFileAndMapFileName.second);

  boost::shared_ptr< devMap< devBase > > mappedDevice( new devMap<devBase> );
  mappedDevice->openDev( ioDevice, registerMapping );

  mtca4u::MotorDriverCardImpl motorDriverCard( mappedDevice , mtca4u::MotorDriverCardConfig());

  mtca4u::MotorControler & motor0 = motorDriverCard.getMotorControler(0);
  mtca4u::MotorControler & motor1 = motorDriverCard.getMotorControler(1);
  
  uint64_t runCounter = 0;
  uint64_t errorCounter = 0;
  time_t startingTime = time(NULL);
  time_t lastPrintTime = startingTime;

  unsigned int initialTargetPosition[2] = {motor0.getTargetPosition(), motor1.getTargetPosition()};
  unsigned int initialMinVelocity[2] = {motor0.getMinimumVelocity(), motor1.getMinimumVelocity()};
  unsigned int initialMaxVelocity[2] = {motor0.getMaximumVelocity(), motor1.getMaximumVelocity()};
  unsigned int initialMaxAcceleration[2] = {motor0.getMaximumAcceleration(), motor1.getMaximumAcceleration()};
  unsigned int initialPositionTolerance[2] = {motor0.getPositionTolerance(), motor1.getPositionTolerance()};

  while(1){
    // controler registers (only they can be read back)
    for (unsigned int motorID = 0; motorID <= 1;  ++motorID){
        mtca4u::MotorControler & motor = motorDriverCard.getMotorControler(motorID);

	// test that writing values in a row work
	motor.setTargetPosition(initialTargetPosition[motorID] + 1000);
	motor.setMinimumVelocity(initialMinVelocity[motorID] + 1);
	motor.setMaximumVelocity(initialMaxVelocity[motorID] + 2);
 	motor.setMaximumAcceleration(initialMaxAcceleration[motorID] + 3);
	motor.setPositionTolerance(initialPositionTolerance[motorID] + 4);
 
	PRINT_AND_COUNT_ERRORS( motor.getTargetPosition() == initialTargetPosition[motorID] + 1000, motorID );
	PRINT_AND_COUNT_ERRORS( motor.getMinimumVelocity() == initialMinVelocity[motorID] + 1, motorID );
	PRINT_AND_COUNT_ERRORS( motor.getMaximumVelocity() == initialMaxVelocity[motorID] + 2, motorID );
	PRINT_AND_COUNT_ERRORS( motor.getMaximumAcceleration() == initialMaxAcceleration[motorID] + 3, motorID );
	PRINT_AND_COUNT_ERRORS( motor.getPositionTolerance() ==  initialPositionTolerance[motorID] + 4, motorID );

	// set back to the initial values (another test plus needed for the next test round because we have to 
	// alternate the register content.
	// This time we write in reverse order, just for the fun of it
	motor.setPositionTolerance(initialPositionTolerance[motorID]);
 	motor.setMaximumAcceleration(initialMaxAcceleration[motorID]);
	motor.setMaximumVelocity(initialMaxVelocity[motorID]);
	motor.setMinimumVelocity(initialMinVelocity[motorID]);
	motor.setTargetPosition(initialTargetPosition[motorID]);
 
	PRINT_AND_COUNT_ERRORS( motor.getTargetPosition() == initialTargetPosition[motorID], motorID );
	PRINT_AND_COUNT_ERRORS( motor.getMinimumVelocity() == initialMinVelocity[motorID], motorID );
	PRINT_AND_COUNT_ERRORS( motor.getMaximumVelocity() == initialMaxVelocity[motorID], motorID );
	PRINT_AND_COUNT_ERRORS( motor.getMaximumAcceleration() == initialMaxAcceleration[motorID], motorID );	
	PRINT_AND_COUNT_ERRORS( motor.getPositionTolerance() ==  initialPositionTolerance[motorID], motorID );
    }

    // patterns for both motors, including writing to controler and driver registers, where only controler
    // registers can be checked. Nevertheless there might be interference.
    motor0.setTargetPosition(0xAAAAAA);
    motor1.setTargetPosition(0x555555);
    motor0.setChopperControlData( mtca4u::ChopperControlData( 0x15555 ) );
    motor1.setPositionTolerance( 0xAAA );
    motor1.setChopperControlData( mtca4u::ChopperControlData( 0x0AAAA ) );
    motor0.setPositionTolerance( 0x555 );
    
    PRINT_AND_COUNT_ERRORS( motor0.getTargetPosition() == 0xAAAAAA, 0 );
    PRINT_AND_COUNT_ERRORS( motor1.getTargetPosition() == 0x555555, 1 );
    PRINT_AND_COUNT_ERRORS( motor0.getPositionTolerance() ==  0x555, 0 );
    PRINT_AND_COUNT_ERRORS( motor1.getPositionTolerance() ==  0xAAA, 1 );

    // shuffle the order when setting back to defaults
    motor0.setTargetPosition(initialTargetPosition[0]);
    motor0.setChopperControlData( mtca4u::ChopperControlData( 0x0 ) );
    motor1.setChopperControlData( mtca4u::ChopperControlData( 0x0 ) );
    motor1.setTargetPosition(initialTargetPosition[1]);
    motor1.setPositionTolerance( initialPositionTolerance[1] );
    motor0.setPositionTolerance( initialPositionTolerance[0] );
    
    PRINT_AND_COUNT_ERRORS( motor0.getTargetPosition() == initialTargetPosition[0], 0 );
    PRINT_AND_COUNT_ERRORS( motor1.getTargetPosition() == initialTargetPosition[1], 1 );
    PRINT_AND_COUNT_ERRORS( motor0.getPositionTolerance() == initialPositionTolerance[1], 1 );
    PRINT_AND_COUNT_ERRORS( motor1.getPositionTolerance() == initialPositionTolerance[0], 0 );

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
