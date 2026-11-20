// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "MotorDriverCardFactory.h"

#include "impl/MotorDriverCardImpl.h"
#include "MotorDriverCardConfigXML.h"
#include "MotorDriverCardDummy.h"

#include <ChimeraTK/Device.h>

#include <utility>

namespace ChimeraTK::MotorDriver {

  // the copy constructor is intentionally not implemented

  MotorDriverCardFactory& MotorDriverCardFactory::instance() {
    static MotorDriverCardFactory motorDriverCardFactory;
    return motorDriverCardFactory;
  }

  boost::shared_ptr<MotorDriverCard> MotorDriverCardFactory::createMotorDriverCard(
      const std::string& alias, const std::string& mapModuleName, const std::string& motorConfigFileName) {
    boost::lock_guard<boost::mutex> guard(_factoryMutex);

    auto id = std::make_pair(alias, mapModuleName);
    if(_dummyMode) {
      auto card = _dummyMotorDriverCards[id];
      if(!card) {
        // The constructor is private, so boost::make_shared cannot be used here.
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        _dummyMotorDriverCards[id].reset(new MotorDriverCardDummy());
      }

      return _dummyMotorDriverCards[id];
    }

    // Check if we have the card and if the weak reference is still valid. That way we can guarantee that there is only
    // one MotorDriverCard instance accessing the device (at least in this process) while also being able to re-open
    // the underlying device by giving up all motors using the card.
    auto iterator = _motorDriverCards.find(id);
    if(iterator != _motorDriverCards.end()) {
      auto card = iterator->second.lock();
      if(card) {
        return card;
      }
    }

    boost::shared_ptr<MotorDriverCard> motorDriverCard;
    {
      boost::shared_ptr<ChimeraTK::Device> device(new ChimeraTK::Device);
      device->open(alias);
      MotorDriverCardConfig cardConfig = MotorDriverCardConfigXML::read(motorConfigFileName);

      // The constructor is private, so boost::make_shared cannot be used here.
      // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
      motorDriverCard.reset(new MotorDriverCardImpl(device, mapModuleName, cardConfig));
    }
    _motorDriverCards[id] = motorDriverCard;

    return motorDriverCard;
  }

  bool MotorDriverCardFactory::getDummyMode() const {
    return _dummyMode;
  }

  void MotorDriverCardFactory::setDummyMode(bool dummyMode) {
    _dummyMode = dummyMode;
  }

  void MotorDriverCardFactory::setDeviceAccessDMapFilePath(std::string dmapFileName) {
    ChimeraTK::BackendFactory::getInstance().setDMapFilePath(std::move(dmapFileName));
  }

  std::string MotorDriverCardFactory::getDeviceAccessDMapFilePath() {
    return ChimeraTK::BackendFactory::getInstance().getDMapFilePath();
  }

} // namespace ChimeraTK::MotorDriver
