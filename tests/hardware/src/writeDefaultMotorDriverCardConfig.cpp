#include "MotorDriverCardConfigXML.h"

#include <iostream>

using namespace mtca4u;

int main() {
  MotorDriverCardConfigXML::write("MotorDriverCardConfig_default.xml", MotorDriverCardConfig());
  return 0;
}
