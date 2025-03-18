// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "MotorDriverCardFactory.h"

#include <ChimeraTK/BackendFactory.h>
#include <ChimeraTK/DMapFileParser.h>

#include <iostream>
#include <unistd.h>

using namespace ChimeraTK;

int main(int argc, char* argv[]) {
  if(argc != 4) {
    std::cout << argv[0]
              << " just creates an instance of MotorDirverCardImpl, which "
                 "initialises all registers."
              << std::endl;
    std::cout << "You can use this programme to quickly reset all registers "
                 "and turn off the drivers."
              << std::endl
              << std::endl;
    std::cout << "Usage: " << argv[0] << " dmapFile moduleName motorConfig.xml" << std::endl;
    return -1;
  }

  std::string moduleName = argv[2];
  std::string motorConfigFileName = argv[3];

  std::string dmapFileName = argv[1];
  std::string deviceAlias = ChimeraTK::DMapFileParser().parse(dmapFileName)->begin()->deviceName;
  ChimeraTK::BackendFactory::getInstance().setDMapFilePath(dmapFileName);

  (void)ChimeraTK::MotorDriverCardFactory::instance().createMotorDriverCard(
      deviceAlias, moduleName, motorConfigFileName);
}
