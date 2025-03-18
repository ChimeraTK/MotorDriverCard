// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "HexSpinBox.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <stdint.h>
#include <string>

/** A widget which contains a label, input field and default field,
 *  plus convenience getter (and setter?).
 */
class ParameterWidget : public QWidget {
 public:
  ParameterWidget(QWidget* parent_, std::string const& parameterName, uint32_t defaultValue = 0xDEADBEEF);
  ParameterWidget(ParameterWidget const&) = delete;
  ParameterWidget& operator=(ParameterWidget const&) = delete;

  uint32_t getValue();
  void setValue(uint32_t value);

 protected:
  QHBoxLayout* horizontalLayout;
  QLabel* label;
  HexSpinBox* parameterSpinBox;
  QLabel* defaultValueLabel;
};
