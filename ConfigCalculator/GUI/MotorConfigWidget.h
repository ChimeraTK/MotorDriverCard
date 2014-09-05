#ifndef MOTOR_CONFIG_WIDGET_H
#define MOTOR_CONFIG_WIDGET_H

#include "ParametersCalculator.h"
#include <QWidget>
#include "ui_MotorConfigWidget.h"

class MotorConfigWidget: public QWidget{
  Q_OBJECT

 public:
   MotorConfigWidget(QWidget * parent_ = NULL);

   ParametersCalculator::TMC429Parameters getTMC429Parameters();
   bool motorIsActive();

 private slots:
   void recalculateTMC429Parameters();
   
 private:
   ParametersCalculator::TMC429Parameters _tmc429Parameters;
   void updateChipParameters();

   Ui::MotorConfigWidgetForm _motorConfigWidgetForm;
};

#endif// MOTOR_CONFIG_WIDGET_H
