#ifndef MTCA4U_TMC260WORDS_H
#define MTCA4U_TMC260WORDS_H

#include "MultiVariableWord.h"

namespace mtca4u{

  //FIXME implement all the types correctly. To make it compile we
  // start with typedefs
  typedef MultiVariableWord DriverControlData;
  typedef MultiVariableWord ChopperControlData;
  typedef MultiVariableWord CoolStepControlData;
  typedef MultiVariableWord StallGuardControlData;
  typedef MultiVariableWord DriverConfigData;
    

}// namespace mtca4u

#endif// MTCA4U_TMC260WORDS_H
