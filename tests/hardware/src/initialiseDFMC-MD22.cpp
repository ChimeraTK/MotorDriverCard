#include <iostream>

#include "MotorDriverCardImpl.h"
#include <unistd.h>

using namespace mtca4u;

int main( int argc, char* argv[] )
{
  if (argc !=2){
    std::cout <<  argv[0] << " just creates an instance of MotorDirverCardImpl, which initialises all registers."
	      << std::endl;
     std::cout <<  "You can use this programme to quickly reset all registers and turn off the drivers."
	      << std::endl<< std::endl;
    std::cout << "Usage: " << argv[0] << " dmapFile" << std::endl;
    return -1;
  }

  // In principle we just need a MappedDevice and a MotorDriverCardConfig to initialise the MotorDriverCard.
  // Unfortunately the MappedDevice as a devMap< devBase > is a bit clumsy to build at the moment because
  // we have to open the dmap file manually, and then open the device and the map ourselves to pass it to the
  // open command (Side effect of devMap being a template and not properly using only devBase as interface.
  // To be improved...)

  std::pair<std::string, std::string> deviceFileAndMapFileName =
    dmapFileParser().parse( argv[1] )->begin()->getDeviceFileAndMapFileName();

  boost::shared_ptr< devPCIE > ioDevice( new devPCIE );
  ioDevice->openDev( deviceFileAndMapFileName.first );

  boost::shared_ptr< mapFile > registerMapping =  mapFileParser().parse(deviceFileAndMapFileName.second);

  boost::shared_ptr< devMap< devBase > > mappedDevice( new devMap<devBase> );
  mappedDevice->openDev( ioDevice, registerMapping );

  mtca4u::MotorDriverCardImpl motorDriverCard( mappedDevice , mtca4u::MotorDriverCardConfig());
}
