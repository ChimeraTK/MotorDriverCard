#ifndef MTCA4U_XML_EXCEPTION_H
#define MTCA4U_XML_EXCEPTION_H

#include <MtcaMappedDevice/exBase.h>

namespace mtca4u{

  class XMLException: public exBase {
  public:
    
    XMLException(const std::string & message):
      exBase(message, 0) {}
    virtual ~XMLException() throw(){}
  };

}// namespace mtca4u

#endif // MTCA4U_XML_
