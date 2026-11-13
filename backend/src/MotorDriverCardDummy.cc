#include "MotorDriverCardDummy.h"

#include "DFMC_MD22Constants.h"
#include "MotorControlerDummy.h"

#include <ChimeraTK/Exception.h>

#include <boost/make_shared.hpp>

#include <iostream>

using namespace mtca4u::dfmc_md22;

namespace mtca4u {

  MotorDriverCardDummy::MotorDriverCardDummy() {
    _motorControllers.reserve(N_MOTORS_MAX);
    for(unsigned int id = 0; id < N_MOTORS_MAX; ++id) {
      _motorControllers.emplace_back(boost::make_shared<MotorControlerDummy>(id));
    }
  }

  boost::shared_ptr<MotorControler> MotorDriverCardDummy::getMotorControler(unsigned int motorControlerID) {
    try {
      return _motorControllers.at(motorControlerID);
    }
    catch(std::out_of_range& e) {
      std::stringstream errorMessage;
      errorMessage << "motorControlerID " << motorControlerID << " is too large. " << e.what();
      throw ChimeraTK::logic_error(errorMessage.str());
    }
  }

  PowerMonitor& MotorDriverCardDummy::getPowerMonitor() {
    throw ChimeraTK::logic_error("getPowerMonitor() is not implemented inMotorDriverCardDummy");
  }

  bool MotorDriverCardDummy::isFunctional() const {
    return true;
  }

  void MotorDriverCardDummy::reconfigure(bool /* enableMotor */) {
    // The dummy keeps no device state that could go stale, and the motor
    // current has no meaning for it.
  }

} // namespace mtca4u
