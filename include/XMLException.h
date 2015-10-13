#ifndef MTCA4U_XML_EXCEPTION_H
#define MTCA4U_XML_EXCEPTION_H

#include <mtca4u/Exception.h>

namespace mtca4u{

  class XMLException: public mtca4u::Exception {
  public:
    
    XMLException(const std::string & message):
      Exception(message, 0) {}
    virtual ~XMLException() throw(){}
  };

}// namespace mtca4u

#endif // MTCA4U_XML_
