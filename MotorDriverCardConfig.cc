#include "MotorDriverCardConfig.h"

unsigned int const INTERFACE_CONFIGURATION_DEFAULT = 0x000122;
unsigned int const STEPPER_MOTOR_GLOBAL_PARAMETERS_DEFAULT = 0x010701;

namespace mtca4u{

  MotorDriverCardConfig::MotorDriverCardConfig()
    : coverDatagram(0),
      coverPositionAndLength(0),
      datagramHighWord(0),
      datagramLowWord(0),
      interfaceConfiguration(INTERFACE_CONFIGURATION_DEFAULT),
      positionCompareInterruptData(0),
      positionCompareWord(0),
      stepperMotorGlobalParameters(STEPPER_MOTOR_GLOBAL_PARAMETERS_DEFAULT)
  {
    // initialises the with the default constructors of the motor controler config
    motorControlerConfigurations.resize( dfmc_md22::N_MOTORS_MAX );
  }

  bool MotorDriverCardConfig::operator==(MotorDriverCardConfig const& right) const{
    if (coverDatagram != right.coverDatagram ){return false;}
    if (coverPositionAndLength != right.coverPositionAndLength ){return false;}
    if (datagramHighWord != right.datagramHighWord ){return false;}
    if (datagramLowWord	!= right.datagramLowWord ){return false;}
    if (interfaceConfiguration != right.interfaceConfiguration ){return false;}
    if (positionCompareInterruptData != right.positionCompareInterruptData ){return false;}
    if (positionCompareWord != right.positionCompareWord ){return false;}
    if (stepperMotorGlobalParameters != right.stepperMotorGlobalParameters ){return false;}

    for( size_t i = 0; i < motorControlerConfigurations.size(); ++i){
	if ( motorControlerConfigurations[i] !=  right.motorControlerConfigurations[i] ){return false;}
    }

    return true;
  }
    
    

}// namespace mtca4u
