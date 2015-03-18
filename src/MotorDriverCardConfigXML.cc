#include "MotorDriverCardConfigXML.h"
#include "XMLException.h"

#include <pugixml.hpp>

#define CARD_CONFIG_ADD_REGISTER( VALUE )\
  cardConfigFiller.addParameter(#VALUE,\
				motorDriverCardConfig. VALUE ,\
				defaultCardConfig. VALUE)

#define CARD_CONFIG_ADD_PARAMETER( VALUE )\
  cardConfigFiller.addParameter(#VALUE,\
				motorDriverCardConfig. VALUE ,\
				defaultCardConfig. VALUE ,\
				"Parameter")

#define CONTROLER_CONFIG_ADD_REGISTER( VALUE )\
  controlerConfigFiller.addParameter(#VALUE,\
				     motorControlerConfig. VALUE ,\
				     defaultControlerConfig. VALUE)

#define CONTROLER_CONFIG_ADD_PARAMETER( VALUE )\
  controlerConfigFiller.addParameter(#VALUE,\
				     motorControlerConfig. VALUE ,\
				     defaultControlerConfig. VALUE ,\
				     "Parameter")


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

  pugi::xml_node cardConfigXML = doc.child("MotorDriverCardConfig");
 
  setValueIfFound( "coverDatagram", cardConfig.coverDatagram, cardConfigXML );
  setValueIfFound( "coverPositionAndLength", cardConfig.coverPositionAndLength, cardConfigXML );
  setValueIfFound( "datagramHighWord", cardConfig.datagramHighWord, cardConfigXML );
  setValueIfFound( "datagramLowWord", cardConfig.datagramLowWord, cardConfigXML );
  setValueIfFound( "interfaceConfiguration",  cardConfig.interfaceConfiguration, cardConfigXML );
  setValueIfFound( "positionCompareInterruptData",  cardConfig.positionCompareInterruptData, cardConfigXML );
  setValueIfFound( "positionCompareWord", cardConfig.positionCompareWord, cardConfigXML );
  setValueIfFound( "stepperMotorGlobalParameters",  cardConfig.stepperMotorGlobalParameters, cardConfigXML );
  setValueIfFound( "controlerSpiWaitingTime", cardConfig.controlerSpiWaitingTime, cardConfigXML,
		   "Parameter");

  for(size_t i = 0; i < cardConfig.motorControlerConfigurations.size(); ++i){
    std::stringstream motorIDAsString;
    motorIDAsString << i;
    pugi::xml_node controlerConfigXML = cardConfigXML.find_child_by_attribute("MotorControlerConfig","motorID",
								    motorIDAsString.str().c_str());
    if ( controlerConfigXML ){
      cardConfig.motorControlerConfigurations[i] = parseControlerConfig( controlerConfigXML );
    }
 }

  return cardConfig;
}

void MotorDriverCardConfigXML::setValueIfFound(std::string const & parameterName,
					       unsigned int & parameterContent,
					       pugi::xml_node const & parentNode,
					       std::string const & tagName){
  pugi::xml_node parameterNode = parentNode.find_child_by_attribute(tagName.c_str(), "name", 
								    parameterName.c_str());
  parameterContent = parameterNode.attribute("value").as_int( parameterContent /* = default vale*/ );
}

void MotorDriverCardConfigXML::setValueIfFound(std::string const & parameterName,
					       bool & flag,
					       pugi::xml_node const & parentNode,
					       std::string const & tagName){
  pugi::xml_node parameterNode = parentNode.find_child_by_attribute(tagName.c_str(), "name",
								    parameterName.c_str());
  flag = parameterNode.attribute("value").as_bool( flag );
}

void MotorDriverCardConfigXML::setValueIfFound(std::string const & registerName,
					       TMC429InputWord & inputWord,
					       pugi::xml_node const & parentNode){
  pugi::xml_node registerNode = parentNode.find_child_by_attribute("Register","name",registerName.c_str());
  unsigned int readValue = registerNode.attribute("value").as_int( inputWord.getDATA() /* = default vale*/ );
  inputWord.setDATA(readValue);
}

void MotorDriverCardConfigXML::setValueIfFound(std::string const & registerName,
					       TMC260Word & inputWord,
					       pugi::xml_node const & parentNode){
  pugi::xml_node registerNode = parentNode.find_child_by_attribute("Register","name",registerName.c_str());
  unsigned int readValue = registerNode.attribute("value").as_int( inputWord.getPayloadData() /* = default vale*/ );
  inputWord.setPayloadData(readValue);
}

MotorControlerConfig MotorDriverCardConfigXML::parseControlerConfig(  pugi::xml_node const & controlerConfigXML ){
  MotorControlerConfig controlerConfig;

  setValueIfFound("accelerationThresholdData", controlerConfig.accelerationThresholdData, controlerConfigXML);
  setValueIfFound("actualPosition", controlerConfig.actualPosition, controlerConfigXML);
  setValueIfFound("chopperControlData", controlerConfig.chopperControlData, controlerConfigXML);
  setValueIfFound("coolStepControlData", controlerConfig.coolStepControlData, controlerConfigXML);
  setValueIfFound("decoderReadoutMode", controlerConfig.decoderReadoutMode, controlerConfigXML);
  setValueIfFound("dividersAndMicroStepResolutionData", controlerConfig.dividersAndMicroStepResolutionData,
		  controlerConfigXML);
  setValueIfFound("driverConfigData", controlerConfig.driverConfigData, controlerConfigXML);
  setValueIfFound("driverControlData", controlerConfig.driverControlData, controlerConfigXML);
  setValueIfFound("enabled", controlerConfig.enabled, controlerConfigXML);
  setValueIfFound("interruptData", controlerConfig.interruptData, controlerConfigXML);
  setValueIfFound("maximumAccelleration", controlerConfig.maximumAccelleration, controlerConfigXML);
  setValueIfFound("maximumVelocity", controlerConfig.maximumVelocity, controlerConfigXML);
  setValueIfFound("microStepCount", controlerConfig.microStepCount, controlerConfigXML);
  setValueIfFound("minimumVelocity", controlerConfig.minimumVelocity, controlerConfigXML);
  setValueIfFound("positionTolerance", controlerConfig.positionTolerance, controlerConfigXML);
  setValueIfFound("proportionalityFactorData", controlerConfig.proportionalityFactorData, controlerConfigXML);
  setValueIfFound("referenceConfigAndRampModeData", controlerConfig.referenceConfigAndRampModeData,
		  controlerConfigXML);
  setValueIfFound("stallGuardControlData", controlerConfig.stallGuardControlData, controlerConfigXML);
  setValueIfFound("targetPosition", controlerConfig.targetPosition, controlerConfigXML);
  setValueIfFound("targetVelocity", controlerConfig.targetVelocity, controlerConfigXML);  
  setValueIfFound("driverSpiWaitingTime", controlerConfig.driverSpiWaitingTime, controlerConfigXML,
		  "Parameter");  

  return controlerConfig;
}

  void MotorDriverCardConfigXML::write(std::string fileName, 
				       MotorDriverCardConfig const & motorDriverCardConfig){
    write( fileName, motorDriverCardConfig, false );
  }

  void MotorDriverCardConfigXML::writeSparse(std::string fileName, 
				       MotorDriverCardConfig const & motorDriverCardConfig){
    write( fileName, motorDriverCardConfig, true );
  }


  void MotorDriverCardConfigXML::write(std::string fileName, 
				       MotorDriverCardConfig const & motorDriverCardConfig,
				       bool sparse){
    pugi::xml_document doc;
    pugi::xml_node cardConfigXML = doc.append_child("MotorDriverCardConfig");

    MotorDriverCardConfig defaultCardConfig;

    NodeFiller cardConfigFiller(cardConfigXML, sparse);

    CARD_CONFIG_ADD_REGISTER( coverDatagram );
    CARD_CONFIG_ADD_REGISTER( coverPositionAndLength );
    CARD_CONFIG_ADD_REGISTER( datagramHighWord );
    CARD_CONFIG_ADD_REGISTER( datagramLowWord );
    CARD_CONFIG_ADD_REGISTER( interfaceConfiguration );
    CARD_CONFIG_ADD_REGISTER( positionCompareInterruptData );
    CARD_CONFIG_ADD_REGISTER( positionCompareWord );
    CARD_CONFIG_ADD_REGISTER( stepperMotorGlobalParameters );
    CARD_CONFIG_ADD_PARAMETER( controlerSpiWaitingTime );

    for (unsigned int i=0; i < motorDriverCardConfig.motorControlerConfigurations.size(); ++i){
      MotorControlerConfig defaultControlerConfig;
      MotorControlerConfig motorControlerConfig = motorDriverCardConfig.motorControlerConfigurations[i];

      if (sparse && (motorControlerConfig == defaultControlerConfig )){
	continue;
      }

      pugi::xml_node controlerConfigXML = cardConfigXML.append_child("MotorControlerConfig");
      controlerConfigXML.append_attribute("motorID") = i;

      NodeFiller controlerConfigFiller(controlerConfigXML, sparse);
      
      CONTROLER_CONFIG_ADD_REGISTER( accelerationThresholdData );
      CONTROLER_CONFIG_ADD_REGISTER( actualPosition );
      CONTROLER_CONFIG_ADD_REGISTER( chopperControlData );
      CONTROLER_CONFIG_ADD_REGISTER( coolStepControlData );
      CONTROLER_CONFIG_ADD_REGISTER( decoderReadoutMode );
      CONTROLER_CONFIG_ADD_REGISTER( dividersAndMicroStepResolutionData );
      CONTROLER_CONFIG_ADD_REGISTER( driverConfigData );
      CONTROLER_CONFIG_ADD_REGISTER( driverControlData );
      CONTROLER_CONFIG_ADD_REGISTER( enabled);
      CONTROLER_CONFIG_ADD_REGISTER( interruptData );
      CONTROLER_CONFIG_ADD_REGISTER( maximumAccelleration );
      CONTROLER_CONFIG_ADD_REGISTER( maximumVelocity );
      CONTROLER_CONFIG_ADD_REGISTER( microStepCount );
      CONTROLER_CONFIG_ADD_REGISTER( minimumVelocity );
      CONTROLER_CONFIG_ADD_REGISTER( positionTolerance );
      CONTROLER_CONFIG_ADD_REGISTER( proportionalityFactorData );
      CONTROLER_CONFIG_ADD_REGISTER( referenceConfigAndRampModeData );
      CONTROLER_CONFIG_ADD_REGISTER( stallGuardControlData );
      CONTROLER_CONFIG_ADD_REGISTER( targetPosition );
      CONTROLER_CONFIG_ADD_REGISTER( targetVelocity );
      CONTROLER_CONFIG_ADD_PARAMETER( driverSpiWaitingTime );

    }

    if (!doc.save_file( fileName.c_str() )){
      std::stringstream message;
      message << "Could not write XML file \"" << fileName << "\"";
      throw XMLException(message.str());
    }
  }

  MotorDriverCardConfigXML::NodeFiller::NodeFiller(pugi::xml_node & node, bool sparse)
    : _node(node), _writeAlways( !sparse ){
  }

  void MotorDriverCardConfigXML::NodeFiller::addParameter( std::string const & parameterName, 
							   unsigned int value,
							   unsigned int defaultValue,
							   std::string const & tagName){
    if ( _writeAlways || (value != defaultValue) ){
      pugi::xml_node parameterNode = _node.append_child(tagName.c_str());
      parameterNode.append_attribute("name") = parameterName.c_str();
      // register contents is written as hex, other parameters are written as decimal
      std::string valueString;
      if (tagName == "Register"){
	valueString  = toHexString(value);
      }
      else{
	valueString  = toDecString(value);
      }
      parameterNode.append_attribute("value") = valueString.c_str();
    }
  }

  void MotorDriverCardConfigXML::NodeFiller::addParameter( std::string const & registerName, 
							   TMC429InputWord const & value,
							   TMC429InputWord const & defaultValue){
    addParameter( registerName, value.getDATA(), defaultValue.getDATA() );
  }

  void MotorDriverCardConfigXML::NodeFiller::addParameter( std::string const & registerName, 
							   TMC260Word const & value,
							   TMC260Word const & defaultValue){
    addParameter( registerName, value.getPayloadData(), defaultValue.getPayloadData() );
  }

  void MotorDriverCardConfigXML::NodeFiller::addParameter( std::string const & parameterName, 
							   bool value,
							   bool defaultValue,
							   std::string const & tagName){
    if ( _writeAlways || (value != defaultValue) ){
      pugi::xml_node parameterNode = _node.append_child(tagName.c_str());
      parameterNode.append_attribute("name") = parameterName.c_str();
      parameterNode.append_attribute("value") = (value?"true":"false");
    }
  }

  std::string MotorDriverCardConfigXML::NodeFiller::toHexString(unsigned int value){
    std::stringstream s;
    s << "0x" << std::uppercase << std::hex << value;
    return s.str();
  }

  std::string MotorDriverCardConfigXML::NodeFiller::toDecString(unsigned int value){
    std::stringstream s;
    s << std::dec << value;
    return s.str();
  }

}// namespace mtca4u

