#include "QtMotorConfigCalculator.h"
#include "DFMC_MD22Constants.h"

#include <QFileDialog>
#include "MotorDriverCardConfigXML.h"
#include "populateMotorExpertPanel.h"

QtMotorConfigCalculator::QtMotorConfigCalculator(QWidget * parent_)
  : QWidget(parent_){
  verticalLayout = new QVBoxLayout(this);
  
  motorTabWidget = new QTabWidget(this);
  
  // first add the standard motor config widgets for all motors
  for (size_t motorID = 0; motorID < mtca4u::dfmc_md22::N_MOTORS_MAX; ++motorID){
    motorConfigWidgets.push_back(new MotorConfigWidget(this));
    QString tabLabel("Motor ");
    tabLabel += QString::number(motorID);

    motorTabWidget->addTab(motorConfigWidgets[motorID], tabLabel);

    // disable all motors but the first one
    if (motorID > 0){
      motorConfigWidgets[motorID]->setMotorEnabled(false);
    }
  }
  // next: create the expert tab
  expertTabWidget = new QTabWidget(this);

  // add the card expert widget
  cardExpertWidget = new ParametersPanel(this);
  expertTabWidget->addTab(cardExpertWidget,"MD22 Card");

  // add the motor expert widgets
  for (size_t motorID = 0; motorID < mtca4u::dfmc_md22::N_MOTORS_MAX; ++motorID){
    ParametersPanel * thisMotorExpertWidget = new ParametersPanel(this);
    motorExpertWidgets.push_back(thisMotorExpertWidget);
    populateMotorExpertPanel(thisMotorExpertWidget);
    QString tabLabel("Motor ");
    tabLabel += QString::number(motorID);

    expertTabWidget->addTab(thisMotorExpertWidget, tabLabel);
  }
  
  motorTabWidget->addTab(expertTabWidget,"Expert Settings");
  verticalLayout->addWidget(motorTabWidget);
  
  writeButton = new QPushButton("Write XML file",this);
  verticalLayout->addWidget(writeButton);

  connect(writeButton, SIGNAL( clicked() ),
	  this, SLOT( write() ) );

  this->resize(620, 480);
}

void QtMotorConfigCalculator::write(){
  QString fileName = QFileDialog::getSaveFileName( NULL, "Save config file",
						   QString(), tr("XML files (*.xml)" ));
  
  if (fileName.isEmpty()){
    return;
  }

  mtca4u::MotorDriverCardConfig cardConfig;
  for (size_t motorID = 0; motorID < mtca4u::dfmc_md22::N_MOTORS_MAX; ++motorID){
    // only write motors which are activated for sparse xml files
    if(  motorConfigWidgets[motorID]->motorIsEnabled() ){
      cardConfig.motorControlerConfigurations[motorID] = 
	motorConfigWidgets[motorID]->getConfig();
    }
  }
  
  mtca4u::MotorDriverCardConfigXML::writeSparse(fileName.toStdString(), cardConfig);

}
