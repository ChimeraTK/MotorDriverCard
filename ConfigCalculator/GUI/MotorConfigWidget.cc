#include "MotorConfigWidget.h"
#include <cmath>
#include <stdexcept>
#include <list>
#include <string>

#include <QMessageBox>

MotorConfigWidget::MotorConfigWidget( QWidget * parent_ )
  : QWidget( parent_ ),
    // the chipParameters do not have a default constructor
    _chipParameters( 6,  //pulseDiv_
		       11, // rampDiv_
		       1466, // aMax_
		       1398, // vMax_
		       6, // pDiv_
		       174, // pMul_
		       4, // controllerMicroStepValue_
		       4, // driverMicroStepValue_
		       3, // currentScale_
		       std::list<std::string>())
{
  _motorConfigWidgetForm.setupUi(this);

  // recalculate all parameters whenever an input changes
  connect(_motorConfigWidgetForm.iMax, SIGNAL(valueChanged(double)),
	  this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.nStepsPerTurn, SIGNAL(valueChanged(int)),
	  this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.maxRPM, SIGNAL(valueChanged(int)),
	  this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.timeToVMax, SIGNAL(valueChanged(double)),
	  this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.microsteps, SIGNAL(currentIndexChanged(int)),
	  this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.systemClock, SIGNAL(valueChanged(int)),
	  this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.positiveSwitchCheckBox, SIGNAL(stateChanged(int)),
	  this, SLOT(recalculateChipParameters()));

  connect(_motorConfigWidgetForm.negativeSwitchCheckBox, SIGNAL(stateChanged(int)),
	  this, SLOT(recalculateChipParameters()));
}

mtca4u::MotorControlerConfig MotorConfigWidget::getConfig(){
  return ConfigCalculator::calculateConfig( _chipParameters, 
					    getEndSwitchConfig() );
}

bool MotorConfigWidget::motorIsEnabled(){
  return _motorConfigWidgetForm.motorEnabledCheckBox->isChecked();
}

void MotorConfigWidget::recalculateChipParameters(){
  unsigned int microsteps = pow(2, _motorConfigWidgetForm.microsteps->currentIndex() );

  ParametersCalculator::PhysicalParameters
    physicalParameters( _motorConfigWidgetForm.systemClock->value(),
			microsteps,
			_motorConfigWidgetForm.nStepsPerTurn->value(),
			_motorConfigWidgetForm.maxRPM->value(),
			_motorConfigWidgetForm.timeToVMax->value(),
			_motorConfigWidgetForm.iMax->value() );
  try{ 
    _chipParameters = ParametersCalculator::calculateParameters( physicalParameters );
  }catch(std::exception & e){
     QString errorMessage("Error, could not calculate parameters!\n\n");
     errorMessage += QString("An exception was thrown:\n") + e.what();
    QMessageBox::critical(NULL, "Parameter Calculation Error", errorMessage );
  }

  updateChipParameters();
}

void MotorConfigWidget::updateChipParameters(){
  _motorConfigWidgetForm.pulseDivDisplay->setText( 
    QString::number( _chipParameters.pulseDiv ) );

  _motorConfigWidgetForm.rampDivDisplay->setText( 
    QString::number( _chipParameters.rampDiv ) );

  _motorConfigWidgetForm.vMaxDisplay->setText( 
    QString::number( _chipParameters.vMax ) );

  _motorConfigWidgetForm.aMaxDisplay->setText( 
    QString::number( _chipParameters.aMax ) );

  _motorConfigWidgetForm.pMulDisplay->setText( 
    QString::number( _chipParameters.pMul ) );

  _motorConfigWidgetForm.pDivDisplay->setText( 
    QString::number( _chipParameters.pDiv ) );

  _motorConfigWidgetForm.controllerMicrostepValueDisplay->setText( 
    QString::number( _chipParameters.controllerMicroStepValue ));

  _motorConfigWidgetForm.driverMicrostepValueDisplay->setText( 
    QString::number( _chipParameters.driverMicroStepValue ));

  _motorConfigWidgetForm.currentScaleDisplay->setText( 
    QString::number( _chipParameters.currentScale ) );

  QString warningText;
  for (std::list<std::string>::const_iterator warningIter = 
	 _chipParameters.warnings.begin();
       warningIter != _chipParameters.warnings.end();
       ++warningIter){
    warningText += QString(warningIter->c_str()) + "\n";
  }

  _motorConfigWidgetForm.warningsBrowser->setText( warningText );
}

void MotorConfigWidget::setMotorEnabled(bool motorEnabled){
  _motorConfigWidgetForm.motorEnabledCheckBox->setChecked(motorEnabled);
}

ConfigCalculator::EndSwitchConfig MotorConfigWidget::getEndSwitchConfig(){
  if(_motorConfigWidgetForm.positiveSwitchCheckBox->isChecked()){
    if(_motorConfigWidgetForm.negativeSwitchCheckBox->isChecked()){
      return ConfigCalculator::USE_BOTH;
    }else{
      return ConfigCalculator::IGNORE_NEGATIVE;
    }
 }else{
    if(_motorConfigWidgetForm.negativeSwitchCheckBox->isChecked()){
      return ConfigCalculator::IGNORE_POSITIVE;
    }else{
      return ConfigCalculator::IGNORE_BOTH;
    }
  }
}
