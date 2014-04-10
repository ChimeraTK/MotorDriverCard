#ifndef MTCA4U_MOTOR_REFERENCE_SWITCH_DATA
#define MTCA4U_MOTOR_REFERENCE_SWITCH_DATA

#include "MultiVariableWord.h"

namespace mtca4u{

  /** A class containing an unigned int which is 
   *  used as a bit field holding the status bits
   *  of both end switches.
   *
   *  @todo The word should also contain the information if the switches are enabled.
   */
  class MotorReferenceSwitchData : public MultiVariableWord{
  public:
    /// The default constructor initialises everyting to 0 (all switches inactive);
    MotorReferenceSwitchData(unsigned int dataWord =0 );
    /** The switch which is reached when moving in positive direction.
     *  \li 0 = inactive
     *  \li 1 = active
     */
    ADD_VARIABLE( PositiveSwitch, 0, 0 );
    /** The switch which is reached when moving in negative direction.
     *  \li 0 = inactive
     *  \li 1 = active
     */
    ADD_VARIABLE( NegativeSwitch, 1, 1 ); 

    /// A word containing the active flags for all (both) switches.
    ADD_VARIABLE( SwitchStatuses, 0, 1 );

    /** Operator to allow conversion to bool for convenient checking if any of the switches
     *  is active.
     *  Example: if (motor->getReferenceSwitchData()) { cout << "one or both end switches are active" << endl; }
     */
    operator bool() const;
    
  };

}//namespace mtca4u

#endif// MTCA4U_MOTOR_REFERENCE_SWITCH_DATA
