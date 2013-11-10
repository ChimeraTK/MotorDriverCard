#ifndef MTCA4U_MOTOR_DRIVER_CARD_H
#define MTCA4U_MOTOR_DRIVER_CARD_H

#include "TMC429Words.h"
#include <MtcaMappedDevice/exBase.h>

namespace mtca4u
{
  class MotorControler;
  class PowerMonitor;

  class MotorDriverException: public exBase {
  public:
    
    enum {  WRONG_MOTOR_ID, SPI_READ_TIMEOUT };
    
    MotorDriverException(const std::string & message, unsigned int id):
      exBase(message, id) {}
    virtual ~MotorDriverException() throw(){}
  };


  /**
   * A class to access the DFMC-MD22 motor driver card, which provides two 
   * MotorControlers.
   */
  class MotorDriverCard{
  public:
    /** Get acces to one of the two motor controlers on this board. 
     *  Valid IDs are 0 and 1.
     *  Throws a MotorDriverException if the ID is invalid.
     */
    virtual MotorControler & getMotorControler(unsigned int motorControlerID) = 0;
    
    virtual ReferenceSwitchData getReferenceSwitchRegister() = 0;

    virtual unsigned int getControlerChipVersion() = 0;
 
    /// Get a reference to the power monitor.
    virtual PowerMonitor & getPowerMonitor() = 0;
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_H
