#include <iostream>

#include "MotorDriverCardFactory.h"
#include "MotorDriverCardImpl.h"
#include <unistd.h>

using namespace mtca4u;

int main( int argc, char* argv[] )
{
  if (argc !=2 && argc!=3){
    std::cout <<  argv[0] << " just creates an instance of MotorDirverCardImpl, which initialises all registers."
	      << std::endl;
     std::cout <<  "You can use this programme to quickly reset all registers and turn off the drivers. When specifying a motor config it will be used, otherwise the default paramerets will be loaded."
	      << std::endl<< std::endl;
    std::cout << "Usage: " << argv[0] << " dmapFile motorConfig.xml" << std::endl;
    return -1;
  }

  std::string motorConfigFileName;
  if (argc == 3){
    motorConfigFileName=argv[2];
  }
  // else leave the string empty, which will load the devault config

  std::pair<std::string, std::string> deviceFileAndMapFileName =
    dmapFileParser().parse( argv[1] )->begin()->getDeviceFileAndMapFileName();

  (void) mtca4u::MotorDriverCardFactory::instance().createMotorDriverCard( 
		deviceFileAndMapFileName.first,
		deviceFileAndMapFileName.second,
		motorConfigFileName);
}
