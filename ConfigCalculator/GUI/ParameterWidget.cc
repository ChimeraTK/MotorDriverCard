#include "ParameterWidget.h"

ParameterWidget::ParameterWidget(QWidget * parent_, std::string const & parameterName)
  : QWidget(parent_){
  horizontalLayout = new QHBoxLayout(this);

  label =  new QLabel(parameterName.c_str(), this);
  horizontalLayout->addWidget( label );

  parameterSpinBox = new QSpinBox(this);
  horizontalLayout->addWidget( parameterSpinBox );

  defaultValueLabel =  new QLabel("0xDEADBEEF", this);
  defaultValueLabel->setFrameShape(QFrame::StyledPanel);
  defaultValueLabel->setFrameShadow(QFrame::Sunken);
  horizontalLayout->addWidget( defaultValueLabel );
}

int ParameterWidget::getValue(){
  return parameterSpinBox->value();
}
