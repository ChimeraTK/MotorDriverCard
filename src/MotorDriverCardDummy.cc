#include "MotorDriverCardDummy.h"

#include <stdexcept>

#include "MotorControlerDummy.h"
#include "MotorDriverException.h"
#include "DFMC_MD22Constants.h"
using namespace mtca4u::dfmc_md22;

namespace mtca4u{

MotorDriverCardDummy::MotorDriverCardDummy()
  : _motorControlers(N_MOTORS_MAX)
{
  for (unsigned int id = 0; id < _motorControlers.size() ; ++id){
    _motorControlers[id].reset( new MotorControlerDummy(id) );
  }
}

boost::shared_ptr<MotorControler> MotorDriverCardDummy::getMotorControler(unsigned int motorControlerID){
  try{
    return _motorControlers.at(motorControlerID);
  }
  catch(std::out_of_range &e){
    std::stringstream errorMessage;
      errorMessage << "motorControlerID " << motorControlerID << " is too large. "
		   << e.what();
      throw MotorDriverException(errorMessage.str(),
				 MotorDriverException::WRONG_MOTOR_ID);
  }
}
  
PowerMonitor & MotorDriverCardDummy::getPowerMonitor(){
  throw MotorDriverException( "getPowerMonitor() is not implemented inMotorDriverCardDummy", MotorDriverException::NOT_IMPLEMENTED );	
}


}// namespace mtca4u
