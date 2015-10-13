#ifndef OUT_OF_RANGE_EXCEPTION_H
#define OUT_OF_RANGE_EXCEPTION_H

#include <mtca4u/Exception.h>

namespace mtca4u{

  class OutOfRangeException: public mtca4u::Exception {
  public:
    
    enum {  TOO_LARGE, TOO_SMALL };
    
    OutOfRangeException(const std::string & message, unsigned int id):
      Exception(message, id) {}
    virtual ~OutOfRangeException() throw(){}
  };

}// namespace mtca4u

#endif // OUT_OF_RANGE_EXCEPTION_H
