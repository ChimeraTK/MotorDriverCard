#include "MotorDriverCardFactory.h"

#include <MtcaMappedDevice/devBase.h>
#include <MtcaMappedDevice/devPCIE.h>
#include <MtcaMappedDevice/devMap.h>
#include "DFMC_MD22Dummy.h"
#include "MotorDriverCardImpl.h"
#include "MotorDriverCardConfigXML.h"
#include "MotorDriverCardDummy.h"

#include <boost/thread/locks.hpp>

namespace mtca4u{

  MotorDriverCardFactory::MotorDriverCardFactory(){
    /* empty */
  }

  // the copy constructor is intentionally not implemented

  MotorDriverCardFactory & MotorDriverCardFactory::instance(){
    static MotorDriverCardFactory motorDriverCardFactory;
    return motorDriverCardFactory;
  }

  boost::shared_ptr<MotorDriverCard> 
    MotorDriverCardFactory::createMotorDriverCard(std::string deviceFileName,
						  std::string mapFileName,
						  std::string motorConfigFileName){
    boost::lock_guard<boost::mutex> guard(_factoryMutex);
    
    // if the card is not in the map it will be created, otherwise the
    // existing one is delivered
    boost::shared_ptr<MotorDriverCard> & motorDriverCard = 
      _motorDriverCards[deviceFileName];

    // create a new instance of the motor driver card if the shared pointer is
    // empty, which means it has just been added to the map
    if (!motorDriverCard){
      if (deviceFileName=="/dummy/MotorDriverCard"){
	// just create a MotorDriverCardDummy
	motorDriverCard.reset(new MotorDriverCardDummy());
      }else{
	// create a real MotorDriverCardImpl, with either pcie or dummy io device
	boost::shared_ptr< devBase > ioDevice;

	if (deviceFileName == mapFileName){
	  ioDevice.reset(new DFMC_MD22Dummy());
	}else{
	  ioDevice.reset(new devPCIE);
	}

	ioDevice->openDev(deviceFileName);

	boost::shared_ptr< mapFile > registerMapping = mapFileParser().parse(mapFileName);

	boost::shared_ptr< devMap< devBase > > mappedDevice(new devMap<devBase>);
	mappedDevice->openDev(ioDevice, registerMapping);

	MotorDriverCardConfig cardConfig = 
	  MotorDriverCardConfigXML::read(motorConfigFileName);

	motorDriverCard.reset(new MotorDriverCardImpl(mappedDevice, cardConfig));
      }// else deviceFileName=="/dummy/MotorDriverCard" 

    }// if (!motorDriverCard)

    return motorDriverCard;
  }

}// namespace mtca4u
