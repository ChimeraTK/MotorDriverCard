#include "QtMotorConfigCalculator.h"
#include <QtGui>

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  QtMotorConfigCalculator qtMotorConfigCalculator;
  qtMotorConfigCalculator.setWindowTitle("QtMotorConfigCalculator");
  qtMotorConfigCalculator.setWindowIcon(QIcon(":/DESY_logo_nofade.png"));

  qtMotorConfigCalculator.show();

  return app.exec();
}
