// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "MotorControler.h"
#include "TMC429Words.h"

#define MCX_DECLARE_SET_GET_VALUE(NAME, VARIABLE_IN_UNITS)                                                             \
  virtual void set##NAME(unsigned int VARIABLE_IN_UNITS) = 0;                                                          \
  virtual unsigned int get##NAME() = 0

#define MCX_DECLARE_SIGNED_SET_GET_VALUE(NAME, VARIABLE_IN_UNITS)                                                      \
  virtual void set##NAME(int VARIABLE_IN_UNITS) = 0;                                                                   \
  virtual int get##NAME() = 0

#define MCX_DECLARE_SET_GET_TYPED_REGISTER(NAME, VARIABLE_NAME)                                                        \
  virtual void set##NAME(NAME const& VARIABLE_NAME) = 0;                                                               \
  virtual NAME get##NAME() = 0

namespace ChimeraTK {
  /**
   * A class to control stepper motors using the DFMC-MD22 card.
   * This class describes the full (expert) functionality of one motor (access
   * to all registers of the driver and controller chip registers).
   *
   * This purely virtual class defines the interface.
   *
   * @author Martin Killenberg <martin.killenberg@desy.de>
   */
  class MotorControlerExpert : public MotorControler {
   public:
    virtual unsigned int getStallGuardValue() = 0; ///< Get the stall guard value  (which units? what
                                                   ///< does it mean? Expert?)
    virtual unsigned int getCoolStepValue() = 0;   ///< Get the CoolStepValue (in units?, what does it
                                                   ///< mean? Expert?)

    MCX_DECLARE_SET_GET_TYPED_REGISTER(AccelerationThresholdData, accelerationThresholdData);
    MCX_DECLARE_SET_GET_TYPED_REGISTER(ProportionalityFactorData, proportionalityFactorData);
    MCX_DECLARE_SET_GET_TYPED_REGISTER(ReferenceConfigAndRampModeData, referenceConfigAndRampModeData);
    MCX_DECLARE_SET_GET_TYPED_REGISTER(InterruptData, interruptData);
    MCX_DECLARE_SET_GET_TYPED_REGISTER(DividersAndMicroStepResolutionData, dividersAndMicroStepResolutionData);

    virtual void setDriverControlData(DriverControlData const& driverControlData) = 0;
    virtual void setChopperControlData(ChopperControlData const& chopperControlData) = 0;
    virtual void setCoolStepControlData(CoolStepControlData const& coolStepControlData) = 0;
    virtual void setStallGuardControlData(StallGuardControlData const& stallGuardControlData) = 0;
    virtual void setDriverConfigData(DriverConfigData const& driverConfigData) = 0;

    [[nodiscard]] virtual DriverControlData const& getDriverControlData() const = 0;
    [[nodiscard]] virtual ChopperControlData const& getChopperControlData() const = 0;
    [[nodiscard]] virtual CoolStepControlData const& getCoolStepControlData() const = 0;
    [[nodiscard]] virtual StallGuardControlData const& getStallGuardControlData() const = 0;
    [[nodiscard]] virtual DriverConfigData const& getDriverConfigData() const = 0;
  };

} // namespace ChimeraTK
