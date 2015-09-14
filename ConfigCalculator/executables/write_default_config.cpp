#include <MotorDriverCardConfigXML.h>
using namespace mtca4u;

int main(){
  MotorDriverCardConfigXML().write( "MotorDriverCardDefaultConfig.xml",
				    MotorDriverCardConfig() );

  return 0;
}
