#include "ParameterWidget.h"

ParameterWidget::ParameterWidget(QWidget * parent_, std::string const & parameterName, uint32_t defaultValue)
  : QWidget(parent_), horizontalLayout(NULL), label(NULL), parameterSpinBox(NULL), defaultValueLabel(NULL){
  horizontalLayout = new QHBoxLayout(this);

  label =  new QLabel(parameterName.c_str(), this);
  horizontalLayout->addWidget( label );

  parameterSpinBox = new HexSpinBox(this);
  parameterSpinBox->setRange(0x0000000,0xFFFFFFFFL);
  parameterSpinBox->setValue(defaultValue);
  horizontalLayout->addWidget( parameterSpinBox );

  defaultValueLabel =  new QLabel(QString("0x%1 ").arg(defaultValue,0,16), this);
  defaultValueLabel->setFrameShape(QFrame::StyledPanel);
  defaultValueLabel->setFrameShadow(QFrame::Sunken);
  horizontalLayout->addWidget( defaultValueLabel );
}

uint32_t ParameterWidget::getValue(){
  return parameterSpinBox->value();
}

void ParameterWidget::setValue(uint32_t value){
  parameterSpinBox->setValue(value);
}
