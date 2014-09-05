#ifndef MOTOR_CONFIG_WIDGET_H
#define MOTOR_CONFIG_WIDGET_H

#include "ParametersCalculator.h"
#include "ConfigCalculator.h"
#include <QWidget>
#include "ui_MotorConfigWidget.h"

class MotorConfigWidget: public QWidget{
  Q_OBJECT

 public:
   MotorConfigWidget(QWidget * parent_ = NULL);
 
   mtca4u::MotorControlerConfig getConfig();
   bool motorIsEnabled();

   void setMotorEnabled(bool motorEnabled);

 private slots:
   void recalculateTMC429Parameters();
   
 private:
   ParametersCalculator::TMC429Parameters _tmc429Parameters;
   void updateChipParameters();
   ConfigCalculator::EndSwitchConfig getEndSwitchConfig();

   Ui::MotorConfigWidgetForm _motorConfigWidgetForm;
};

#endif// MOTOR_CONFIG_WIDGET_H
