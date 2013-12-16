#ifndef MTCA4U_MOTOR_DRIVER_CARD_CONFIG_XML_H
#define MTCA4U_MOTOR_DRIVER_CARD_CONFIG_XML_H

#include "MotorDriverCardConfig.h"
#include "MotorControlerConfig.h"

namespace pugi{
  class xml_node;
}

namespace mtca4u{

  class MotorDriverCardConfigXML{
  public:
    static MotorDriverCardConfig read(std::string fileName);
    static void write(std::string fileName, MotorDriverCardConfig const & motorDriverCardConfig);
    static void write_sparse(std::string fileName, MotorDriverCardConfig const & motorDriverCardConfig);

  private:
    static void write(std::string fileName, MotorDriverCardConfig const & motorDriverCardConfig,
		      bool sparse);

    /** The register content is passed by reference because it serves as default value and is overwritten
     *	if the value is found in the XML file.
     */
    static void setValueIfFound(std::string const & registerName, unsigned int & registerContent,
				pugi::xml_node const & parentNode);

    static void setValueIfFound(std::string const & registerName, TMC429InputWord * inputWord,
				pugi::xml_node const & parentNode);

    static void setValueIfFound(std::string const & registerName, TMC260Word * inputWord,
				pugi::xml_node const & parentNode);

    static void setValueIfFound(std::string const & registerName, bool & flag,
				pugi::xml_node const & parentNode);

    static MotorControlerConfig parseControlerConfig(  pugi::xml_node const & controlerConfigXML );

    class NodeFiller{
    public:
      NodeFiller(pugi::xml_node & node, bool sparse);
      void addRegister( std::string const & registerName, unsigned int value,
			unsigned int defaultValue);
      void addRegister( std::string const & registerName, TMC429InputWord const & value,
			TMC429InputWord const & defaultValue);
    private:
      pugi::xml_node & _node;
      bool _writeAlways;
    };
  };

}// namespace mtca4u


#endif // MTCA4U_MOTOR_DRIVER_CARD_CONFIG_XML_H
