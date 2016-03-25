#include "MotorDriverCardFactory.h"

#include "MotorDriverCardImpl.h"
#include "MotorDriverCardConfigXML.h"
#include "MotorDriverCardDummy.h"
#include <mtca4u/MapFileParser.h>
#include <mtca4u/Device.h>

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
MotorDriverCardFactory::createMotorDriverCard(std::string alias,
		std::string mapModuleName,
		std::string motorConfigFileName){
	boost::lock_guard<boost::mutex> guard(_factoryMutex);
	// if the card is not in the map it will be created, otherwise the
	// existing one is delivered
	//TSK - fix me - now key must be a pair of deviceFileName and mapModuleName, not only deviceFileName
	boost::shared_ptr<MotorDriverCard> & motorDriverCard =  _motorDriverCards[std::make_pair(alias, mapModuleName)];

	// create a new instance of the motor driver card if the shared pointer is
	// empty, which means it has just been added to the map
	if (!motorDriverCard){
		if (_dummyMode){ //if (deviceFileName=="custom://MotorDriverCardDummy")
		  // just create a MotorDriverCardDummy
		  motorDriverCard.reset(new MotorDriverCardDummy());
		}else{
		  boost::shared_ptr<Device> device(new Device);
		  device->open(alias);
		  MotorDriverCardConfig cardConfig = MotorDriverCardConfigXML::read(motorConfigFileName);

		  motorDriverCard.reset(new MotorDriverCardImpl(device, mapModuleName, cardConfig));
		}// else deviceFileName=="/dummy/MotorDriverCard"

	}// if (!motorDriverCard)

	return motorDriverCard;
}

bool MotorDriverCardFactory::getDummyMode()
{
	return _dummyMode;
}

void MotorDriverCardFactory::setDummyMode(bool dummyMode)
{
	_dummyMode = dummyMode;
}

void MotorDriverCardFactory::setDeviceaccessDMapFilePath(std::string dmapFileName){
  BackendFactory::getInstance().setDMapFilePath( dmapFileName );
}

std::string MotorDriverCardFactory::getDeviceaccessDMapFilePath(){
  return BackendFactory::getInstance().getDMapFilePath();
}

}// namespace mtca4u
