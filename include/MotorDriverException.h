#ifndef MTCA4U_MOTOR_DRIVER_EXCEPTION_H
#define MTCA4U_MOTOR_DRIVER_EXCEPTION_H

#include <mtca4u/Exception.h>

namespace mtca4u
{
  /** The type of exception thrown by MotorDriverCard and MotorControler.
   */
  class MotorDriverException: public mtca4u::Exception {
  public:
    
    /** The different error types of the MotorDriverException.
     */
    enum {  WRONG_MOTOR_ID, SPI_TIMEOUT, SPI_ERROR, WRONG_FIRMWARE_VERSION };
    
    MotorDriverException(const std::string & message, unsigned int id):
      Exception(message, id) {}
    virtual ~MotorDriverException() throw(){}
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_EXCEPTION_H
