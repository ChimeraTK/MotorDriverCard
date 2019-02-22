#include "ParametersPanel.h"
#include "ParameterWidget.h"

#include <iostream>

ParametersPanel::ParametersPanel(QWidget* parent_) : QWidget(parent_), parametersMap(), verticalLayout(NULL) {
  // We use an outer and an inner vertical layout.
  // The outer layout just contains the header, inner layout and a spacer.
  QVBoxLayout* outerVerticalLayout = new QVBoxLayout(this);

  // add a header line
  QHBoxLayout* header = new QHBoxLayout();
  // make the header three items, like the ParametersWidget
  header->addWidget(new QWidget(this)); // empty
  header->addWidget(new QLabel("Value"));
  header->addWidget(new QLabel("Default Value"));
  outerVerticalLayout->addLayout(header);

  verticalLayout = new QVBoxLayout();
  outerVerticalLayout->addLayout(verticalLayout);
  outerVerticalLayout->addStretch(-1); // -1 = add at the end
  QLabel* asteriskLabel = new QLabel("Fields marked with (*) are modified by the basic config calculation.", this);
  outerVerticalLayout->addWidget(asteriskLabel);
}

void ParametersPanel::addParameter(std::string const& parameterName,
    uint32_t defaultValue,
    std::string additionalText) {
  parametersMap[parameterName] = new ParameterWidget(this, parameterName + " " + additionalText, defaultValue);

  verticalLayout->addWidget(parametersMap[parameterName]);
}

uint32_t ParametersPanel::operator[](std::string const& parameterName) {
  std::map<std::string, ParameterWidget*>::const_iterator paramIter = parametersMap.find(parameterName);
  if(paramIter == parametersMap.end()) {
    return 0xDEADBEEF;
  }
  return paramIter->second->getValue();
}

void ParametersPanel::setParameter(std::string const& parameterName, uint32_t value) {
  std::map<std::string, ParameterWidget*>::const_iterator paramIter = parametersMap.find(parameterName);
  if(paramIter == parametersMap.end()) {
    return;
  }
  paramIter->second->setValue(value);
}
