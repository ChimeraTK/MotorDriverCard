#include "QtMotorConfigCalculator.h"

#include "DFMC_MD22Constants.h"
#include "getParameters.h"
#include "MotorDriverCardConfigXML.h"
#include "populatePanels.h"

#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>

QtMotorConfigCalculator::QtMotorConfigCalculator(QWidget* parent_)
: QWidget(parent_), verticalLayout(NULL), motorTabWidget(NULL), loadButton(NULL), writeButton(NULL),
  endSwitchConfigurationCombo(NULL), expertTabWidget(NULL), motorConfigWidgets(), motorExpertWidgets(),
  cardExpertWidget(NULL) {
  verticalLayout = new QVBoxLayout(this);

  QTabWidget* mainTabWidget = new QTabWidget(this);
  motorTabWidget = new QTabWidget(this);

  // first add the standard motor config widgets for all motors
  for(size_t motorID = 0; motorID < mtca4u::dfmc_md22::N_MOTORS_MAX; ++motorID) {
    motorConfigWidgets.push_back(new MotorConfigWidget(this));
    QString tabLabel("Motor ");
    tabLabel += QString::number(motorID);

    motorTabWidget->addTab(motorConfigWidgets[motorID], tabLabel);

    // disable all motors but the first one
    if(motorID > 0) {
      motorConfigWidgets[motorID]->setMotorEnabled(false);
    }
  }

  auto horizontalLayout = new QHBoxLayout(this);
  auto endSwitchConfiguration = new QLabel(this);
  endSwitchConfiguration->setText("All end switches are active when");
  horizontalLayout->addWidget(endSwitchConfiguration);
  endSwitchConfigurationCombo = new QComboBox(this);
  endSwitchConfigurationCombo->insertItem(0, "High (default)");
  endSwitchConfigurationCombo->insertItem(1, "Low");
  horizontalLayout->addWidget(endSwitchConfigurationCombo);
  auto motorTabVerticalLayout = new QVBoxLayout(this);
  motorTabVerticalLayout->addLayout(horizontalLayout);
  motorTabVerticalLayout->addWidget(motorTabWidget);

  connect(endSwitchConfigurationCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCardExpertPanel(int)));

  auto basicConfigurationWidget = new QWidget(this);
  basicConfigurationWidget->setLayout(motorTabVerticalLayout);

  // next: create the expert tab
  expertTabWidget = new QTabWidget(this);

  // add the card expert widget
  cardExpertWidget = new ParametersPanel(this);
  expertTabWidget->addTab(cardExpertWidget, "MD22 Card");
  populateDriverCardExpertPanel(cardExpertWidget);

  // add the motor expert widgets
  for(size_t motorID = 0; motorID < mtca4u::dfmc_md22::N_MOTORS_MAX; ++motorID) {
    // Define the expert widget
    ParametersPanel* thisMotorExpertWidget = new ParametersPanel(this);
    motorExpertWidgets.push_back(thisMotorExpertWidget);
    populateMotorExpertPanel(thisMotorExpertWidget);
    QString tabLabel("Motor ");
    tabLabel += QString::number(motorID);

    // Define a scroll area to hold the expert widget
    QScrollArea* thisScrollArea = new QScrollArea;
    thisScrollArea->setWidget(thisMotorExpertWidget);

    // Add scroll area with expert widget to the tab
    expertTabWidget->addTab(thisScrollArea, tabLabel);

    // make the expert widgets known to the motor config widgets
    motorConfigWidgets[motorID]->setMotorExpertPanel(thisMotorExpertWidget, expertTabWidget, motorID + 1);
  }

  mainTabWidget->addTab(basicConfigurationWidget, "Basic Configuration");
  mainTabWidget->addTab(expertTabWidget, "Expert Settings");
  verticalLayout->addWidget(mainTabWidget);

  auto buttonLayout = new QHBoxLayout(this);
  loadButton = new QPushButton("Load XML file", this);
  buttonLayout->addWidget(loadButton);
  writeButton = new QPushButton("Write XML file", this);
  buttonLayout->addWidget(writeButton);
  verticalLayout->addLayout(buttonLayout);

  connect(loadButton, SIGNAL(clicked()), this, SLOT(load()));
  connect(writeButton, SIGNAL(clicked()), this, SLOT(write()));

  this->resize(620, 480);
}

void QtMotorConfigCalculator::load() {
  QString fileName = QFileDialog::getOpenFileName(NULL, "Load config file", QString(), tr("XML files (*.xml)"));

  if(fileName.isEmpty()) {
    return;
  }

  mtca4u::MotorDriverCardConfig cardConfig;
  try {
    cardConfig = mtca4u::MotorDriverCardConfigXML::read(fileName.toStdString());
  }
  catch(std::exception const& e) {
    QMessageBox::critical(NULL, "Could not load config file", QString(e.what()));
    return;
  }

  applyConfig(cardConfig);
}

void QtMotorConfigCalculator::applyConfig(mtca4u::MotorDriverCardConfig const& cardConfig) {
  setMotorDriverCardParameters(cardExpertWidget, cardConfig);
  endSwitchConfigurationCombo->setCurrentIndex(static_cast<int>(cardConfig.interfaceConfiguration.getInv_ref()));

  mtca4u::MotorControlerConfig const defaultMotorConfig;
  for(size_t motorID = 0; motorID < mtca4u::dfmc_md22::N_MOTORS_MAX; ++motorID) {
    auto const& motorConfig = cardConfig.motorControlerConfigurations[motorID];
    setMotorParameters(motorExpertWidgets[motorID], motorConfig);

    // Sparse files omit registers that keep their default. A written motor
    // without an explicit enable flag therefore still has to be activated.
    motorConfigWidgets[motorID]->setMotorEnabled(motorConfig.enabled || motorConfig != defaultMotorConfig);
  }
}

void QtMotorConfigCalculator::write() {
  QString fileName = QFileDialog::getSaveFileName(NULL, "Save config file", QString(), tr("XML files (*.xml)"));

  if(fileName.isEmpty()) {
    return;
  }

  mtca4u::MotorDriverCardConfig cardConfig = getMotorDriverCardParameters(cardExpertWidget);
  for(size_t motorID = 0; motorID < mtca4u::dfmc_md22::N_MOTORS_MAX; ++motorID) {
    // only write motors which are activated for sparse xml files
    if(motorConfigWidgets[motorID]->motorIsEnabled()) {
      //	      cardConfig.motorControlerConfigurations[motorID] =
      //		motorConfigWidgets[motorID]->getConfig();
      cardConfig.motorControlerConfigurations[motorID] = getMotorParameters(motorExpertWidgets[motorID]);
    }
  }

  mtca4u::MotorDriverCardConfigXML::writeSparse(fileName.toStdString(), cardConfig);
}

void QtMotorConfigCalculator::updateCardExpertPanel(int index) {
  mtca4u::MotorDriverCardConfig cardConfig = getMotorDriverCardParameters(cardExpertWidget);
  cardConfig.interfaceConfiguration.setInv_ref(index);
  cardExpertWidget->setParameter("interfaceConfiguration", cardConfig.interfaceConfiguration.getDATA());
}
