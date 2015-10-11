#include "MotorDriverCardFactory.h"

//#include <mtca4u/BaseDevice.h>
//#include <mtca4u/PcieDevice.h>
//#include <mtca4u/Device.h>
#include "DFMC_MD22Dummy.h"
#include "MotorDriverCardImpl.h"
#include "MotorDriverCardConfigXML.h"
#include "MotorDriverCardDummy.h"
#include <mtca4u/MapFileParser.h>

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
			std::cout<<"----if----"<<std::endl;
		}else{
			//mtca4u::DeviceFactory FactoryInstance = mtca4u::DeviceFactory::getInstance();
			//boost::shared_ptr< Device > device = FactoryInstance.createDevice(alias);
			boost::shared_ptr<Device> device ( new Device());
			std::cout<<"----else b4 open----"<<alias<<std::endl;

			//device->open(alias);
			std::string mapFileName = "DFMC_MD22_test.mapp"; // fixme just temp.
			boost::shared_ptr<DFMC_MD22Dummy> dummyDevice;
			dummyDevice.reset( new DFMC_MD22Dummy(mapFileName, mapModuleName) );
			MapFileParser fileParser;
			boost::shared_ptr<RegisterInfoMap> registerMapping = fileParser.parse(mapFileName);
			device->open(dummyDevice, registerMapping);

			std::cout<<"----else after open----"<<std::endl;

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

}// namespace mtca4u
