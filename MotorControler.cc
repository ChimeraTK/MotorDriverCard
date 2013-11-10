#include "MotorControler.h"

#include <boost/lexical_cast.hpp>
#include "MotorDriverCardImpl.h"

// just save some typing...
#define REG_OBJECT_FROM_SUFFIX( SUFFIX )\
  _driverCard._mappedDevice->getRegObject( createMotorRegisterName( SUFFIX ) )

namespace mtca4u
{

  MotorControler::MotorControler( unsigned int ID, MotorDriverCardImpl & driverCard ) 
    : _driverCard(driverCard) , _id(ID), 
      _actualPosition( REG_OBJECT_FROM_SUFFIX( "ACTUAL_POS" ) )
  {}

  unsigned int MotorControler::getID(){
    return _id;
  }
   
  unsigned int MotorControler::getActualPosition(){
    int readValue;
    _actualPosition.readReg( &readValue );
    return static_cast<unsigned int>(readValue) ;
  }

  std::string  MotorControler::createMotorRegisterName( std::string suffix ){
    return std::string("WORD_M")
           + boost::lexical_cast<std::string>(_id+1) + "_"
           + suffix;
  }

}// namespace mtca4u
