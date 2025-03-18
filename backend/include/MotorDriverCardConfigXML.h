// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "MotorDriverCardConfig.h"

namespace ChimeraTK {

  class MotorDriverCardConfigXML {
   public:
    static MotorDriverCardConfig read(std::string fileName);
    static void write(std::string fileName, MotorDriverCardConfig const& motorDriverCardConfig);
    static void writeSparse(std::string fileName, MotorDriverCardConfig const& motorDriverCardConfig);

   private:
    static void write(std::string fileName, MotorDriverCardConfig const& motorDriverCardConfig, bool sparse);
  };

} // namespace ChimeraTK
