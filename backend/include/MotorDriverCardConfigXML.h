#ifndef MTCA4U_MOTOR_DRIVER_CARD_CONFIG_XML_H
#define MTCA4U_MOTOR_DRIVER_CARD_CONFIG_XML_H

#include "MotorControlerConfig.h"
#include "MotorDriverCardConfig.h"

namespace pugi {
  class xml_node;
}

namespace mtca4u {

  class MotorDriverCardConfigXML {
   public:
    static MotorDriverCardConfig read(std::string fileName);
    static void write(std::string fileName, MotorDriverCardConfig const& motorDriverCardConfig);
    static void writeSparse(std::string fileName, MotorDriverCardConfig const& motorDriverCardConfig);

   private:
    static void write(std::string fileName, MotorDriverCardConfig const& motorDriverCardConfig, bool sparse);

    /** The parameter content is passed by reference because it serves as default
     *value and is overwritten if the value is found in the XML file.
     */
    static void setValueIfFound(std::string const& parameterName,
        unsigned int& parameterContent,
        pugi::xml_node const& parentNode,
        std::string const& tagName = "Register");

    static void setValueIfFound(std::string const& parameterName,
        int& parameterContent,
        pugi::xml_node const& parentNode,
        std::string const& tagName = "Register");

    static void setValueIfFound(std::string const& registerName,
        TMC429InputWord& inputWord,
        pugi::xml_node const& parentNode);

    static void setValueIfFound(std::string const& registerName,
        TMC260Word& inputWord,
        pugi::xml_node const& parentNode);

    static void setValueIfFound(std::string const& parameterName, bool& flag, pugi::xml_node const& parentNode,
        std::string const& tagName = "Register");

    static MotorControlerConfig parseControlerConfig(pugi::xml_node const& controlerConfigXML);

    /** As we do not use a validating parser, we have to do checks on schema
     * evolution manually :-( check that the old name is not there so we don't
     * accidentally ignore a parameter that is meant to be set.
     */
    static void checkForOldInvalidTagName(std::string const& parameterName,
        pugi::xml_node const& parentNode,
        std::string const& tagName = "Register");

    class NodeFiller {
     public:
      NodeFiller(pugi::xml_node& node, bool sparse);
      void addParameter(std::string const& parameterName, unsigned int value, unsigned int defaultValue,
          std::string const& tagName = "Register");
      void addParameter(
          std::string const& parameterName, int value, int defaultValue, std::string const& tagName = "Register");
      void addParameter(std::string const& registerName,
          TMC429InputWord const& value,
          TMC429InputWord const& defaultValue);
      void addParameter(std::string const& registerName, TMC260Word const& value, TMC260Word const& defaultValue);
      void addParameter(
          std::string const& parameterName, bool value, bool defaultValue, std::string const& tagName = "Register");

     private:
      pugi::xml_node& _node;
      bool _writeAlways;

      std::string toHexString(unsigned int value);
      template<class T>
      std::string toDecString(T value);
      // std::string toDecString(int value);
    };
  };

} // namespace mtca4u

#endif // MTCA4U_MOTOR_DRIVER_CARD_CONFIG_XML_H
