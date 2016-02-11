#include "ParametersPanel.h"
#include "ParameterWidget.h"

#include <iostream>

ParametersPanel::ParametersPanel(QWidget * parent_) : QWidget(parent_){
  // We use an outer and an inner vertical layout.
  // The outer layout just contains the header, inner layout and a spacer.
  QVBoxLayout * outerVerticalLayout = new QVBoxLayout(this);

  // add a header line
  QHBoxLayout * header = new QHBoxLayout();
  // make the header three items, like the ParametersWidget
  header->addWidget(new QWidget(this) ); // empty
  header->addWidget(new QLabel("Value") );
  header->addWidget(new QLabel("Default Value") );
  outerVerticalLayout->addLayout(header);

  verticalLayout = new QVBoxLayout();
  outerVerticalLayout->addLayout(verticalLayout);
  outerVerticalLayout->addStretch(-1); // -1 = add at the end
}

void ParametersPanel::addParameter(std::string const & parameterName, std::string additionalText){
  parametersMap[parameterName] = new ParameterWidget(this, parameterName +" "+ additionalText);
  verticalLayout->addWidget(parametersMap[parameterName]);
}

int ParametersPanel::operator[](std::string const & parameterName){
  std::map< std::string, ParameterWidget *>::const_iterator paramIter = parametersMap.find(parameterName);
  if (paramIter == parametersMap.end()){
    return 0xDEADBEEF;
  }
  return paramIter->second->getValue();
}
