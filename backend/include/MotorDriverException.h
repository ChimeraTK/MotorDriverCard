#ifndef MTCA4U_MOTOR_DRIVER_EXCEPTION_H
#define MTCA4U_MOTOR_DRIVER_EXCEPTION_H

#include <exception>
#include <string>

namespace mtca4u

/*FIXME This does not conform with the new ChimeraTK exception scheme.
 *      Remove this class and use ChimeraTK/Exception.h instead.
 */
{
  /** The type of exception thrown by MotorDriverCard and MotorControler.
   */
  class MotorDriverException: public std::exception {
  public:
    /** The different error types of the MotorDriverException.
     */
    enum ExceptionID { WRONG_MOTOR_ID, SPI_TIMEOUT, SPI_ERROR, WRONG_FIRMWARE_VERSION,
		       XML_ERROR, OUT_OF_RANGE, NOT_IMPLEMENTED};

    protected:
      std::string  _message;      /**< exception description*/
      ExceptionID  _id;           /**< exception ID*/
    
  public:
    /** The constuctor gets a message and an ID as arguments.
     */
    MotorDriverException(const std::string & message, ExceptionID id):
      _message(message), _id(id) {}

    /** What returns the expection message.
     */
  virtual const char* what() const throw(){
      return _message.c_str();
    }

    /** Returns the ID to progammatically distinguish the exceptions.
     */
    virtual ExceptionID getID() const{
      return _id;
    }
	
    virtual ~MotorDriverException() throw(){}
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_EXCEPTION_H
