#ifndef _PARAMETER_WIDGET_H_
#define _PARAMETER_WIDGET_H_

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include "HexSpinBox.h"

#include <string>

/** A widget which contains a label, input field and default field,
 *  plus convenience getter (and setter?).
 */
class ParameterWidget : public QWidget{
 public:
  ParameterWidget(QWidget * parent_, std::string const & parameterName);
  int getValue();
 protected:
  QHBoxLayout *horizontalLayout;
  QLabel * label;
  HexSpinBox * parameterSpinBox;
  QLabel * defaultValueLabel;
};

#endif // _PARAMETER_WIDGET_H_
