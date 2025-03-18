// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "MultiVariableWord.h"
#include "TMC260Constants.h"

namespace ChimeraTK {

  class TMC260Word : public MultiVariableWord {
   public:
    virtual void setAddress(unsigned int) = 0;
    virtual unsigned int getAddress() const = 0;
    virtual void setPayloadData(unsigned int) = 0;
    virtual unsigned int getPayloadData() const = 0;
  };

  // FIXME implement all the types correctly.
  class DriverControlData : public TMC260Word {
   public:
    DriverControlData(unsigned int dataWord = 0) {
      setAddress(tmc260::ADDRESS_DRIVER_CONTROL);
      setPayloadData(dataWord);
    }
    ADD_VARIABLE(Address, 18, 19);
    ADD_VARIABLE(PayloadData, 0, 17);
    ADD_VARIABLE(Interpolation, 9, 9);
    ADD_VARIABLE(DoubleEdge, 8, 8);
    ADD_VARIABLE(MicroStepResolution, 0, 3);
  };

  class ChopperControlData : public TMC260Word {
   public:
    ChopperControlData(unsigned int dataWord = 0) {
      setAddress(tmc260::ADDRESS_CHOPPER_CONFIG);
      setPayloadData(dataWord);
    }
    ADD_VARIABLE(Address, 17, 19);
    ADD_VARIABLE(PayloadData, 0, 16);
    ADD_VARIABLE(BlankingTime, 15, 16);
    ADD_VARIABLE(ChopperMode, 14, 14);
    ADD_VARIABLE(RandomOffTime, 13, 13);
    ADD_VARIABLE(HysteresisDecrementInterval, 11, 12);
    ADD_VARIABLE(HysteresisEndValue, 7, 10);
    ADD_VARIABLE(HysteresisStartValue, 4, 6);
    ADD_VARIABLE(OffTime, 0, 3);
  };

  class CoolStepControlData : public TMC260Word {
   public:
    CoolStepControlData(unsigned int dataWord = 0) {
      setAddress(tmc260::ADDRESS_COOL_STEP_CONFIG);
      setPayloadData(dataWord);
    }
    ADD_VARIABLE(Address, 17, 19);
    ADD_VARIABLE(PayloadData, 0, 16);
  };

  class StallGuardControlData : public TMC260Word {
   public:
    StallGuardControlData(unsigned int dataWord = 0) {
      setAddress(tmc260::ADDRESS_STALL_GUARD_CONFIG);
      setPayloadData(dataWord);
    }
    ADD_VARIABLE(Address, 17, 19);
    ADD_VARIABLE(PayloadData, 0, 16);
    ADD_VARIABLE(FilterEnable, 16, 16);
    ADD_VARIABLE(StallGuardThreshold, 8, 14);
    ADD_VARIABLE(CurrentScale, 0, 4);
  };

  class DriverConfigData : public TMC260Word {
   public:
    DriverConfigData(unsigned int dataWord = 0) {
      setAddress(tmc260::ADDRESS_DRIVER_CONFIG);
      setPayloadData(dataWord);
    }
    ADD_VARIABLE(Address, 17, 19);
    ADD_VARIABLE(PayloadData, 0, 16);
  };

  /** This class represents the driver status of the TMC260 Read Response (data
   * sheet section 6.10).
   *
   *  @todo FIXME I think they are the general status bits 0:7 only. At least
   * that's what's implemented here.
   */
  class DriverStatusData : public MultiVariableWord {
   public:
    DriverStatusData(unsigned int dataWord = 0) : MultiVariableWord(dataWord) {}
    ADD_VARIABLE(StallGuardStatus, 0, 0);
    ADD_VARIABLE(OvertemperatureShutdown, 1, 1);
    ADD_VARIABLE(OvertemperatureWarning, 2, 2);
    ADD_VARIABLE(ShortToGroundCoilA, 3, 3);
    ADD_VARIABLE(ShortToGroundCoilB, 4, 4);
    ADD_VARIABLE(ShortToGroundIndicators, 3, 4);
    ADD_VARIABLE(OpenLoadCoilA, 5, 5);
    ADD_VARIABLE(OpenLoadCoilB, 6, 6);
    ADD_VARIABLE(OpenLoadIndicators, 5, 6);

    // Standstill indicator bit, set if no STEP input was seen for 2^20 clock cycles of 16Mhz (~65ms)
    ADD_VARIABLE(StandstillIndicator, 7, 7);
  };

} // namespace ChimeraTK
