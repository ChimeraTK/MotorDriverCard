#ifndef MTCA4U_MOTOR_DRIVER_CARD_H
#define MTCA4U_MOTOR_DRIVER_CARD_H

#include "TMC429Word.h"
#include "DFMC-MD22_Constants.h"

namespace mtca4u
{
  class MotorControler;
  class PowerMonitor;

  /**
   * A class to access the DFMC-MD22 motor driver card, which provides two 
   * MotorControlers.
   */
  class MotorDriverCard{
  public:
    /** Nested helper class to describe the reference switch register.
     */
    class ReferenceSwitchData : public TMC429Word
    {
      public: 
        ADD_VARIABLE(Right1, 0, 0);
        ADD_VARIABLE(Left1, 1, 1);
        ADD_VARIABLE(Right2, 2, 2);
        ADD_VARIABLE(Left2, 3, 3);
        ADD_VARIABLE(Right3, 4, 4);
        ADD_VARIABLE(Left3, 5, 6);
	/// Constructor to define the correct address.
	ReferenceSwitchData(unsigned int data = 0){
	  setSMDA(SMDA_COMMON);
	  setIDX_JDX(JDX_REFERENCE_SWITCH);
	  setDATA(data);	
	}
    };

    /** Get acces to one of the two motor controlers on this board. 
     *  Valid IDs are 0 and 1.
     *  Throws a MotorDriverException if the ID is invalid.
     */
    virtual MotorControler & getMotorControler(unsigned int motorControlerID) = 0;
    
    virtual ReferenceSwitchData getReferenceSwitchRegister() = 0;

    virtual unsigned int getControlerChipVersion() = 0;
 
    /// Get a reference to the power monitor.
    virtual PowerMonitor & getPowerMonitor() = 0;
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_H
