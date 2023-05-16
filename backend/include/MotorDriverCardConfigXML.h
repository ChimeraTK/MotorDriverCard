#ifndef MTCA4U_MOTOR_DRIVER_CARD_CONFIG_XML_H
#define MTCA4U_MOTOR_DRIVER_CARD_CONFIG_XML_H

#include "MotorDriverCardConfig.h"

namespace mtca4u {

  class MotorDriverCardConfigXML {
   public:
    static MotorDriverCardConfig read(std::string fileName);
    static void write(std::string fileName, MotorDriverCardConfig const& motorDriverCardConfig);
    static void writeSparse(std::string fileName, MotorDriverCardConfig const& motorDriverCardConfig);

   private:
    static void write(std::string fileName, MotorDriverCardConfig const& motorDriverCardConfig, bool sparse);
  };

} // namespace mtca4u

#endif // MTCA4U_MOTOR_DRIVER_CARD_CONFIG_XML_H
