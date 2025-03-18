// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ConfigCalculator.h"
#include "MotorDriverCardConfig.h"
#include "MotorDriverCardConfigXML.h"

#include <iostream>

int main(int argc, char* argv[]) {
  if((argc < 5) || (argc > 8)) {
    std::cout << "Calculate the MD22 config parameters from the "
              << "physical motor parameters and write them to a config "
              << "file. Both motors get the same config.\n\n"
              << "usage: " << argv[0] << " fileName.xml iMax nStepsPerTurn maxRPM [timeToVMax=0.5s]"
              << " [endSwitchConfig=0x0] [microsteps=16]\n\n"
              << " iMax: maximum coil current in A\n"
              << " nStepsPerTurn: number if full steps per turn\n"
              << " maxRPM: maximum velocity in RPM\n"
              << " timeToVMax: time to reach the maximum velocity in seconds\n\n"
              << "endSwitchConfig: \n"
              << " 0x0   Both end switches enalbled\n"
              << " 0x100 Ignore negative end switch\n"
              << " 0x200 Ignore positive end switch\n"
              << " 0x300 Ignore both end switches\n\n"
              << "microsteps must be one of {1,2,4,8,16,32,64}" << std::endl;
    return -1;
  }

  double iMax = atof(argv[2]);
  int nStepsPerTurn = atoi(argv[3]);
  double maxRPM = atof(argv[4]);

  double timeToVMax;
  if(argc > 5) {
    timeToVMax = atof(argv[5]);
  }
  else {
    timeToVMax = 0.5;
  }

  int endSwitchConfig;
  if(argc > 6) {
    endSwitchConfig = atof(argv[6]);
  }
  else {
    endSwitchConfig = 0x0;
  }

  int microsteps;
  if(argc > 7) {
    microsteps = atof(argv[7]);
  }
  else {
    microsteps = 16;
  }

  std::cout << argv[1] << " " << iMax << " " << nStepsPerTurn << " " << maxRPM << " " << timeToVMax << " " << std::hex
            << "0x" << endSwitchConfig << std::dec << " " << microsteps << std::endl;

  ParametersCalculator::PhysicalParameters physicalParameters(32, // system clock, hard coded
      microsteps, nStepsPerTurn, maxRPM, timeToVMax, iMax);

  ParametersCalculator::ChipParameters chipParameters = ParametersCalculator::calculateParameters(physicalParameters);

  if(chipParameters.warnings.size()) {
    std::cout << "\n Warnings during conversion:" << std::endl;
    for(std::list<std::string>::const_iterator it = chipParameters.warnings.begin();
        it != chipParameters.warnings.end(); ++it) {
      std::cout << *it << std::endl;
    }
  }

  ChimeraTK::MotorControlerConfig motorConfig = ConfigCalculator::calculateConfig(
      chipParameters, static_cast<ConfigCalculator::EndSwitchConfig>(endSwitchConfig));

  ChimeraTK::MotorDriverCardConfig cardConfig;
  // set all motor configs to the same, calculated config
  for(std::vector<ChimeraTK::MotorControlerConfig>::iterator it = cardConfig.motorControlerConfigurations.begin();
      it != cardConfig.motorControlerConfigurations.end(); ++it) {
    *it = motorConfig;
  }

  ChimeraTK::MotorDriverCardConfigXML::writeSparse(argv[1], cardConfig);

  return chipParameters.warnings.size();
}
