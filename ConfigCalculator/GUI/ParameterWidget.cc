// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ParameterWidget.h"

ParameterWidget::ParameterWidget(QWidget* parent_, std::string const& parameterName, uint32_t defaultValue)
: QWidget(parent_) {
  // NOLINTBEGIN(cppcoreguidelines-owning-memory) the Qt objects are owned by their parent widget
  _horizontalLayout = new QHBoxLayout(this);

  _label = new QLabel(parameterName.c_str(), this);
  _horizontalLayout->addWidget(_label);

  _parameterSpinBox = new HexSpinBox(this);
  _parameterSpinBox->setRange(static_cast<int>(0x80000000U), 0x7FFFFFFF);
  _parameterSpinBox->setValue(static_cast<int>(defaultValue));
  _horizontalLayout->addWidget(_parameterSpinBox);

  _defaultValueLabel = new QLabel(QString("0x%1 ").arg(defaultValue, 0, 16), this);
  _defaultValueLabel->setFrameShape(QFrame::StyledPanel);
  _defaultValueLabel->setFrameShadow(QFrame::Sunken);
  _horizontalLayout->addWidget(_defaultValueLabel);
  // NOLINTEND(cppcoreguidelines-owning-memory)
}

uint32_t ParameterWidget::getValue() {
  return _parameterSpinBox->value();
}

void ParameterWidget::setValue(uint32_t value) {
  _parameterSpinBox->setValue(static_cast<int>(value));
}
