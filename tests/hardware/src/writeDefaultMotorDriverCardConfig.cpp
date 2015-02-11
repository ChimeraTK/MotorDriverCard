#include <iostream>

#include "MotorDriverCardConfigXML.h"

using namespace mtca4u;

int main( )
{
  MotorDriverCardConfigXML::write("MotorDriverCardConfig_default.xml", MotorDriverCardConfig());
  return 0;
}

