/*
 * StepperMotorException.h
 *
 *  Created on: Oct 26, 2016
 *      Author: vitimic
 */

#ifndef INCLUDE_STEPPERMOTOREXCEPTION_H_
#define INCLUDE_STEPPERMOTOREXCEPTION_H_

#include <exception>
#include <string>

#define EXCEPTION_MESSAGE                                                      \
  std::string("error in ") + std::string(__PRETTY_FUNCTION__)

namespace ChimeraTK {

/** The type of exception thrown by StepperMotor.
 *
 */
class StepperMotorException : public std::exception {
public:
  /** The different error types of the StepperMotor.
   */
  enum ExceptionID { NOT_VALID_CONVERTER, FEATURE_NOT_AVAILABLE };

protected:
  std::string _message; /**< exception description*/
  ExceptionID _id;      /**< exception ID*/

public:
  /** The constuctor gets a message and an ID as arguments.
   */
  StepperMotorException(const std::string &message, ExceptionID id)
      : _message(message), _id(id) {}

  /** What returns the expection message.
   */
  virtual const char *what() const throw() { return _message.c_str(); }

  /** Returns the ID to progammatically distinguish the exceptions.
   */
  virtual ExceptionID getID() const { return _id; }

  virtual ~StepperMotorException() throw() {}
};

} // namespace ChimeraTK

namespace mtca4u {
using namespace ChimeraTK;
}

#endif /* INCLUDE_STEPPERMOTOREXCEPTION_H_ */
