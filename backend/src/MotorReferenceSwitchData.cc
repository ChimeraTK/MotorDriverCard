// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "MotorReferenceSwitchData.h"

namespace ChimeraTK {

  MotorReferenceSwitchData::MotorReferenceSwitchData(unsigned int dataWord) : MultiVariableWord(dataWord) {}

  MotorReferenceSwitchData::operator bool() const {
    return static_cast<bool>(getSwitchesActiveWord());
  }

} // namespace ChimeraTK
