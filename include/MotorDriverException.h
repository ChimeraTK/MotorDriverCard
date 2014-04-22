#ifndef MTCA4U_MOTOR_DRIVER_EXCEPTION_H
#define MTCA4U_MOTOR_DRIVER_EXCEPTION_H

#include <MtcaMappedDevice/exBase.h>

namespace mtca4u
{
  /** The type of exception thrown by MotorDriverCard and MotorControler.
   */
  class MotorDriverException: public mtca4u::exBase {
  public:
    
    /** The different error types of the MotorDriverException.
     */
    enum {  WRONG_MOTOR_ID, SPI_TIMEOUT, SPI_ERROR };
    
    MotorDriverException(const std::string & message, unsigned int id):
      exBase(message, id) {}
    virtual ~MotorDriverException() throw(){}
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_EXCEPTION_H
