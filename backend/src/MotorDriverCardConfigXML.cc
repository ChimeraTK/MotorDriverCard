#include "MotorDriverCardConfigXML.h"

#include "ChimeraTK/Exception.h"
#include "schema.h"
#include <libxml++/libxml++.h>
#include <libxml/parser.h>

#include <charconv>
#include <iostream>

namespace detail {
  class NodeFiller {
   public:
    NodeFiller(xmlpp::Node* node, bool sparse);
    void addParameter(std::string const& parameterName, unsigned int value, unsigned int defaultValue,
        std::string const& tagName = "Register");
    void addParameter(
        std::string const& parameterName, int value, int defaultValue, std::string const& tagName = "Register");
    void addParameter(std::string const& registerName, mtca4u::TMC429InputWord const& value,
        mtca4u::TMC429InputWord const& defaultValue);
    void addParameter(
        std::string const& registerName, mtca4u::TMC260Word const& value, mtca4u::TMC260Word const& defaultValue);
    void addParameter(
        std::string const& parameterName, bool value, bool defaultValue, std::string const& tagName = "Register");

   private:
    xmlpp::Node* _node;
    bool _writeAlways;

    std::string toHexString(unsigned int value);
  };
} // namespace detail

detail::NodeFiller::NodeFiller(xmlpp::Node* node, bool sparse) : _node(node), _writeAlways(!sparse) {}

void detail::NodeFiller::addParameter(
    std::string const& parameterName, unsigned int value, unsigned int defaultValue, std::string const& tagName) {
  if(_writeAlways || (value != defaultValue)) {
    auto parameterNode = _node->add_child(tagName);
    parameterNode->set_attribute("name", parameterName);

    std::string valueString;
    if(tagName == "Register") {
      valueString = toHexString(value);
    }
    else {
      valueString = std::to_string(value);
    }
    parameterNode->set_attribute("value", valueString);
  }
}

void detail::NodeFiller::addParameter(
    std::string const& parameterName, int value, int defaultValue, std::string const& tagName) {
  if(_writeAlways || (value != defaultValue)) {
    auto* parameterNode = _node->add_child(tagName);
    parameterNode->set_attribute("name", parameterName);
    // register content of signed is always written as decimal
    parameterNode->set_attribute("value", std::to_string(value));
  }
}

void detail::NodeFiller::addParameter(std::string const& registerName, mtca4u::TMC429InputWord const& value,
    mtca4u::TMC429InputWord const& defaultValue) {
  addParameter(registerName, value.getDATA(), defaultValue.getDATA());
}

void detail::NodeFiller::addParameter(
    std::string const& registerName, mtca4u::TMC260Word const& value, mtca4u::TMC260Word const& defaultValue) {
  addParameter(registerName, value.getPayloadData(), defaultValue.getPayloadData());
}

void detail::NodeFiller::addParameter(
    std::string const& parameterName, bool value, bool defaultValue, std::string const& tagName) {
  if(_writeAlways || (value != defaultValue)) {
    auto* parameterNode = _node->add_child(tagName);
    parameterNode->set_attribute("name", parameterName);
    parameterNode->set_attribute("value", (value ? "true" : "false"));
  }
}

std::string detail::NodeFiller::toHexString(unsigned int value) {
  std::stringstream s;
  s << "0x" << std::uppercase << std::hex << value;
  return s.str();
}

namespace mtca4u {

  template<typename T>
  static void getFromXml(
      const std::string& parameterName, T& value, const xmlpp::Node* parent, const std::string& tagName = "Register") {
    std::string stringValue{};
    getFromXml(parameterName, stringValue, parent, tagName);
    if(stringValue.empty()) {
      return;
    }

    auto base = 10;
    if(stringValue.find("0x") == 0 || stringValue.find("0X") == 0) {
      base = 16;
      if(stringValue.size() == 2) {
        throw ChimeraTK::logic_error("Invalid XML");
      }
    }

    if constexpr(std::is_base_of_v<TMC260Word, T>) {
      unsigned int val{};

      auto [_, ec] = std::from_chars(
          stringValue.data() + (base == 16 ? 2 : 0), stringValue.data() + stringValue.size(), val, base);
      if(ec != std::errc() || strlen(_) != 0) {
        throw ChimeraTK::logic_error("Invalid XML");
      }
      value.setPayloadData(val);

      return;
    }
    else if constexpr(std::is_base_of_v<TMC429InputWord, T>) {
      unsigned int val{};

      auto [_, ec] = std::from_chars(
          stringValue.data() + (base == 16 ? 2 : 0), stringValue.data() + stringValue.size(), val, base);
      if(ec != std::errc() || strlen(_) != 0) {
        throw ChimeraTK::logic_error("Invalid XML");
      }
      value.setDATA(val);

      return;
    }
    else if constexpr(std::is_same_v<T, bool>) {
      std::transform(stringValue.begin(), stringValue.end(), stringValue.begin(), ::tolower);
      std::istringstream is(stringValue);
      is >> std::boolalpha >> value;
    }
    else {
      auto [_, ec] = std::from_chars(
          stringValue.data() + (base == 16 ? 2 : 0), stringValue.data() + stringValue.size(), value, base);
      if(ec != std::errc()) {
        throw ChimeraTK::logic_error("Invalid XML");
      }
    }
  }

  template<>
  void getFromXml(
      const std::string& parameterName, std::string& value, const xmlpp::Node* parent, const std::string& tagName) {
    std::string xPath{"//" + tagName + "[@name=\"" + parameterName + "\"]"};
    auto nodes = parent->find(xPath);
    if(nodes.empty()) {
      return;
    }

    if(nodes.size() != 1) {
      return;
    }

    for(const auto* node : nodes) {
      const auto* element = dynamic_cast<const xmlpp::Element*>(node);
      value = element->get_attribute_value("value");
    }
  }

  MotorControlerConfig controllerConfigFromXml(const xmlpp::Node* rootNode, const std::string& controllerId) {
    MotorControlerConfig controlerConfig;
    auto xpathHelper = "MotorControlerConfig[@motorID=" + controllerId + "]/";

    getFromXml(
        "accelerationThresholdData", controlerConfig.accelerationThresholdData, rootNode, xpathHelper + "Register");
    getFromXml("chopperControlData", controlerConfig.chopperControlData, rootNode, xpathHelper + "Register");
    getFromXml("coolStepControlData", controlerConfig.coolStepControlData, rootNode, xpathHelper + "Register");
    getFromXml("decoderReadoutMode", controlerConfig.decoderReadoutMode, rootNode, xpathHelper + "Register");
    getFromXml("dividersAndMicroStepResolutionData", controlerConfig.dividersAndMicroStepResolutionData, rootNode,
        xpathHelper + "Register");
    getFromXml("driverConfigData", controlerConfig.driverConfigData, rootNode, xpathHelper + "Register");
    getFromXml("driverControlData", controlerConfig.driverControlData, rootNode, xpathHelper + "Register");
    getFromXml("enabled", controlerConfig.enabled, rootNode, xpathHelper + "Register");
    getFromXml("interruptData", controlerConfig.interruptData, rootNode, xpathHelper + "Register");
    getFromXml("maximumAcceleration", controlerConfig.maximumAcceleration, rootNode, xpathHelper + "Register");
    getFromXml("maximumVelocity", controlerConfig.maximumVelocity, rootNode, xpathHelper + "Register");
    getFromXml("microStepCount", controlerConfig.microStepCount, rootNode, xpathHelper + "Register");
    getFromXml("minimumVelocity", controlerConfig.minimumVelocity, rootNode, xpathHelper + "Register");
    getFromXml("positionTolerance", controlerConfig.positionTolerance, rootNode, xpathHelper + "Register");
    getFromXml(
        "proportionalityFactorData", controlerConfig.proportionalityFactorData, rootNode, xpathHelper + "Register");
    getFromXml("referenceConfigAndRampModeData", controlerConfig.referenceConfigAndRampModeData, rootNode,
        xpathHelper + "Register");
    getFromXml("stallGuardControlData", controlerConfig.stallGuardControlData, rootNode, xpathHelper + "Register");
    getFromXml("targetPosition", controlerConfig.targetPosition, rootNode, xpathHelper + "Register");
    getFromXml("targetVelocity", controlerConfig.targetVelocity, rootNode, xpathHelper + "Register");
    getFromXml("driverSpiWaitingTime", controlerConfig.driverSpiWaitingTime, rootNode, xpathHelper + "Parameter");

    const auto* xpath{R"(//Register[@name="maximumAccelleration"])"};
    auto nodes = rootNode->find(xpath);
    if(!nodes.empty()) {
      std::stringstream message;
      message << "Found old, invalid Register name maximumAccelleration. Please update your config file!";
      throw ChimeraTK::logic_error(message.str());
    }
    return controlerConfig;
  }
} // namespace mtca4u

#define CARD_CONFIG_ADD_REGISTER(VALUE)                                                                                \
  cardConfigFiller.addParameter(#VALUE, motorDriverCardConfig.VALUE, defaultCardConfig.VALUE)

#define CARD_CONFIG_ADD_PARAMETER(VALUE)                                                                               \
  cardConfigFiller.addParameter(#VALUE, motorDriverCardConfig.VALUE, defaultCardConfig.VALUE, "Parameter")

#define CONTROLER_CONFIG_ADD_REGISTER(VALUE)                                                                           \
  controlerConfigFiller.addParameter(#VALUE, motorControlerConfig.VALUE, defaultControlerConfig.VALUE)

#define CONTROLER_CONFIG_ADD_PARAMETER(VALUE)                                                                          \
  controlerConfigFiller.addParameter(#VALUE, motorControlerConfig.VALUE, defaultControlerConfig.VALUE, "Parameter")

namespace mtca4u {

  MotorDriverCardConfig MotorDriverCardConfigXML::read(std::string fileName) {
    xmlpp::DomParser parser;
    xmlpp::RelaxNGValidator validator;

    try {
      validator.parse_memory(RELAXNG_SCHEMA_STRING);
    }
    catch(xmlpp::exception& e) {
      throw ChimeraTK::logic_error("Invalid RNG schema. Must not happen: " + std::string{e.what()});
    }

    try {
      parser.parse_file(fileName);

      if(validator.get_schema() != nullptr) {
        validator.validate(parser.get_document());
      }
    }
    catch(xmlpp::validity_error& e) {
      std::stringstream message;
      message << "Failed to validate XML file, please check your config";
      throw ChimeraTK::logic_error(message.str());
    }
    catch(xmlpp::exception& e) {
      std::stringstream message;
      message << "Could not load XML file \"" << fileName << "\": " << e.what();
      throw ChimeraTK::logic_error(message.str());
    }

    // a config with default values
    MotorDriverCardConfig cardConfig;

    auto* rootNode = parser.get_document()->get_root_node();

    getFromXml("coverDatagram", cardConfig.coverDatagram, rootNode);
    getFromXml("coverPositionAndLength", cardConfig.coverPositionAndLength, rootNode);
    getFromXml("datagramHighWord", cardConfig.datagramHighWord, rootNode);
    getFromXml("datagramLowWord", cardConfig.datagramLowWord, rootNode);
    getFromXml("interfaceConfiguration", cardConfig.interfaceConfiguration, rootNode);
    getFromXml("positionCompareInterruptData", cardConfig.positionCompareInterruptData, rootNode);
    getFromXml("positionCompareWord", cardConfig.positionCompareWord, rootNode);
    getFromXml("stepperMotorGlobalParameters", cardConfig.stepperMotorGlobalParameters, rootNode);
    getFromXml("controlerSpiWaitingTime", cardConfig.controlerSpiWaitingTime, rootNode, "Parameter");

    std::string xPath{"//MotorControlerConfig"};
    auto nodes = rootNode->find(xPath);

    for(auto node : nodes) {
      auto element = dynamic_cast<const xmlpp::Element*>(node);
      auto idString = element->get_attribute_value("motorID");
      if(idString.empty()) {
        throw ChimeraTK::logic_error("Controller config is missing its ID");
      }

      size_t val{};
      auto [_, ec] = std::from_chars(idString.data(), idString.data() + idString.size(), val);
      if(ec != std::errc()) {
        throw ChimeraTK::logic_error("Unparseable motor id");
      }

      cardConfig.motorControlerConfigurations[val] = controllerConfigFromXml(node, idString);
    }

    return cardConfig;
  }

  void MotorDriverCardConfigXML::write(std::string fileName, MotorDriverCardConfig const& motorDriverCardConfig) {
    write(fileName, motorDriverCardConfig, false);
  }

  void MotorDriverCardConfigXML::writeSparse(std::string fileName, MotorDriverCardConfig const& motorDriverCardConfig) {
    write(fileName, motorDriverCardConfig, true);
  }

  void MotorDriverCardConfigXML::write(
      std::string fileName, MotorDriverCardConfig const& motorDriverCardConfig, bool sparse) {
    xmlpp::Document doc;
    auto rootNode = doc.create_root_node("MotorDriverCardConfig");

    MotorDriverCardConfig defaultCardConfig;

    detail::NodeFiller cardConfigFiller(rootNode, sparse);

    CARD_CONFIG_ADD_REGISTER(coverDatagram);
    CARD_CONFIG_ADD_REGISTER(coverPositionAndLength);
    CARD_CONFIG_ADD_REGISTER(datagramHighWord);
    CARD_CONFIG_ADD_REGISTER(datagramLowWord);
    CARD_CONFIG_ADD_REGISTER(interfaceConfiguration);
    CARD_CONFIG_ADD_REGISTER(positionCompareInterruptData);
    CARD_CONFIG_ADD_REGISTER(positionCompareWord);
    CARD_CONFIG_ADD_REGISTER(stepperMotorGlobalParameters);
    CARD_CONFIG_ADD_PARAMETER(controlerSpiWaitingTime);

    for(unsigned int i = 0; i < motorDriverCardConfig.motorControlerConfigurations.size(); ++i) {
      MotorControlerConfig defaultControlerConfig;
      MotorControlerConfig motorControlerConfig = motorDriverCardConfig.motorControlerConfigurations[i];

      if(sparse && (motorControlerConfig == defaultControlerConfig)) {
        continue;
      }

      auto controllerConfigNode = rootNode->add_child("MotorControlerConfig");
      controllerConfigNode->set_attribute("motorID", std::to_string(i));

      detail::NodeFiller controlerConfigFiller(controllerConfigNode, sparse);

      CONTROLER_CONFIG_ADD_REGISTER(accelerationThresholdData);
      // CONTROLER_CONFIG_ADD_REGISTER( actualPosition );
      CONTROLER_CONFIG_ADD_REGISTER(chopperControlData);
      CONTROLER_CONFIG_ADD_REGISTER(coolStepControlData);
      CONTROLER_CONFIG_ADD_REGISTER(decoderReadoutMode);
      CONTROLER_CONFIG_ADD_REGISTER(dividersAndMicroStepResolutionData);
      CONTROLER_CONFIG_ADD_REGISTER(driverConfigData);
      CONTROLER_CONFIG_ADD_REGISTER(driverControlData);
      CONTROLER_CONFIG_ADD_REGISTER(enabled);
      CONTROLER_CONFIG_ADD_REGISTER(interruptData);
      CONTROLER_CONFIG_ADD_REGISTER(maximumAcceleration);
      CONTROLER_CONFIG_ADD_REGISTER(maximumVelocity);
      CONTROLER_CONFIG_ADD_REGISTER(microStepCount);
      CONTROLER_CONFIG_ADD_REGISTER(minimumVelocity);
      CONTROLER_CONFIG_ADD_REGISTER(positionTolerance);
      CONTROLER_CONFIG_ADD_REGISTER(proportionalityFactorData);
      CONTROLER_CONFIG_ADD_REGISTER(referenceConfigAndRampModeData);
      CONTROLER_CONFIG_ADD_REGISTER(stallGuardControlData);
      CONTROLER_CONFIG_ADD_REGISTER(targetPosition);
      CONTROLER_CONFIG_ADD_REGISTER(targetVelocity);
      CONTROLER_CONFIG_ADD_PARAMETER(driverSpiWaitingTime);
    }

    try {
      doc.write_to_file_formatted(fileName, "UTF-8");
    }
    catch(xmlpp::exception& ex) {
      std::stringstream message;
      message << "Could not write XML file \"" << fileName << "\": " << ex.what();
      throw ChimeraTK::logic_error(message.str());
    }
  }
} // namespace mtca4u
