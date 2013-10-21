#ifndef MTCA4U_MOTOR_DRIVER_CARD_EXPERT_H
#define MTCA4U_MOTOR_DRIVER_CARD_EXPERT_H

#include "TMC429Register.h"
#include "MotorDriverCard.h"

namespace mtca4u
{
  /**
   * Extendet interface for experts of the DFMC-MD22 motor driver card.
   */
  class MotorDriverCardExpert: public MotorDriverCard{
  public:
    // the helper classes
    class CoverPositionAndLength: public TMC429Register
    {
      public:
        ADD_VARIABLE(CoverLength, 0, 4);
	ADD_VARIABLE(CoverPosition, 8, 13);
	ADD_VARIABLE(CoverWaiting, 23, 23);
	CoverPositionAndLength(unsigned int data){
	  setSmda(0x3);
	  setIDX_JDX(0x2);
	  setData(data);
	}
    };

    /** For covenience this class allows access to the polarity bits individually
     *  and in addition by using the "Polarities" field, so some bits can be 
     *  accessed by more than one function.
     */
    class StepperMotorGlobalParameters : public TMC429Register
    {
      public:
        ADD_VARIABLE(LastSteperMotorDriver, 0, 1);
        ADD_VARIABLE(Polarities, 2, 6);
	// we break the naming convention here to stick with the data sheet
        ADD_VARIABLE(Polarity_nSCS_S, 2, 2);
        ADD_VARIABLE(Polarity_SCK_S,  3, 3);
        ADD_VARIABLE(Polarity_PH_AB,  4, 4);
        ADD_VARIABLE(Polarity_FD,     5, 5);
        ADD_VARIABLE(Polarity_DAC_AB, 6, 6);

        ADD_VARIABLE(CsCommonIndividual, 7, 7);
        ADD_VARIABLE(Clk2_div, 8, 15);
        ADD_VARIABLE(ContinuousUpdate, 16, 16);
        ADD_VARIABLE(RefMux, 20, 20);
        ADD_VARIABLE(Mot1r, 21, 21);	
	StepperMotorGlobalParameters(unsigned int data){
	  setSmda(0x3);
	  setIDX_JDX(0xF);
	  setData(data);
	}
    };

    class InterfaceConfiguration : public TMC429Register
    {
      public: 
        ADD_VARIABLE(Inv_ref, 0, 0);
        ADD_VARIABLE(Sdo_int, 1, 1);
        ADD_VARIABLE(StepHalf, 2, 2);
        ADD_VARIABLE(Inv_stp, 3, 3);
        ADD_VARIABLE(Inv_dir, 4, 4);
        ADD_VARIABLE(Es_sd, 5, 5);
        ADD_VARIABLE(Pos_comp_sel_0, 6, 6);
        ADD_VARIABLE(Pos_comp_sel_1, 7, 7);
        ADD_VARIABLE(En_refr, 8, 8);
	/// Constructor to define the correct address.
	InterfaceConfiguration(unsigned int data){
	  setSmda(0x3);
	  setIDX_JDX(0x4);
	  setData(data);
	}
    }

    /// Expert function 
    virtual void setDatagramLowWord(unsigned int datagramLowWord) = 0;
    /// Expert function 
    virtual void setDatagramHighWord(unsigned int datagramHighWord) = 0;
    /// Expert function 
    virtual void setCoverPositionAndLength(
		    CoverPositionAndLength const & coverPositionAndLength) = 0;
    /// Expert function 
    virtual void setCoverDatagram(unsigned int coverDatagram) = 0;
    /// Expert function 
    virtual void setStepperMotorGlobalParametersRegister(
		    StepperMotorGlobalParameters const & stepperMotorGlobalParameters) = 0;

    /// Expert function: Set the Interface configuration register of the
    /// TCM429 chip
    virtual void setInterfaceConfiguration(
		    InterfaceConfiguration const & interfaceConfiguration) = 0;

     /// FIXME: expert or not?
    virtual void setReferenceSwitchRegister(ReferenceSwitchData referenceSwitchData) = 0;

   /// Expert function: Power down the TCM429 chip
    void powerDown() = 0;
 
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_EXPERT_H
