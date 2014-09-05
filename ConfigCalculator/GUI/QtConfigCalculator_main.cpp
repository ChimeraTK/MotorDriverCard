#include <QtGui>
#include "MotorConfigWidget.h"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  MotorConfigWidget motorConfigWidget;
  motorConfigWidget.setWindowTitle("QtConfigCalculator");
  motorConfigWidget.setWindowIcon(  QIcon(":/DESY_logo_nofade.png") );

  motorConfigWidget.show();

  return app.exec();
}
