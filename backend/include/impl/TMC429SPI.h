// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "SPIviaPCIe.h"
#include "TMC429Words.h"

namespace ChimeraTK {
  /** This class implements the special format of the SPI registers in the TPC429
   * chip.
   */
  class TMC429SPI {
   public:
    TMC429SPI(boost::shared_ptr<ChimeraTK::Device> const& device, std::string const& moduleName,
        std::string const& writeRegisterName, std::string const& syncRegisterName,
        std::string const& readbackRegisterName, unsigned int spiWaitingTime = SPIviaPCIe::SPI_DEFAULT_WAITING_TIME);

    TMC429OutputWord read(unsigned int smda, unsigned int idx_jdx);
    void write(unsigned int smda, unsigned int idx_jdx, unsigned int data);
    void write(TMC429InputWord const& writeWord);

   private:
    SPIviaPCIe _spiViaPCIe;
  };

} // namespace ChimeraTK
