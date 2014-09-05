#include <QtGui>
#include "QtConfigCalculator.h"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  QtConfigCalculator qtConfigCalculator;
  qtConfigCalculator.setWindowTitle("QtConfigCalculator");
  //motorConfigWidget.setWindowIcon(  QIcon(":/DESY_logo_nofade.png") );

  qtConfigCalculator.show();

  return app.exec();
}
