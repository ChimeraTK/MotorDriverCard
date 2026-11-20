// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ParameterWidget.h"

ParameterWidget::ParameterWidget(QWidget* parent_, std::string const& parameterName, uint32_t defaultValue)
: QWidget(parent_) {
  // NOLINTBEGIN(cppcoreguidelines-owning-memory) the Qt objects are owned by their parent widget
  horizontalLayout = new QHBoxLayout(this);

  label = new QLabel(parameterName.c_str(), this);
  horizontalLayout->addWidget(label);

  parameterSpinBox = new HexSpinBox(this);
  parameterSpinBox->setRange(static_cast<int>(0x80000000U), 0x7FFFFFFF);
  parameterSpinBox->setValue(static_cast<int>(defaultValue));
  horizontalLayout->addWidget(parameterSpinBox);

  defaultValueLabel = new QLabel(QString("0x%1 ").arg(defaultValue, 0, 16), this);
  defaultValueLabel->setFrameShape(QFrame::StyledPanel);
  defaultValueLabel->setFrameShadow(QFrame::Sunken);
  horizontalLayout->addWidget(defaultValueLabel);
  // NOLINTEND(cppcoreguidelines-owning-memory)
}

uint32_t ParameterWidget::getValue() {
  return parameterSpinBox->value();
}

void ParameterWidget::setValue(uint32_t value) {
  parameterSpinBox->setValue(static_cast<int>(value));
}
