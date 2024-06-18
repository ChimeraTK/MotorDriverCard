#include "MotorDriverCardFactory.h"

#include "impl/MotorDriverCardImpl.h"
#include "MotorDriverCardConfigXML.h"
#include "MotorDriverCardDummy.h"

#include <ChimeraTK/Device.h>

#include <boost/thread/locks.hpp>

namespace mtca4u {

  MotorDriverCardFactory::MotorDriverCardFactory() : _factoryMutex(), _motorDriverCards(), _dummyMode(false) {
    /* empty */
  }

  // the copy constructor is intentionally not implemented

  MotorDriverCardFactory& MotorDriverCardFactory::instance() {
    static MotorDriverCardFactory motorDriverCardFactory;
    return motorDriverCardFactory;
  }

  boost::shared_ptr<MotorDriverCard> MotorDriverCardFactory::createMotorDriverCard(
      std::string alias, std::string mapModuleName, std::string motorConfigFileName) {
    boost::lock_guard<boost::mutex> guard(_factoryMutex);

    // Check if we have the card and if the weak reference is still valid. That way we can guarantee that there is only
    // one MotorDriverCard instance accessing the device (at least in this process) while also being able to re-open
    // the underlying device by giving up all motors using the card.
    auto id = std::make_pair(alias, mapModuleName);
    auto iterator = _motorDriverCards.find(id);
    if(iterator != _motorDriverCards.end()) {
      auto card = iterator->second.lock();
      if(card) return card;
    }

    boost::shared_ptr<MotorDriverCard> motorDriverCard;
    if(_dummyMode) {
      // just create a MotorDriverCardDummy
      motorDriverCard.reset(new MotorDriverCardDummy());
    }
    else {
      boost::shared_ptr<ChimeraTK::Device> device(new ChimeraTK::Device);
      device->open(alias);
      MotorDriverCardConfig cardConfig = MotorDriverCardConfigXML::read(motorConfigFileName);

      motorDriverCard.reset(new MotorDriverCardImpl(device, mapModuleName, cardConfig));
    }
    _motorDriverCards[id] = motorDriverCard;

    return motorDriverCard;
  }

  bool MotorDriverCardFactory::getDummyMode() {
    return _dummyMode;
  }

  void MotorDriverCardFactory::setDummyMode(bool dummyMode) {
    _dummyMode = dummyMode;
  }

  void MotorDriverCardFactory::setDeviceaccessDMapFilePath(std::string dmapFileName) {
    ChimeraTK::BackendFactory::getInstance().setDMapFilePath(dmapFileName);
  }

  std::string MotorDriverCardFactory::getDeviceaccessDMapFilePath() {
    return ChimeraTK::BackendFactory::getInstance().getDMapFilePath();
  }

} // namespace mtca4u
