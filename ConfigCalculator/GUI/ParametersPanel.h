// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <map>
#include <QHBoxLayout>
#include <QWidget>
#include <stdint.h>
#include <string>

class ParameterWidget;

class ParametersPanel : public QWidget {
  //  Q_OBJECT

 public:
  ParametersPanel(QWidget* parent_);
  ParametersPanel(ParametersPanel const&) = delete;
  ParametersPanel& operator=(ParametersPanel const&) = delete;

  // fixme:remove default for the default
  void addParameter(
      std::string const& parameterName, uint32_t defaultValue = 0xDEADBEEF, std::string additionalText = std::string());
  // convenient way to retrieve the parameter value
  uint32_t operator[](std::string const& parameterName);
  // set a parameter value (actually contained in the spin box)
  void setParameter(std::string const& parameterName, uint32_t value);

 protected:
  std::map<std::string, ParameterWidget*> parametersMap;
  QVBoxLayout* verticalLayout;
};
