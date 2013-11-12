#ifndef MTCA4U_TMC429WORDS_H
#define MTCA4U_TMC429WORDS_H

#include "MultiVariableWord.h"
#include "DFMC_MD22Constants.h"

// All bits from the ADD_VARIABLE are only in this file. Their correctness is tested 
// with a unit test (second, independent implementation of the same information).
// I intentionally decided to directly write the numbers in the code and not define
// constants for each start and stop bit. These constants whould only tempt users
// to use them in their code and start bit-fiddeling themselves. In addition, having a 
// header with so many constants would not make it more  "storing them all in one place" than
// having the numbers here, within their context and two numbers, which belong together, per line.

namespace mtca4u{
  
  /** Structure of a generic single value register of the TMC429 chip.
   *  It implements the full maximum 24 bit data payload. For
   *  shorter data words use an appropriate mask or range check
   *  before setting the values to the data field.
   *  
   *  Classes inheriting from this generic class can provide 
   *  more fine grained access to the data field if this is 
   *  interpreted as multiple variables.
   *
   *  For interpretation of the variables consult the TMC429 data sheet,
   *  chapter 8, page 17ff, especially table 8-2.
   *
   *  We intentionally break the naming convention here and go for
   *  data sheed syntax. This also makes it easier to distiguisch 
   *  MultiVariableWord::getDataWord() (the full 32 bit content)
   *  from TMC429InputWord::getDATA() (the data payload sub word as described in the 
   *  data sheet).
   */
  class TMC429InputWord: public MultiVariableWord {
    public:
      TMC429InputWord(unsigned int dataWord = 0): MultiVariableWord(dataWord){}
      ADD_VARIABLE(RRS, 31, 31 );
      ADD_VARIABLE(SMDA, 29, 30 );
      ADD_VARIABLE(IDX_JDX, 25, 28 );
      ADD_VARIABLE(ADDRESS, 25, 30 ); // combination if SMDA and IDX/JDX
      ADD_VARIABLE(RW , 24, 24);
      ADD_VARIABLE(DATA, 0, 23);
  };

  /** The readback register contains status information in the higher bits,
   *  which contain the SPI address in case of an input word.
   *  See TMC429 data sheet, page 15, Table 7-4
   */
  class TMC429OutputWord: public MultiVariableWord {
    public:
      TMC429OutputWord(unsigned int dataWord = 0): MultiVariableWord(dataWord){}
      ADD_VARIABLE(STATUS_BITS, 24 ,31);
    
      ADD_VARIABLE(INT, 31, 31 );
      ADD_VARIABLE(CDGW, 30, 30 );

      ADD_VARIABLE(SM3, 28, 29 );
      ADD_VARIABLE(RS3, 29, 29 );
      ADD_VARIABLE(xEQt3, 28, 28 );

      ADD_VARIABLE(SM2, 26, 27 );
      ADD_VARIABLE(RS2, 27, 27 );
      ADD_VARIABLE(xEQt2, 26, 26 );

      ADD_VARIABLE(SM1, 24, 25 );
      ADD_VARIABLE(RS1, 25, 25 );
      ADD_VARIABLE(xEQt1, 24, 24 );

      ADD_VARIABLE(DATA, 0, 23);
  };
 
  class ReferenceSwitchData : public TMC429InputWord {
    public: 
      ADD_VARIABLE(Right1, 0, 0);
      ADD_VARIABLE(Left1, 1, 1);
      ADD_VARIABLE(Right2, 2, 2);
      ADD_VARIABLE(Left2, 3, 3);
      ADD_VARIABLE(Right3, 4, 4);
      ADD_VARIABLE(Left3, 5, 5);

      /// Constructor to define the correct address.
      ReferenceSwitchData(unsigned int data = 0){
	setSMDA(tmc429::SMDA_COMMON);
	setIDX_JDX(tmc429::JDX_REFERENCE_SWITCH);
	setDATA(data);	
      }
  };

    // the helper classes
    class CoverPositionAndLength: public TMC429InputWord {
      public:
        ADD_VARIABLE(CoverLength, 0, 4);
	ADD_VARIABLE(CoverPosition, 8, 13);
	ADD_VARIABLE(CoverWaiting, 23, 23);

	CoverPositionAndLength(unsigned int data = 0){
	  setSMDA(tmc429::SMDA_COMMON);
	  setIDX_JDX(tmc429::JDX_COVER_POSITION_AND_LENGTH);
	  setDATA(data);
	}
    };

    /** For covenience this class allows access to the polarity bits individually
     *  and in addition by using the "Polarities" field, so some bits can be 
     *  accessed by more than one function.
     */
    class StepperMotorGlobalParameters : public TMC429InputWord {
      public:
        ADD_VARIABLE(LastStepperMotorDriver, 0, 1);
        ADD_VARIABLE(Polarities, 2, 6);
	// we break the naming convention here to stick with the data sheet
        ADD_VARIABLE(Polarity_nSCS_S, 2, 2);
        ADD_VARIABLE(Polarity_SCK_S,  3, 3);
        ADD_VARIABLE(Polarity_PH_AB,  4, 4);
        ADD_VARIABLE(Polarity_FD_AB,  5, 5);
        ADD_VARIABLE(Polarity_DAC_AB, 6, 6);

        ADD_VARIABLE(CsCommonIndividual, 7, 7);
        ADD_VARIABLE(Clk2_div, 8, 15);
        ADD_VARIABLE(ContinuousUpdate, 16, 16);
        ADD_VARIABLE(RefMux, 20, 20);
        ADD_VARIABLE(Mot1r, 21, 21);
	
	StepperMotorGlobalParameters(unsigned int data = 0){
	  setSMDA(tmc429::SMDA_COMMON);
	  setIDX_JDX(tmc429::JDX_STEPPER_MOTOR_GLOBAL_PARAMETERS);
	  setDATA(data);
	}
    };

    /** See TMC429 data sheet, page 33, table 10-2.
     */
    class InterfaceConfiguration : public TMC429InputWord {
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
	InterfaceConfiguration(unsigned int data = 0){
	  setSMDA(tmc429::SMDA_COMMON);
	  setIDX_JDX(tmc429::JDX_INTERFACE_CONFIGURATION);
	  setDATA(data);
	}
    };

    class PositionCompareInterruptData : public TMC429InputWord {
      public:
        ADD_VARIABLE( InterruptFlag, 0, 0 );
        ADD_VARIABLE( InterruptMask, 8, 8 );
	PositionCompareInterruptData(unsigned int data = 0){
	  setSMDA(tmc429::SMDA_COMMON);
	  setIDX_JDX(tmc429::JDX_POSITION_COMPARE_INTERRUPT);
	  setDATA(data);      
	}
    };

    /** A helper class which contains the variables of the
     *  acceleration treshold register.
     */
    class AccelerationThresholdData : public TMC429InputWord {
    public:
      ADD_VARIABLE(CurrentScalingBelowThreshold, 16, 18);
      ADD_VARIABLE(CurrentScalingAboveThreshold, 20, 22);
      ADD_VARIABLE(CurrentScalingAtRest, 12, 14);
      ADD_VARIABLE(AccelerationThreshold, 0, 10);

      /// Constructor which initialises the IDX correctly
      AccelerationThresholdData(){
	setIDX_JDX( tmc429::IDX_ACCELERATION_THRESHOLD );
      }
      
    };

    /** A helper class to represent contents of the 
     *  proportionality factor register.
     *  Note: Bit 15, which is always 1, is not represented.
     */
    class ProportionalityFactorData : public TMC429InputWord {
    public:
      ADD_VARIABLE(DivisionParameter, 0, 3);
      ADD_VARIABLE(MultiplicationParameter, 8, 14);

      /// Constructor to initialise the IDX correctly
      ProportionalityFactorData(){
	setIDX_JDX( tmc429::IDX_PROPORTIONALITY_FACTORS );	
      }
    };
    
    class ReferenceConfigAndRampModeData : public TMC429InputWord {
    public:
      ADD_VARIABLE(RampMode, 0, 1);
      ADD_VARIABLE(ReferenceConfig, 8, 11);
      ADD_VARIABLE(DISABLE_STOP_L, 8, 8);
      ADD_VARIABLE(DISABLE_STOP_R, 9, 9);
      ADD_VARIABLE(SOFT_STOP, 10, 10);
      ADD_VARIABLE(REF_RnL, 11, 11);
      ADD_VARIABLE(LatchedPosition, 16, 16);

      /// Constructor to initialise the IDX correctly
      ReferenceConfigAndRampModeData(){
	setIDX_JDX( tmc429::IDX_REFERENCE_CONFIG_AND_RAMP_MODE );
      }
    };

    class InterruptData : public TMC429InputWord {
    public:
      ADD_VARIABLE(InterruptFlags, 0, 7);
      ADD_VARIABLE(INT_POS_END, 0, 0);
      ADD_VARIABLE(INT_REF_WRONG, 1, 1);
      ADD_VARIABLE(INT_REF_MISS, 2, 2);
      ADD_VARIABLE(INT_STOP, 3, 3);
      ADD_VARIABLE(INT_STOP_LEFT_LOW, 4, 4);
      ADD_VARIABLE(INT_STOP_RIGHT_LOW, 5, 5);
      ADD_VARIABLE(INT_STOP_LEFT_HIGH, 6, 6);
      ADD_VARIABLE(INT_STOP_RIGHT_HIGH, 7, 7);

      ADD_VARIABLE(MaskFlags, 8, 15);
      ADD_VARIABLE(MASK_POS_END, 8, 8);
      ADD_VARIABLE(MASK_REF_WRONG, 9, 9);
      ADD_VARIABLE(MASK_REF_MISS, 10, 10);
      ADD_VARIABLE(MASK_STOP, 11, 11);
      ADD_VARIABLE(MASK_STOP_LEFT_LOW, 12, 12);
      ADD_VARIABLE(MASK_STOP_RIGHT_LOW, 13, 13);
      ADD_VARIABLE(MASK_STOP_LEFT_HIGH, 14, 14);
      ADD_VARIABLE(MASK_STOP_RIGHT_HIGH, 15, 15);

      /// Constructor to initialise the IDX correctly
      InterruptData(){
	setIDX_JDX( tmc429::IDX_INTERRUPT_MASK_AND_FLAGS );
      }
    };

    class DividersAndMicroStepResolutionData : public TMC429InputWord {
    public:
      ADD_VARIABLE(MicroStepResolution, 0, 2);
      ADD_VARIABLE(RampDivider, 8, 11);
      ADD_VARIABLE(PulseDivider, 12, 15);

      /// Constructor to initialise the IDX correctly
      DividersAndMicroStepResolutionData(){
	setIDX_JDX( tmc429::IDX_DIVIDERS_AND_MICRO_STEP_RESOLUTION );
      }
    };

}// namespace mtca4u

#endif// MTCA4U_TMC429WORDS_H
