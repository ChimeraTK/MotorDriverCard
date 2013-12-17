#include "MotorDriverCardConfigXML.h"
#include "XMLException.h"

#include <pugixml.hpp>

#define CARD_CONFIG_ADD( VALUE )\
  cardConfigFiller.addRegister(#VALUE,\
			       motorDriverCardConfig. VALUE ,\
				 defaultCardConfig. VALUE )

#define CONTROLER_CONFIG_ADD( VALUE )\
  controlerConfigFiller.addRegister(#VALUE,\
				    motorControlerConfig. VALUE ,\
				    defaultControlerConfig. VALUE )


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
  setValueIfFound( "coverPositionAndLength", &(cardConfig.coverPositionAndLength), cardConfigXML );
  setValueIfFound( "datagramHighWord", cardConfig.datagramHighWord, cardConfigXML );
  setValueIfFound( "datagramLowWord", cardConfig.datagramLowWord, cardConfigXML );
  setValueIfFound( "interfaceConfiguration",  &(cardConfig.interfaceConfiguration), cardConfigXML );
  setValueIfFound( "positionCompareInterruptData",  &(cardConfig.positionCompareInterruptData), cardConfigXML );
  setValueIfFound( "positionCompareWord", cardConfig.positionCompareWord, cardConfigXML );
  setValueIfFound( "stepperMotorGlobalParameters",  &(cardConfig.stepperMotorGlobalParameters), cardConfigXML );

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

void MotorDriverCardConfigXML::setValueIfFound(std::string const & registerName,
					       unsigned int & registerContent,
					       pugi::xml_node const & parentNode){
  pugi::xml_node registerNode = parentNode.find_child_by_attribute("Register","name",registerName.c_str());
  registerContent = registerNode.attribute("value").as_int( registerContent, /*default vale*/
							    0 ); /* readout mode, 
								 automatic hex and octal detection,
								 base 10 as default */
}

void MotorDriverCardConfigXML::setValueIfFound(std::string const & registerName,
					       bool & flag,
					       pugi::xml_node const & parentNode){
  pugi::xml_node registerNode = parentNode.find_child_by_attribute("Register","name",registerName.c_str());
  flag = registerNode.attribute("value").as_bool( flag );
}

void MotorDriverCardConfigXML::setValueIfFound(std::string const & registerName,
					       TMC429InputWord * inputWord,
					       pugi::xml_node const & parentNode){
  pugi::xml_node registerNode = parentNode.find_child_by_attribute("Register","name",registerName.c_str());
  unsigned int readValue = registerNode.attribute("value").as_int( inputWord->getDATA(), /*default vale*/
								   0 ); /* readout mode, 
									   automatic hex and octal detection,
									   base 10 as default */
  inputWord->setDATA(readValue);
}

void MotorDriverCardConfigXML::setValueIfFound(std::string const & registerName,
					       TMC260Word * inputWord,
					       pugi::xml_node const & parentNode){
  pugi::xml_node registerNode = parentNode.find_child_by_attribute("Register","name",registerName.c_str());
  unsigned int readValue = registerNode.attribute("value").as_int( inputWord->getPayloadData(), /*default vale*/
								   0 ); /* readout mode, 
									   automatic hex and octal detection,
									   base 10 as default */
  inputWord->setPayloadData(readValue);
}

MotorControlerConfig MotorDriverCardConfigXML::parseControlerConfig(  pugi::xml_node const & controlerConfigXML ){
  MotorControlerConfig controlerConfig;

  setValueIfFound("accelerationThresholdData", &(controlerConfig.accelerationThresholdData), controlerConfigXML);
  setValueIfFound("actualPosition", controlerConfig.actualPosition, controlerConfigXML);
  setValueIfFound("chopperControlData", &(controlerConfig.chopperControlData), controlerConfigXML);
  setValueIfFound("coolStepControlData", &(controlerConfig.coolStepControlData), controlerConfigXML);
  setValueIfFound("decoderReadoutMode", controlerConfig.decoderReadoutMode, controlerConfigXML);
  setValueIfFound("dividersAndMicroStepResolutionData", &(controlerConfig.dividersAndMicroStepResolutionData),
		  controlerConfigXML);
  setValueIfFound("driverConfigData", &(controlerConfig.driverConfigData), controlerConfigXML);
  setValueIfFound("driverControlData", &(controlerConfig.driverControlData), controlerConfigXML);
  setValueIfFound("enabled", controlerConfig.enabled, controlerConfigXML);
  setValueIfFound("interruptData", &(controlerConfig.interruptData), controlerConfigXML);
  setValueIfFound("maximumAccelleration", controlerConfig.maximumAccelleration, controlerConfigXML);
  setValueIfFound("maximumVelocity", controlerConfig.maximumVelocity, controlerConfigXML);
  setValueIfFound("microStepCount", controlerConfig.microStepCount, controlerConfigXML);
  setValueIfFound("minimumVelocity", controlerConfig.minimumVelocity, controlerConfigXML);
  setValueIfFound("positionTolerance", controlerConfig.positionTolerance, controlerConfigXML);
  setValueIfFound("proportionalityFactorData", &(controlerConfig.proportionalityFactorData), controlerConfigXML);
  setValueIfFound("referenceConfigAndRampModeData", &(controlerConfig.referenceConfigAndRampModeData),
		  controlerConfigXML);
  setValueIfFound("stallGuardControlData", &(controlerConfig.stallGuardControlData), controlerConfigXML);
  setValueIfFound("targetPosition", controlerConfig.targetPosition, controlerConfigXML);
  setValueIfFound("targetVelocity", controlerConfig.targetVelocity, controlerConfigXML);  

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

    CARD_CONFIG_ADD( coverDatagram );
    CARD_CONFIG_ADD( coverPositionAndLength );
    CARD_CONFIG_ADD( datagramHighWord );
    CARD_CONFIG_ADD( datagramLowWord );
    CARD_CONFIG_ADD( interfaceConfiguration );
    CARD_CONFIG_ADD( positionCompareInterruptData );
    CARD_CONFIG_ADD( positionCompareWord );
    CARD_CONFIG_ADD( stepperMotorGlobalParameters );

    for (unsigned int i=0; i < motorDriverCardConfig.motorControlerConfigurations.size(); ++i){
      MotorControlerConfig defaultControlerConfig;
      MotorControlerConfig motorControlerConfig = motorDriverCardConfig.motorControlerConfigurations[i];

      if (sparse && (motorControlerConfig == defaultControlerConfig )){
	continue;
      }

      pugi::xml_node controlerConfigXML = cardConfigXML.append_child("MotorControlerConfig");
      controlerConfigXML.append_attribute("motorID") = i;

      NodeFiller controlerConfigFiller(controlerConfigXML, sparse);
      
      CONTROLER_CONFIG_ADD( accelerationThresholdData );
      CONTROLER_CONFIG_ADD( actualPosition );
      CONTROLER_CONFIG_ADD( chopperControlData );
      CONTROLER_CONFIG_ADD( coolStepControlData );
      CONTROLER_CONFIG_ADD( decoderReadoutMode );
      CONTROLER_CONFIG_ADD( dividersAndMicroStepResolutionData );
      CONTROLER_CONFIG_ADD( driverConfigData );
      CONTROLER_CONFIG_ADD( driverControlData );
      CONTROLER_CONFIG_ADD( enabled );
      CONTROLER_CONFIG_ADD( interruptData );
      CONTROLER_CONFIG_ADD( maximumAccelleration );
      CONTROLER_CONFIG_ADD( maximumVelocity );
      CONTROLER_CONFIG_ADD( microStepCount );
      CONTROLER_CONFIG_ADD( minimumVelocity );
      CONTROLER_CONFIG_ADD( positionTolerance );
      CONTROLER_CONFIG_ADD( proportionalityFactorData );
      CONTROLER_CONFIG_ADD( referenceConfigAndRampModeData );
      CONTROLER_CONFIG_ADD( stallGuardControlData );
      CONTROLER_CONFIG_ADD( targetPosition );
      CONTROLER_CONFIG_ADD( targetVelocity );

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

  void MotorDriverCardConfigXML::NodeFiller::addRegister( std::string const & registerName, 
							  unsigned int value,
							  unsigned int defaultValue){
    if ( _writeAlways || (value != defaultValue) ){
      pugi::xml_node registerNode = _node.append_child("Register");
      registerNode.append_attribute("name") = registerName.c_str();
      registerNode.append_attribute("value") = to_hex_string(value).c_str();
    }
  }

  void MotorDriverCardConfigXML::NodeFiller::addRegister( std::string const & registerName, 
							  TMC429InputWord const & value,
							  TMC429InputWord const & defaultValue){
    addRegister( registerName, value.getDATA(), defaultValue.getDATA() );
  }

  void MotorDriverCardConfigXML::NodeFiller::addRegister( std::string const & registerName, 
							  TMC260Word const & value,
							  TMC260Word const & defaultValue){
    addRegister( registerName, value.getPayloadData(), defaultValue.getPayloadData() );
  }

  void MotorDriverCardConfigXML::NodeFiller::addRegister( std::string const & registerName, 
							  bool value,
							  bool defaultValue){
    if ( _writeAlways || (value != defaultValue) ){
      pugi::xml_node registerNode = _node.append_child("Register");
      registerNode.append_attribute("name") = registerName.c_str();
      registerNode.append_attribute("value") = (value?"true":"false");
    }
  }

  std::string MotorDriverCardConfigXML::NodeFiller::to_hex_string(unsigned int value){
    std::stringstream s;
    s << "0x" << std::uppercase << std::hex << value;
    return s.str();
  }

}// namespace mtca4u

