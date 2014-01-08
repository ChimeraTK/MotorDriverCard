#include <iostream>

#include "MotorDriverCardImpl.h"
#include <unistd.h>

int main( int argc, char* argv[] )
{
  if (argc !=2){
    std::cout <<  argv[0] << " just creates an instance of MotorDirverCardImpl, which initialises all registers"
	      << std::endl;
    std::cout << "usage: " << argv[0] << " dmapFile" << std::endl;
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
  //  mtca4u::MotorControler & motor1 = motorDriverCard.getMotorControler(1);
  
  //  sleep(1);
  motor0.setStallGuardControlData( mtca4u::StallGuardControlData(0x5) );
  //  sleep(1);
  motor0.setDriverControlData( mtca4u::DriverControlData(0x4) );

  try{
    motor0.setEnabled(true);

    std::cout << std::hex << "switches are 0x" << motorDriverCard.getReferenceSwitchData().getDataWord() 
	      << std::dec << std::endl;

    mtca4u::ReferenceConfigAndRampModeData referenceConfigAndRampModeData;
    //referenceConfigAndRampModeData.setDISABLE_STOP_L(true);
    //referenceConfigAndRampModeData.setDISABLE_STOP_R(true);
    motor0.setReferenceConfigAndRampModeData( referenceConfigAndRampModeData );

    std::cout << motor0.getActualPosition() << std::endl;
    motor0.setTargetPosition(20000);
    //motor0.setActualPosition(20000);
    try{
      while (motor0.getActualPosition() != motor0.getTargetPosition() ){
	std::cout << "\r" << motor0.getActualPosition()  << "    "
		  << "    switches are : 0x" << std::hex 
		  << motorDriverCard.getReferenceSwitchData().getDataWord() << std::dec
		  << "    " << std::flush;
      }
    }catch(mtca4u::MotorDriverException &e){
      std::cout << "Problem reading target position" << std::endl;
    }
    std::cout << std::endl;

    sleep(1);
    std::cout << motor0.getActualPosition() << std::endl;
    motor0.setTargetPosition(0);
    std::cout << motor0.getActualPosition() << std::endl;
    //  sleep(1);
    std::cout << motor0.getActualPosition() << std::endl;
    try{
      while (motor0.getActualPosition() != motor0.getTargetPosition() ){
	std::cout << "\r" << motor0.getActualPosition() 
		  << "    switches are : 0x" << std::hex 
		  << motorDriverCard.getReferenceSwitchData().getDataWord() << std::dec
		  << "    " << std::flush;
      }
    }catch(mtca4u::MotorDriverException &e){
      std::cout << "Problem reading target position" << std::endl;
    }
    std::cout << std::endl;
  
    motor0.setEnabled(false);
  }
  catch(...){
    motor0.setEnabled(false);
  }
  //  motor0.setTargetPosition(0);

  
}
