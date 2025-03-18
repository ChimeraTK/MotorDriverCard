// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "MotorDriverCardConfigXML.h"

#include <iostream>

using namespace ChimeraTK;

int main() {
  MotorDriverCardConfigXML::write("MotorDriverCardConfig_default.xml", MotorDriverCardConfig());
  return 0;
}
