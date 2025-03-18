// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <MotorDriverCardConfigXML.h>

int main() {
  ChimeraTK::MotorDriverCardConfigXML::write("MotorDriverCardDefaultConfig.xml", ChimeraTK::MotorDriverCardConfig());

  return 0;
}
