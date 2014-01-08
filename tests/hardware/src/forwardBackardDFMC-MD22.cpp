#include <iostream>

#include "MotorDriverCardImpl.h"
#include <unistd.h>

int main( int argc, char* argv[] )
{
  if (argc !=2){
    std::cout << "Move to motor forward, wait a second and move back to the original position."
	      << std::endl << std::endl;
    std::cout << "Usage: " << argv[0] << " dmapFile" << std::endl;
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

  // until now it is a copy of initialiseDFMC-MC22, now we start moving the motor

  mtca4u::MotorControler & motor0 = motorDriverCard.getMotorControler(0);

  // Adapt the settings to the VT-21 motor.
  // FIXME: read in the config from a file

  // filtered stallGuard with 3/32 current scale
  motor0.setStallGuardControlData( mtca4u::StallGuardControlData(0x10002) );
  // coolStep with 1/4 CS minimum current (taken directly from the data sheet, no clue if it works)
  motor0.setCoolStepControlData( mtca4u::CoolStepControlData(0x8202) );
  // 16 microsteps per 90 degrees
  motor0.setDriverControlData( mtca4u::DriverControlData(0x4) );

  try{
    motor0.setEnabled(true);

    std::cout << std::hex << "switches are 0x" << motorDriverCard.getReferenceSwitchData().getDataWord() 
	      << std::dec << std::endl;

    mtca4u::ReferenceConfigAndRampModeData referenceConfigAndRampModeData;
    //referenceConfigAndRampModeData.setDISABLE_STOP_L(true);
    //referenceConfigAndRampModeData.setDISABLE_STOP_R(true);
    motor0.setReferenceConfigAndRampModeData( referenceConfigAndRampModeData );

    std::cout << "Position is " << motor0.getActualPosition() << std::endl;
    std::cout << "Starting the motor..." << std::endl;

    motor0.setTargetPosition(50000);

    try{
      while (motor0.getActualPosition() != motor0.getTargetPosition() ){
	std::cout << "\r Position: " << motor0.getActualPosition()  << "    "
		  << " ,    switches are : 0x" << std::hex 
		  << motorDriverCard.getReferenceSwitchData().getDataWord() << std::dec
		  << "    " << std::flush;
      }
    }catch(mtca4u::MotorDriverException &e){
      std::cout << "Problem reading target position" << std::endl;
    }
    std::cout << std::endl;

    sleep(1);

    std::cout << "Maximum position reached: " << motor0.getActualPosition() << std::endl;
    std::cout << std::endl;
    std::cout << "Moving back..." << std::endl;

    motor0.setTargetPosition(0);
     try{
       while (motor0.getActualPosition() != motor0.getTargetPosition() ){
	 std::cout << "\r Position: " << motor0.getActualPosition() 
		   << " ,    switches are : 0x" << std::hex 
		   << motorDriverCard.getReferenceSwitchData().getDataWord() << std::dec
		   << "    " << std::flush;
       }
     }catch(mtca4u::MotorDriverException &e){
       std::cout << "Problem reading target position" << std::endl;
     }
     std::cout << std::endl;
  
     std::cout << "Original position reached: " << motor0.getActualPosition() << std::endl;

     std::cout << "Turning off the motor." << std::endl;
     motor0.setEnabled(false);
  }
  catch(...){
    motor0.setEnabled(false);
  }
  //  motor0.setTargetPosition(0);

  
}
