#include "MotorDriverCardDummy.h"

#include "DFMC_MD22Constants.h"
#include "MotorControlerDummy.h"

#include <ChimeraTK/Exception.h>

using namespace mtca4u::dfmc_md22;

namespace mtca4u {

  MotorDriverCardDummy::MotorDriverCardDummy() : _motorControlers(N_MOTORS_MAX) {
    for(unsigned int id = 0; id < _motorControlers.size(); ++id) {
      _motorControlers[id].reset(new MotorControlerDummy(id));
    }
  }

  MotorDriverCardDummy::~MotorDriverCardDummy() {}

  boost::shared_ptr<MotorControler> MotorDriverCardDummy::getMotorControler(unsigned int motorControlerID) {
    try {
      return _motorControlers.at(motorControlerID);
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

} // namespace mtca4u
