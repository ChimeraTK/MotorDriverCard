#include "MotorDriverCardConfigXML.h"
#include "XMLException.h"

#include <pugixml.hpp>


namespace mtca4u{

MotorDriverCardConfig  MotorDriverCardConfigXML::read(std::string fileName){
  pugi::xml_document doc;
  if ( !doc.load_file(fileName.c_str()) ){
    std::stringstream message;
    message << "Could not load XML file \"" << fileName << "\"";
    throw XMLException(message.str());
  }

  // a config with default values
  MotorDriverCardConfig cardConfig;

  return cardConfig;
}

}// namespace mtca4u

