#include "QtMotorConfigCalculator.h"
#include "DFMC_MD22Constants.h"

#include "MotorDriverCardConfigXML.h"
#include "getParameters.h"
#include "populatePanels.h"
#include <QFileDialog>

QtMotorConfigCalculator::QtMotorConfigCalculator(QWidget *parent_)
    : QWidget(parent_), verticalLayout(NULL), motorTabWidget(NULL),
      writeButton(NULL), expertTabWidget(NULL), motorConfigWidgets(),
      motorExpertWidgets(), cardExpertWidget(NULL) {
  verticalLayout = new QVBoxLayout(this);

  QTabWidget *mainTabWidget = new QTabWidget(this);
  motorTabWidget = new QTabWidget(this);

  // first add the standard motor config widgets for all motors
  for (size_t motorID = 0; motorID < mtca4u::dfmc_md22::N_MOTORS_MAX;
       ++motorID) {
    motorConfigWidgets.push_back(new MotorConfigWidget(this));
    QString tabLabel("Motor ");
    tabLabel += QString::number(motorID);

    motorTabWidget->addTab(motorConfigWidgets[motorID], tabLabel);

    // disable all motors but the first one
    if (motorID > 0) {
      motorConfigWidgets[motorID]->setMotorEnabled(false);
    }
  }

  // next: create the expert tab
  expertTabWidget = new QTabWidget(this);

  // add the card expert widget
  cardExpertWidget = new ParametersPanel(this);
  expertTabWidget->addTab(cardExpertWidget, "MD22 Card");
  populateDriverCardExpertPanel(cardExpertWidget);

  // add the motor expert widgets
  for (size_t motorID = 0; motorID < mtca4u::dfmc_md22::N_MOTORS_MAX;
       ++motorID) {

    // Define the expert widget
    ParametersPanel *thisMotorExpertWidget = new ParametersPanel(this);
    motorExpertWidgets.push_back(thisMotorExpertWidget);
    populateMotorExpertPanel(thisMotorExpertWidget);
    QString tabLabel("Motor ");
    tabLabel += QString::number(motorID);

    // Define a scroll area to hold the expert widget
    QScrollArea *thisScrollArea = new QScrollArea;
    thisScrollArea->setWidget(thisMotorExpertWidget);

    // Add scroll area with expert widget to the tab
    expertTabWidget->addTab(thisScrollArea, tabLabel);

    // make the expert widgets known to the motor config widgets
    motorConfigWidgets[motorID]->setMotorExpertPanel(
        thisMotorExpertWidget, expertTabWidget, motorID + 1);
  }

  mainTabWidget->addTab(motorTabWidget, "Basic Configuration");
  mainTabWidget->addTab(expertTabWidget, "Expert Settings");
  verticalLayout->addWidget(mainTabWidget);

  writeButton = new QPushButton("Write XML file", this);
  verticalLayout->addWidget(writeButton);

  connect(writeButton, SIGNAL(clicked()), this, SLOT(write()));

  this->resize(620, 480);
}

void QtMotorConfigCalculator::write() {
  QString fileName = QFileDialog::getSaveFileName(
      NULL, "Save config file", QString(), tr("XML files (*.xml)"));

  if (fileName.isEmpty()) {
    return;
  }

  mtca4u::MotorDriverCardConfig cardConfig =
      getMotorDriverCardParameters(cardExpertWidget);
  for (size_t motorID = 0; motorID < mtca4u::dfmc_md22::N_MOTORS_MAX;
       ++motorID) {
    // only write motors which are activated for sparse xml files
    if (motorConfigWidgets[motorID]->motorIsEnabled()) {
      //	      cardConfig.motorControlerConfigurations[motorID] =
      //		motorConfigWidgets[motorID]->getConfig();
      cardConfig.motorControlerConfigurations[motorID] =
          getMotorParameters(motorExpertWidgets[motorID]);
    }
  }

  mtca4u::MotorDriverCardConfigXML::writeSparse(fileName.toStdString(),
                                                cardConfig);
}
