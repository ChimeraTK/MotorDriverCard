#include <iostream>

#include "MotorDriverCardImpl.h"
#include "MotorDriverCardConfigXML.h"
#include <unistd.h>
#include <signal.h>

bool volatile keepRunning = true;

void intHandler(int /*dummy*/ = 0) {
    keepRunning = false;
}

#define CHECK_EXIT_ON_SIGNAL\
  if(!keepRunning){\
    motor0.setEnabled(false);\
    std::cout << std::endl << "Caught signal, ending programme." << std::endl; \
    return 0;\
  }

using namespace mtca4u;

int main( int argc, char* argv[] )
{
  if (argc !=4){
    std::cout << "Move to the 'left' (low values) end switch. Afterward move infinetly between 0.2*nStepsMax and nStepsMax."
	      << std::endl << std::endl;
    std::cout << "Usage: " << argv[0] << " dmapFile MotorDriverConfig.xml nStepsMax" << std::endl;
    return -1;
  }

  signal(SIGINT, intHandler);

  std::pair<std::string, std::string> deviceFileAndMapFileName =
    dmapFileParser().parse( argv[1] )->begin()->getDeviceFileAndMapFileName();

  boost::shared_ptr< devPCIE > ioDevice( new devPCIE );
  ioDevice->openDev( deviceFileAndMapFileName.first );

  boost::shared_ptr< mapFile > registerMapping =  mapFileParser().parse(deviceFileAndMapFileName.second);

  boost::shared_ptr< devMap< devBase > > mappedDevice( new devMap<devBase> );
  mappedDevice->openDev( ioDevice, registerMapping );

  mtca4u::MotorDriverCardConfig motorDriverCardConfig = MotorDriverCardConfigXML::read( argv[2] );

  mtca4u::MotorDriverCardImpl motorDriverCard( mappedDevice , motorDriverCardConfig );

  // until now it is a copy of initialiseDFMC-MC22, now we start moving the motor

  unsigned int nStepsMax = (strtoul( argv[3] , NULL, 0 ) & 0x7FFFFF);// limit to the maximal possible position

  mtca4u::MotorControler & motor0 = motorDriverCard.getMotorControler(0);

  try{
    motor0.setEnabled(true);
    
    std::cout << "moving to left end switch" << std::endl;

    // move as far left as possible
    motor0.setActualPosition(0x7FFFFF); // the maximal possible position
    motor0.setTargetPosition(0x100000); // the minimal possible position

    while(  motorDriverCard.getReferenceSwitchData().getLeft1() != 0x1 ){
      std::cout << "\r Position: " << motor0.getActualPosition()<< std::hex << " ,  switches are 0x" 
		<< motorDriverCard.getReferenceSwitchData().getDATA()
		<< "        " << std::dec <<std::flush;
      if (motorDriverCard.getReferenceSwitchData().getRight1() ){
	std::cout << std::endl << "WARNING: Right end switch went active when the left switch was expected!"
		  << " Check you end switch cabeling!" 
		  << std::endl;
	break;
      }
      
      CHECK_EXIT_ON_SIGNAL

      // FIXME: Check on motor stall / motor is still moving.
    }
    
    std::cout << std::endl;

    //    std::cout << std::hex << "switches are 0x" << motorDriverCard.getReferenceSwitchData().getDataWord() 
    //	      << std::dec << std::endl;

    //    std::cout << "Position is " << motor0.getActualPosition() << std::endl;
    std::cout << "Setting left switch to position 0 " <<std::endl;

    motor0.setEnabled(false);
    motor0.setActualPosition(0);
    motor0.setTargetPosition(0);
    motor0.setEnabled(true);

    std::cout <<"Starting the forward-backward loop ..." << std::endl;

    while (1){
      motor0.setTargetPosition( nStepsMax );
      std::cout << std::endl<< "Moving forward..." << std::endl;

      try{
	while (motor0.getActualPosition() != motor0.getTargetPosition() ){
	  std::cout << "\r Position: " << motor0.getActualPosition()  << "    "
		    << " ,    switches are : 0x" << std::hex 
		    << motorDriverCard.getReferenceSwitchData().getDataWord() << std::dec
		    << "    " << std::flush;
	  if( motorDriverCard.getReferenceSwitchData().getRight1() ){
	    nStepsMax = static_cast<unsigned int>( 0.8*motor0.getActualPosition() );
	  std::cout << std::endl <<"Right end switch reached. Adapting nStepsMax to " << nStepsMax << std::endl;
	    break;
	  }
	  CHECK_EXIT_ON_SIGNAL	  
	}
      }catch(mtca4u::MotorDriverException &e){
	std::cout << "Problem reading target position" << std::endl;
      }
      std::cout << std::endl;

      std::cout << "Maximum position reached: " << motor0.getActualPosition() << std::endl;
      std::cout << std::endl;
      std::cout << "Moving back..." << std::endl;

      motor0.setTargetPosition( static_cast<unsigned int>( 0.2 * nStepsMax) );
      try{
	while (motor0.getActualPosition() != motor0.getTargetPosition() ){
	  std::cout << "\r Position: " << motor0.getActualPosition() 
		    << " ,    switches are : 0x" << std::hex 
		    << motorDriverCard.getReferenceSwitchData().getDataWord() << std::dec
		    << "    " << std::flush;
	  if( motorDriverCard.getReferenceSwitchData().getLeft1() ){
	    std::cout << "Left end switch reached. Adapting calibration." << std::endl;
	    motor0.setActualPosition(0);
	    break;
	  }
	  CHECK_EXIT_ON_SIGNAL
	}
      }catch(mtca4u::MotorDriverException &e){
	std::cout << "Problem reading target position" << std::endl;
      }
      std::cout << std::endl;
  
      std::cout << "Minimal position reached: " << motor0.getActualPosition() << std::endl;
  }
  }
  catch(...){
    motor0.setEnabled(false);
  }
  //  motor0.setTargetPosition(0);

  
}
