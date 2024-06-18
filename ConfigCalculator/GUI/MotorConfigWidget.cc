#include "MotorConfigWidget.h"

#include "getParameters.h"

#include <cmath>
#include <list>
#include <QMessageBox>
#include <stdexcept>
#include <string>

MotorConfigWidget::MotorConfigWidget(QWidget* parent_)
: QWidget(parent_),
  // the chipParameters do not have a default constructor
  _chipParameters(6, // pulseDiv_
      11,            // rampDiv_
      1466,          // aMax_
      1398,          // vMax_
      6,             // pDiv_
      174,           // pMul_
      4,             // controllerMicroStepValue_
      4,             // driverMicroStepValue_
      3,             // currentScale_
      std::list<std::string>()),
  _motorConfigWidgetForm(), _motorExpertPanel(NULL), _expertTabWidget(NULL), _tabIndex(0) {
  _motorConfigWidgetForm.setupUi(this);

  // recalculate all parameters whenever an input changes
  connect(_motorConfigWidgetForm.iMax, SIGNAL(valueChanged(double)), this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.nStepsPerTurn, SIGNAL(valueChanged(int)), this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.maxRPM, SIGNAL(valueChanged(int)), this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.timeToVMax, SIGNAL(valueChanged(double)), this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.microsteps, SIGNAL(currentIndexChanged(int)), this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.systemClock, SIGNAL(valueChanged(int)), this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.positiveEndSwitchComboBox, SIGNAL(currentIndexChanged(int)), this,
      SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.negativeEndSwitchComboBox, SIGNAL(currentIndexChanged(int)), this,
      SLOT(recalculateChipParameters()));

  connect(
      _motorConfigWidgetForm.motorEnabledCheckBox, SIGNAL(toggled(bool)), this, SLOT(setMotorExpertTabEnabled(bool)));
}

mtca4u::MotorControlerConfig MotorConfigWidget::getConfig() {
  return ConfigCalculator::calculateConfig(_chipParameters, getEndSwitchConfig());
}

bool MotorConfigWidget::motorIsEnabled() {
  return _motorConfigWidgetForm.motorEnabledCheckBox->isChecked();
}

void MotorConfigWidget::recalculateChipParameters() {
  unsigned int microsteps = pow(2, _motorConfigWidgetForm.microsteps->currentIndex());

  ParametersCalculator::PhysicalParameters physicalParameters(_motorConfigWidgetForm.systemClock->value(), microsteps,
      _motorConfigWidgetForm.nStepsPerTurn->value(), _motorConfigWidgetForm.maxRPM->value(),
      _motorConfigWidgetForm.timeToVMax->value(), _motorConfigWidgetForm.iMax->value());
  try {
    _chipParameters = ParametersCalculator::calculateParameters(physicalParameters);
  }
  catch(std::exception& e) {
    QString errorMessage("Error, could not calculate parameters!\n\n");
    errorMessage += QString("An exception was thrown:\n") + e.what();
    QMessageBox::critical(NULL, "Parameter Calculation Error", errorMessage);
  }

  updateChipParameters();
  updateMotorExpertPanel();
}

void MotorConfigWidget::updateChipParameters() {
  _motorConfigWidgetForm.pulseDivDisplay->setText(QString::number(_chipParameters.pulseDiv));

  _motorConfigWidgetForm.rampDivDisplay->setText(QString::number(_chipParameters.rampDiv));

  _motorConfigWidgetForm.vMaxDisplay->setText(QString::number(_chipParameters.vMax));

  _motorConfigWidgetForm.aMaxDisplay->setText(QString::number(_chipParameters.aMax));

  _motorConfigWidgetForm.pMulDisplay->setText(QString::number(_chipParameters.pMul));

  _motorConfigWidgetForm.pDivDisplay->setText(QString::number(_chipParameters.pDiv));

  _motorConfigWidgetForm.controllerMicrostepValueDisplay->setText(
      QString::number(_chipParameters.controllerMicroStepValue));

  _motorConfigWidgetForm.driverMicrostepValueDisplay->setText(QString::number(_chipParameters.driverMicroStepValue));

  _motorConfigWidgetForm.currentScaleDisplay->setText(QString::number(_chipParameters.currentScale));

  QString warningText;
  for(std::list<std::string>::const_iterator warningIter = _chipParameters.warnings.begin();
      warningIter != _chipParameters.warnings.end(); ++warningIter) {
    warningText += QString(warningIter->c_str()) + "\n";
  }

  _motorConfigWidgetForm.warningsBrowser->setText(warningText);
}

void MotorConfigWidget::updateMotorExpertPanel() {
  if(!_motorExpertPanel) {
    return;
  }

  mtca4u::MotorControlerConfig calculatedConfig = this->getConfig();
  mtca4u::MotorControlerConfig expertConfig = getMotorParameters(_motorExpertPanel);

  // only set the parameters which have actually been calculated (unfortunately
  // a duplication of the information in the config calculator) Don't always set
  // the DATA part, but only the bits which have been modified. Leave the rest
  // to the expert.
  expertConfig.driverControlData.setMicroStepResolution(calculatedConfig.driverControlData.getMicroStepResolution());
  _motorExpertPanel->setParameter("driverControlData", expertConfig.driverControlData.getPayloadData());
  expertConfig.stallGuardControlData.setCurrentScale(calculatedConfig.stallGuardControlData.getCurrentScale());
  _motorExpertPanel->setParameter("stallGuardControlData", expertConfig.stallGuardControlData.getPayloadData());

  // the other parameters go directly through. We have to replace the whole word
  _motorExpertPanel->setParameter(
      "referenceConfigAndRampModeData", calculatedConfig.referenceConfigAndRampModeData.getDATA());
  _motorExpertPanel->setParameter("proportionalityFactorData", calculatedConfig.proportionalityFactorData.getDATA());
  _motorExpertPanel->setParameter(
      "dividersAndMicroStepResolutionData", calculatedConfig.dividersAndMicroStepResolutionData.getDATA());
  _motorExpertPanel->setParameter("maximumVelocity", calculatedConfig.maximumVelocity);
  _motorExpertPanel->setParameter("maximumAcceleration", calculatedConfig.maximumAcceleration);
}

void MotorConfigWidget::setMotorEnabled(bool motorEnabled) {
  _motorConfigWidgetForm.motorEnabledCheckBox->setChecked(motorEnabled);
}

ConfigCalculator::EndSwitchConfig MotorConfigWidget::getEndSwitchConfig() {
  auto negativeEnabled = _motorConfigWidgetForm.negativeEndSwitchComboBox->currentIndex() == 0;
  auto positiveEnabled = _motorConfigWidgetForm.positiveEndSwitchComboBox->currentIndex() == 0;

  if(positiveEnabled) {
    if(negativeEnabled) {
      return ConfigCalculator::USE_BOTH;
    }

    return ConfigCalculator::IGNORE_NEGATIVE;
  }

  if(negativeEnabled) {
    return ConfigCalculator::IGNORE_POSITIVE;
  }

  return ConfigCalculator::IGNORE_BOTH;
}

void MotorConfigWidget::setMotorExpertPanel(
    ParametersPanel* motorExpertPanel, QTabWidget* expertTabWidget, int tabIndex) {
  _motorExpertPanel = motorExpertPanel;
  _expertTabWidget = expertTabWidget;
  _tabIndex = tabIndex;
  recalculateChipParameters(); // which also updates the expert panel
  setMotorExpertTabEnabled(_motorConfigWidgetForm.motorEnabledCheckBox->isChecked());
}

void MotorConfigWidget::setMotorExpertTabEnabled(bool enabled) {
  // we have to check the pointer, so we cannot just connect the signal directly
  // to the panel
  if(_expertTabWidget) {
    _expertTabWidget->setTabEnabled(_tabIndex, enabled);
  }
}
