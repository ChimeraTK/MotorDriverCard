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
      _actualPosition( REG_OBJECT_FROM_SUFFIX( "ACTUAL_POS" ) ),
      _actualVelocity( REG_OBJECT_FROM_SUFFIX( "V_ACTUAL" ) )
  {}

  unsigned int MotorControler::getID(){
    return _id;
  }
   
  unsigned int MotorControler::getActualPosition(){
    return readRegObject( _actualPosition );
  }

  unsigned int MotorControler::readRegObject( devMap<devBase>::regObject const & registerAccessor){
    int readValue;
    registerAccessor.readReg( &readValue );
    return static_cast<unsigned int>(readValue);
  }

  void MotorControler::setActualPosition(unsigned int position){
    _actualPosition.writeReg(reinterpret_cast<int32_t *>(&position));
  }

  unsigned int MotorControler::getActualVelocity(){
    return readRegObject( _actualVelocity );
  }

  void MotorControler::setActualVelocity(unsigned int velocity){
    _actualVelocity.writeReg(reinterpret_cast<int32_t *>(&velocity));
  }

  std::string  MotorControler::createMotorRegisterName( std::string suffix ){
    return std::string("WORD_M")
           + boost::lexical_cast<std::string>(_id+1) + "_"
           + suffix;
  }

}// namespace mtca4u
