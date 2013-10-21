#ifndef MTCA4U_TMC429REGISTER_H
#define MTCA4U_TMC429REGISTER_H

#include "MultiVariableRegister.h"

namespace mtca4u{
  
  /** A generic single value register of the TMC429 chip.
   *  It implements the full maximum 24 bit data payload. For
   *  shorter registers use an appropriate mask or range check
   *  before setting the values to the data field.
   *  
   *  Classes inheriting from this generic class can provide 
   *  more fine grained access to the data field if this is 
   *  interpreted as multiple variables.
   *
   *  For interpretation of the variables consult the TMC429 data sheet,
   *  chapter 8, page 17ff, especially table 8-2.
   *
   *  We intentionally break the naming convention here and go for
   *  data sheed syntax. This also makes it easier to distiguisch 
   *  MultiVariableRegister::getDataWord() (the full 32 bit content of the register)
   *  from TMC429Register::getDATA() (the data payload sub word as described in the 
   *  data sheet).
   */
  class TMC429Register: public MultiVariableRegister
  {
    public:
      ADD_VARIABLE(SMDA, 29, 30 );
      ADD_VARIABLE(IDX_JDX, 25, 28 );
      ADD_VARIABLE(RW , 24, 24);
      ADD_VARIABLE(DATA, 0, 23);
  };

}// namespace mtca4u

#endif// MTCA4U_TMC429REGISTER_H
