// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "TMC429Words.h"

#include <boost/shared_ptr.hpp>

namespace ChimeraTK {
  class MotorControler;
  class PowerMonitor;

  /**
   * A class to access the DFMC-MD22 motor driver card, which provides two
   * MotorControlers.
   */
  class MotorDriverCard {
   public:
    /** Get acces to one of the two motor controlers on this board.
     *  Valid IDs are 0 and 1.
     *  Throws a MotorDriverException if the ID is invalid.
     */
    virtual boost::shared_ptr<MotorControler> getMotorControler(unsigned int motorControlerID) = 0;

    /// Get a reference to the power monitor.
    virtual PowerMonitor& getPowerMonitor() = 0;

    virtual ~MotorDriverCard() {}
  };

} // namespace ChimeraTK
