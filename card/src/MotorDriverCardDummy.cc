// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "MotorDriverCardDummy.h"

#include "DFMC_MD22Constants.h"
#include "MotorControllerDummy.h"

#include <ChimeraTK/Exception.h>

#include <iostream>
#include <memory>
#include <sstream>

using namespace ChimeraTK::MotorDriver::dfmc_md22;

namespace ChimeraTK::MotorDriver {

  MotorDriverCardDummy::MotorDriverCardDummy() {
    _motorControllers.reserve(N_MOTORS_MAX);
    for(unsigned int id = 0; id < N_MOTORS_MAX; ++id) {
      _motorControllers.emplace_back(std::make_shared<MotorControllerDummy>(id));
    }
  }

  std::shared_ptr<MotorController> MotorDriverCardDummy::getMotorController(unsigned int motorControllerID) {
    try {
      return _motorControllers.at(motorControllerID);
    }
    catch(std::out_of_range& e) {
      std::stringstream errorMessage;
      errorMessage << "motorControllerID " << motorControllerID << " is too large. " << e.what();
      throw ChimeraTK::logic_error(errorMessage.str());
    }
  }

  PowerMonitor& MotorDriverCardDummy::getPowerMonitor() {
    throw ChimeraTK::logic_error("getPowerMonitor() is not implemented inMotorDriverCardDummy");
  }

} // namespace ChimeraTK::MotorDriver
