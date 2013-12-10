#ifndef OUT_OF_RANGE_EXCEPTION_H
#define OUT_OF_RANGE_EXCEPTION_H

#include <MtcaMappedDevice/exBase.h>

namespace mtca4u{

  class OutOfRangeException: public exBase {
  public:
    
    enum {  TOO_LARGE, TOO_SMALL };
    
    OutOfRangeException(const std::string & message, unsigned int id):
      exBase(message, id) {}
    virtual ~OutOfRangeException() throw(){}
  };

}// namespace mtca4u

#endif // OUT_OF_RANGE_EXCEPTION_H
