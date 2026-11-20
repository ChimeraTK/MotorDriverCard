// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "QtMotorConfigCalculator.h"

#include "DFMC_MD22Constants.h"
#include "getParameters.h"
#include "MotorDriverCardConfigXML.h"
#include "populatePanels.h"

#include <QFileDialog>
using namespace ChimeraTK::MotorDriver;

QtMotorConfigCalculator::QtMotorConfigCalculator(QWidget* parent_) : QWidget(parent_) {
  // NOLINTBEGIN(cppcoreguidelines-owning-memory) the Qt objects are owned by their parent widget
  _verticalLayout = new QVBoxLayout(this);

  auto* mainTabWidget = new QTabWidget(this);
  _motorTabWidget = new QTabWidget(this);

  // first add the standard motor config widgets for all motors
  for(size_t motorID = 0; motorID < ChimeraTK::MotorDriver::dfmc_md22::N_MOTORS_MAX; ++motorID) {
    _motorConfigWidgets.push_back(new MotorConfigWidget(this));
    QString tabLabel("Motor ");
    tabLabel += QString::number(motorID);

    _motorTabWidget->addTab(_motorConfigWidgets[motorID], tabLabel);

    // disable all motors but the first one
    if(motorID > 0) {
      _motorConfigWidgets[motorID]->setMotorEnabled(false);
    }
  }

  auto* _horizontalLayout = new QHBoxLayout(this);
  auto* endSwitchConfiguration = new QLabel(this);
  endSwitchConfiguration->setText("All end switches are active when");
  _horizontalLayout->addWidget(endSwitchConfiguration);
  auto* endSwitchConfigurationCombo = new QComboBox(this);
  endSwitchConfigurationCombo->insertItem(0, "High (default)");
  endSwitchConfigurationCombo->insertItem(1, "Low");
  _horizontalLayout->addWidget(endSwitchConfigurationCombo);
  auto* motorTabVerticalLayout = new QVBoxLayout(this);
  motorTabVerticalLayout->addLayout(_horizontalLayout);
  motorTabVerticalLayout->addWidget(_motorTabWidget);

  connect(endSwitchConfigurationCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCardExpertPanel(int)));

  auto* basicConfigurationWidget = new QWidget(this);
  basicConfigurationWidget->setLayout(motorTabVerticalLayout);

  // next: create the expert tab
  _expertTabWidget = new QTabWidget(this);

  // add the card expert widget
  _cardExpertWidget = new ParametersPanel(this);
  _expertTabWidget->addTab(_cardExpertWidget, "MD22 Card");
  populateDriverCardExpertPanel(_cardExpertWidget);

  // add the motor expert widgets
  for(size_t motorID = 0; motorID < ChimeraTK::MotorDriver::dfmc_md22::N_MOTORS_MAX; ++motorID) {
    // Define the expert widget
    auto* thisMotorExpertWidget = new ParametersPanel(this);
    _motorExpertWidgets.push_back(thisMotorExpertWidget);
    populateMotorExpertPanel(thisMotorExpertWidget);
    QString tabLabel("Motor ");
    tabLabel += QString::number(motorID);

    // Define a scroll area to hold the expert widget
    auto* thisScrollArea = new QScrollArea;
    thisScrollArea->setWidget(thisMotorExpertWidget);

    // Add scroll area with expert widget to the tab
    _expertTabWidget->addTab(thisScrollArea, tabLabel);

    // make the expert widgets known to the motor config widgets
    _motorConfigWidgets[motorID]->setMotorExpertPanel(
        thisMotorExpertWidget, _expertTabWidget, static_cast<int>(motorID) + 1);
  }

  mainTabWidget->addTab(basicConfigurationWidget, "Basic Configuration");
  mainTabWidget->addTab(_expertTabWidget, "Expert Settings");
  _verticalLayout->addWidget(mainTabWidget);

  _writeButton = new QPushButton("Write XML file", this);
  _verticalLayout->addWidget(_writeButton);

  connect(_writeButton, SIGNAL(clicked()), this, SLOT(write()));

  this->resize(620, 480);
  // NOLINTEND(cppcoreguidelines-owning-memory)
}

void QtMotorConfigCalculator::write() {
  QString fileName = QFileDialog::getSaveFileName(nullptr, "Save config file", QString(), tr("XML files (*.xml)"));

  if(fileName.isEmpty()) {
    return;
  }

  ChimeraTK::MotorDriver::MotorDriverCardConfig cardConfig = getMotorDriverCardParameters(_cardExpertWidget);
  for(size_t motorID = 0; motorID < ChimeraTK::MotorDriver::dfmc_md22::N_MOTORS_MAX; ++motorID) {
    // only write motors which are activated for sparse xml files
    if(_motorConfigWidgets[motorID]->motorIsEnabled()) {
      //	      cardConfig.motorControllerConfigurations[motorID] =
      //		_motorConfigWidgets[motorID]->getConfig();
      cardConfig.motorControllerConfigurations[motorID] = getMotorParameters(_motorExpertWidgets[motorID]);
    }
  }

  ChimeraTK::MotorDriver::MotorDriverCardConfigXML::writeSparse(fileName.toStdString(), cardConfig);
}

void QtMotorConfigCalculator::updateCardExpertPanel(int index) {
  ChimeraTK::MotorDriver::MotorDriverCardConfig cardConfig = getMotorDriverCardParameters(_cardExpertWidget);
  cardConfig.interfaceConfiguration.setInv_ref(index);
  _cardExpertWidget->setParameter("interfaceConfiguration", cardConfig.interfaceConfiguration.getDATA());
}
