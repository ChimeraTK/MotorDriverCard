// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "MotorDriverCard.h"

#include <vector>

namespace ChimeraTK {
  /**
   * A dummy implementation of the MotorDriverCard. It
   * provides MotorControlerDummys and a PowerMonitorDummy.
   */
  class MotorDriverCardDummy : public MotorDriverCard {
   public:
    boost::shared_ptr<MotorControler> getMotorControler(unsigned int motorControlerID) override;

    PowerMonitor& getPowerMonitor() override;
    ~MotorDriverCardDummy() override = default;

   private:
    // The constructor does not need to know anything. There is noting to
    // configure in the dummy, and it does not access any hardware (or hardware
    // dummy).
    MotorDriverCardDummy();

    // The factory needs access to the constructor because it is the only official
    // ways to create a MotorDriverCard
    friend class MotorDriverCardFactory;

    // For testing we need access to the constructor
    friend class MotorDriverCardDummyTest;

    std::vector<boost::shared_ptr<MotorControler>> _motorControllers;
  };

} // namespace ChimeraTK
