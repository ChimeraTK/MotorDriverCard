#include "MotorReferenceSwitchData.h"

namespace mtca4u{
  
  MotorReferenceSwitchData::MotorReferenceSwitchData(unsigned int dataWord)
    : MultiVariableWord( dataWord )
  {}

  MotorReferenceSwitchData::operator bool() const{
    return static_cast<bool>(getSwitchesActiveWord());
  }

}//namespace mtca4u
