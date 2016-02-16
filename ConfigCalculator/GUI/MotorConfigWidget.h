#ifndef MOTOR_CONFIG_WIDGET_H
#define MOTOR_CONFIG_WIDGET_H

#include "ParametersCalculator.h"
#include "ConfigCalculator.h"
#include <QWidget>
#include "ui_MotorConfigWidget.h"
#include "ParametersPanel.h"

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

   /** Assign an expert panel which is updated with the newly calculated parameters
    */
   void setMotorExpertPanel(ParametersPanel *motorExpertPanel, QTabWidget *expertTabWidget, int tabIndex);

 private slots:
   void recalculateChipParameters();
   void setMotorExpertTabEnabled(bool enabled);
   
 private:
   ParametersCalculator::ChipParameters _chipParameters;
   void updateChipParameters();
   void updateMotorExpertPanel();
   ConfigCalculator::EndSwitchConfig getEndSwitchConfig();

   Ui::MotorConfigWidgetForm _motorConfigWidgetForm;

   // Only needed to update and enable/disable the expert settings
   ParametersPanel *_motorExpertPanel;
   QTabWidget *_expertTabWidget;
   int _tabIndex;

};

#endif// MOTOR_CONFIG_WIDGET_H
