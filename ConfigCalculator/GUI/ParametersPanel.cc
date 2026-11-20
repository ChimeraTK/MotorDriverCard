// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ParametersPanel.h"

#include "ParameterWidget.h"

#include <iostream>

ParametersPanel::ParametersPanel(QWidget* parent_) : QWidget(parent_), parametersMap() {
  // We use an outer and an inner vertical layout.
  // The outer layout just contains the header, inner layout and a spacer.
  auto* outerVerticalLayout = new QVBoxLayout(this);

  // add a header line
  auto* header = new QHBoxLayout();
  // make the header three items, like the ParametersWidget
  header->addWidget(new QWidget(this)); // empty
  header->addWidget(new QLabel("Value"));
  header->addWidget(new QLabel("Default Value"));
  outerVerticalLayout->addLayout(header);

  verticalLayout = new QVBoxLayout();
  outerVerticalLayout->addLayout(verticalLayout);
  outerVerticalLayout->addStretch(-1); // -1 = add at the end
  auto* asteriskLabel = new QLabel("Fields marked with (*) are modified by the basic config calculation.", this);
  outerVerticalLayout->addWidget(asteriskLabel);
}

void ParametersPanel::addParameter(
    std::string const& parameterName, uint32_t defaultValue, std::string additionalText) {
  parametersMap[parameterName] = new ParameterWidget(this, parameterName + " " + additionalText, defaultValue);

  verticalLayout->addWidget(parametersMap[parameterName]);
}

uint32_t ParametersPanel::operator[](std::string const& parameterName) {
  auto paramIter = parametersMap.find(parameterName);
  if(paramIter == parametersMap.end()) {
    return 0xDEADBEEF;
  }
  return paramIter->second->getValue();
}

void ParametersPanel::setParameter(std::string const& parameterName, uint32_t value) {
  auto paramIter = parametersMap.find(parameterName);
  if(paramIter == parametersMap.end()) {
    return;
  }
  paramIter->second->setValue(value);
}
