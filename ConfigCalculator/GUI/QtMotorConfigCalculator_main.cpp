// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "QtMotorConfigCalculator.h"

using namespace ChimeraTK::MotorDriver;

int main(int argc, char** argv) {
  QApplication app(argc, argv);

  QtMotorConfigCalculator qtMotorConfigCalculator;
  qtMotorConfigCalculator.setWindowTitle("QtMotorConfigCalculator");
  qtMotorConfigCalculator.setWindowIcon(QIcon(":/DESY_logo_nofade.png"));

  qtMotorConfigCalculator.show();

  return QApplication::exec();
}
