#ifndef _PARAMETER_WIDGET_H_
#define _PARAMETER_WIDGET_H_

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include "HexSpinBox.h"

#include <string>
#include <stdint.h>

/** A widget which contains a label, input field and default field,
 *  plus convenience getter (and setter?).
 */
class ParameterWidget : public QWidget{
 public:
  ParameterWidget(QWidget * parent_, std::string const & parameterName, uint32_t defaultValue = 0xDEADBEEF);
  ParameterWidget(ParameterWidget const &)=delete;
  ParameterWidget & operator=(ParameterWidget const &)=delete;
  

  uint32_t getValue();
  void setValue(uint32_t value);
 protected:
  QHBoxLayout *horizontalLayout;
  QLabel * label;
  HexSpinBox * parameterSpinBox;
  QLabel * defaultValueLabel;
};

#endif // _PARAMETER_WIDGET_H_
