#ifndef MTCA4U_MOTOR_DRIVER_CARD_CONFIG_XML_H
#define MTCA4U_MOTOR_DRIVER_CARD_CONFIG_XML_H

#include <pugixml.hpp>

#include "MotorDriverCardConfig.h"
#include "MotorControlerConfig.h"

namespace mtca4u{

  class MotorDriverCardConfigXML{
  public:
    static MotorDriverCardConfig read(std::string fileName);
    static void write(std::string fileName, MotorDriverCardConfig const & motorDriverCardConfig);
  };
  
}// namespace mtca4u


#endif // MTCA4U_MOTOR_DRIVER_CARD_CONFIG_XML_H
