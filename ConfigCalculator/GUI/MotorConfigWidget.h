#ifndef MOTOR_CONFIG_WIDGET_H
#define MOTOR_CONFIG_WIDGET_H

#include "ParametersCalculator.h"
#include "ConfigCalculator.h"
#include <QWidget>
#include "ui_MotorConfigWidget.h"

/** A widget that calculates chip parameters
 *  from physical parameters of the motor and
 *  displays conversion warnings.
 */
class MotorConfigWidget: public QWidget{
  Q_OBJECT

 public:
  /** The constructor has no special parameters, except for 
   *  the parent of the widget.
   */
   MotorConfigWidget(QWidget * parent_ = NULL);
 
   /** Get access to the config calculated from the chip parameters.
    */
   mtca4u::MotorControlerConfig getConfig();

   /** Check whether the motor is enabled. This allows
    *  to ignore the config and write sparse config files 
    *  if only the enables motor are taken into accout.
    */
   bool motorIsEnabled();

   /** Change the enabled status.
    */
   void setMotorEnabled(bool motorEnabled);

 private slots:
   void recalculateChipParameters();
   
 private:
   ParametersCalculator::ChipParameters _chipParameters;
   void updateChipParameters();
   ConfigCalculator::EndSwitchConfig getEndSwitchConfig();

   Ui::MotorConfigWidgetForm _motorConfigWidgetForm;
};

#endif// MOTOR_CONFIG_WIDGET_H
