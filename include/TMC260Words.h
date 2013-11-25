#ifndef MTCA4U_TMC260WORDS_H
#define MTCA4U_TMC260WORDS_H

#include "MultiVariableWord.h"
#include "TMC260Constants.h"

namespace mtca4u{

  //FIXME implement all the types correctly. To make it compile we
  // start with typedefs
  class DriverControlData: public MultiVariableWord{
  public:
    DriverControlData(unsigned int dataWord = 0): MultiVariableWord(){
      setAddress(tmc260::ADDRESS_DRIVER_CONTROL);
      setPayloadData(dataWord);}
    ADD_VARIABLE(Address, 18, 19);
    ADD_VARIABLE(PayloadData, 0, 17);
    ADD_VARIABLE(Interpolation, 9, 9);
    ADD_VARIABLE(DoubleEdge, 8, 8);
    ADD_VARIABLE(MicroStepResolution, 0, 3);
  };

  class ChopperControlData: public MultiVariableWord{
  public:
    ChopperControlData(unsigned int dataWord = 0): MultiVariableWord(){
      setAddress(tmc260::ADDRESS_CHOPPER_CONFIG);
      setPayloadData(dataWord);}
    ADD_VARIABLE(Address, 17, 19);
    ADD_VARIABLE(PayloadData, 0, 16);
    ADD_VARIABLE(BlankingTime, 15, 16);
    ADD_VARIABLE(ChopperMode, 14, 14);   
    ADD_VARIABLE(RandomOffTime, 13, 13);
    ADD_VARIABLE(HysteresisDecrementInterval, 11, 12);
    ADD_VARIABLE(HysteresisEndValue, 7, 10);
    ADD_VARIABLE(HysteresisStartValue, 4, 6);
    ADD_VARIABLE(OffTime, 0, 3);    
  };

  class CoolStepControlData: public MultiVariableWord{
  public:
    CoolStepControlData(unsigned int dataWord = 0): MultiVariableWord(){
    setAddress(tmc260::ADDRESS_COOL_STEP_CONFIG);
      setPayloadData(dataWord);}
    ADD_VARIABLE(Address, 17, 19);
    ADD_VARIABLE(PayloadData, 0, 16);
  };
  
  class StallGuardControlData: public MultiVariableWord{
  public:
    StallGuardControlData(unsigned int dataWord = 0): MultiVariableWord(){
      setAddress(tmc260::ADDRESS_STALL_GUARD_CONFIG);
      setPayloadData(dataWord);}
    ADD_VARIABLE(Address, 17, 19);
    ADD_VARIABLE(PayloadData, 0, 16);
  };
  
  class DriverConfigData: public MultiVariableWord{
  public:
    DriverConfigData(unsigned int dataWord = 0): MultiVariableWord(){
      setAddress(tmc260::ADDRESS_DRIVER_CONFIG);
      setPayloadData(dataWord);}
    ADD_VARIABLE(Address, 17, 19);
    ADD_VARIABLE(PayloadData, 0, 16);
  };
    
}// namespace mtca4u

#endif// MTCA4U_TMC260WORDS_H
