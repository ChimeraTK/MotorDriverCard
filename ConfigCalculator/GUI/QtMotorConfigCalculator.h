#ifndef QT_MOTOR_CONFIG_CALCULATOR_H
#define QT_MOTOR_CONFIG_CALCULATOR_H

#include "MotorConfigWidget.h"
#include "ParametersPanel.h"

#include <QPushButton>
#include <QScrollArea>
#include <QTabWidget>

class QComboBox;

namespace mtca4u {
  struct MotorDriverCardConfig;
} // namespace mtca4u

/** A widget containing two MotorConfigWidget in a TabWidget,
 *  and buttons which load a config from and write it to an XML file.
 */
class QtMotorConfigCalculator : public QWidget {
  Q_OBJECT

 public:
  /** The constructor expects no parameters except for the
   *  parent widget.
   */
  QtMotorConfigCalculator(QWidget* parent_ = NULL);
  QtMotorConfigCalculator(QtMotorConfigCalculator const&) = delete;
  QtMotorConfigCalculator& operator=(QtMotorConfigCalculator const&) = delete;

 private:
  QVBoxLayout* verticalLayout;
  QTabWidget* motorTabWidget;
  QPushButton* loadButton;
  QPushButton* writeButton;
  QComboBox* endSwitchConfigurationCombo;

  QTabWidget* expertTabWidget;
  std::vector<MotorConfigWidget*> motorConfigWidgets;
  std::vector<ParametersPanel*> motorExpertWidgets;
  ParametersPanel* cardExpertWidget;

  void applyConfig(mtca4u::MotorDriverCardConfig const& config);

 private slots:
  // Get the file name via dialogue and perform the loading/writing.
  void load();
  void write();
  void updateCardExpertPanel(int index);
};

#endif // QT_MOTOR_CONFIG_CALCULATOR_H
