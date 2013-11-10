#define BOOST_TEST_MODULE TMC429WordsTest
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "TMC429Words.h"
using namespace mtca4u;
using namespace mtca4u::tmc429;

/** This function is used to evaluate that the bit pattern given in
 *  ADD_VARIABLE is correct. It takes an empty word and sets all bits described in the setVARIABLE function.
 *  Afterwards it returns the whole data word, which can be compared to the correct bit pattern with only the
 *  bits according to the pattern are on.
 *  By using the bit numbers in the definition and the bit pattern in the evaluation the chance of 
 *  having the same error in these two independent implementations is minimised.
 * 
 *  As setVARIABLE and getVARIABLE are both generated from the same macro, which has been tested independently,
 *  just using the setter is sufficient.
 */
template<class T> unsigned int createDataWordUsingSetterFunction( void (T::* setterFunction )(unsigned int) ){
  T t;
  (t.*setterFunction)(0xFFFFFFFF);
  return t.getDataWord();
}

unsigned int createDataWordFromIdxJdx( unsigned int idxJdx ){
  TMC429InputWord inputWord;
  inputWord.setIDX_JDX( idxJdx );
  return inputWord.getDataWord();
}

BOOST_AUTO_TEST_SUITE( TMC429WordsTestSuite )

BOOST_AUTO_TEST_CASE( testTMC429InputWord ){
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429InputWord> ( &TMC429InputWord::setRRS )     == 0x80000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429InputWord> ( &TMC429InputWord::setSMDA )    == 0x60000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429InputWord> ( &TMC429InputWord::setIDX_JDX ) == 0x1E000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429InputWord> ( &TMC429InputWord::setADDRESS ) == 0x7E000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429InputWord> ( &TMC429InputWord::setRW )      == 0x01000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429InputWord> ( &TMC429InputWord::setDATA )    == 0x00FFFFFF );
}

BOOST_AUTO_TEST_CASE( testTMC429OutputWord ){
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setSTATUS_BITS ) == 0xFF000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setINT )         == 0x80000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setCDGW )        == 0x40000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setSM3 )         == 0x30000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setRS3 )         == 0x20000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setxEQt3 )       == 0x10000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setSM2 )         == 0x0C000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setRS2 )         == 0x08000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setxEQt2 )       == 0x04000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setSM1 )         == 0x03000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setRS1 )         == 0x02000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setxEQt1 )       == 0x01000000 );
  BOOST_CHECK( createDataWordUsingSetterFunction<TMC429OutputWord> ( &TMC429OutputWord::setDATA )        == 0x00FFFFFF );
}

BOOST_AUTO_TEST_CASE( testReferenceSwitchData ){
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceSwitchData> ( &ReferenceSwitchData::setRight1 ) == 0x7C000001 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceSwitchData> ( &ReferenceSwitchData::setLeft1 )  == 0x7C000002 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceSwitchData> ( &ReferenceSwitchData::setRight2 ) == 0x7C000004 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceSwitchData> ( &ReferenceSwitchData::setLeft2 )  == 0x7C000008 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceSwitchData> ( &ReferenceSwitchData::setRight3 ) == 0x7C000010 );
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceSwitchData> ( &ReferenceSwitchData::setLeft3 )  == 0x7C000020 );
}

BOOST_AUTO_TEST_CASE( testCoverPositionAndLength ){
  BOOST_CHECK( createDataWordUsingSetterFunction<CoverPositionAndLength> ( &CoverPositionAndLength::setCoverLength )   == 0x6400001F );  
  BOOST_CHECK( createDataWordUsingSetterFunction<CoverPositionAndLength> ( &CoverPositionAndLength::setCoverPosition ) == 0x64003F00 );  
  BOOST_CHECK( createDataWordUsingSetterFunction<CoverPositionAndLength> ( &CoverPositionAndLength::setCoverWaiting )  == 0x64800000 );  
}

BOOST_AUTO_TEST_CASE( testStepperMotorGlobalParameters ){
  BOOST_CHECK( createDataWordUsingSetterFunction<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setLastStepperMotorDriver) == 0x7E000003);  
  BOOST_CHECK( createDataWordUsingSetterFunction<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setPolarities)             == 0x7E00007C);  
  BOOST_CHECK( createDataWordUsingSetterFunction<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setPolarity_nSCS_S)        == 0x7E000004);  
  BOOST_CHECK( createDataWordUsingSetterFunction<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setPolarity_SCK_S)         == 0x7E000008);  
  BOOST_CHECK( createDataWordUsingSetterFunction<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setPolarity_PH_AB)         == 0x7E000010);  
  BOOST_CHECK( createDataWordUsingSetterFunction<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setPolarity_FD_AB)         == 0x7E000020);  
  BOOST_CHECK( createDataWordUsingSetterFunction<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setPolarity_DAC_AB)        == 0x7E000040);  
  BOOST_CHECK( createDataWordUsingSetterFunction<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setCsCommonIndividual)     == 0x7E000080);  
  BOOST_CHECK( createDataWordUsingSetterFunction<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setClk2_div)               == 0x7E00FF00);  
  BOOST_CHECK( createDataWordUsingSetterFunction<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setContinuousUpdate)       == 0x7E010000);  
  BOOST_CHECK( createDataWordUsingSetterFunction<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setRefMux)                 == 0x7E100000);  
  BOOST_CHECK( createDataWordUsingSetterFunction<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setMot1r)                  == 0x7E200000);  
}

BOOST_AUTO_TEST_CASE( testInterfaceConfiguration ){
  BOOST_CHECK( createDataWordUsingSetterFunction<InterfaceConfiguration>(&InterfaceConfiguration::setInv_ref)  == 0x68000001);  
  BOOST_CHECK( createDataWordUsingSetterFunction<InterfaceConfiguration>(&InterfaceConfiguration::setSdo_int)  == 0x68000002);  
  BOOST_CHECK( createDataWordUsingSetterFunction<InterfaceConfiguration>(&InterfaceConfiguration::setStepHalf) == 0x68000004);  
  BOOST_CHECK( createDataWordUsingSetterFunction<InterfaceConfiguration>(&InterfaceConfiguration::setInv_stp)  == 0x68000008);  
  BOOST_CHECK( createDataWordUsingSetterFunction<InterfaceConfiguration>(&InterfaceConfiguration::setInv_dir)  == 0x68000010);  
  BOOST_CHECK( createDataWordUsingSetterFunction<InterfaceConfiguration>(&InterfaceConfiguration::setEs_sd)    == 0x68000020);  
  BOOST_CHECK( createDataWordUsingSetterFunction<InterfaceConfiguration>(&InterfaceConfiguration::setPos_comp_sel_0) == 0x68000040);  
  BOOST_CHECK( createDataWordUsingSetterFunction<InterfaceConfiguration>(&InterfaceConfiguration::setPos_comp_sel_1) == 0x68000080);  
  BOOST_CHECK( createDataWordUsingSetterFunction<InterfaceConfiguration>(&InterfaceConfiguration::setEn_refr)  == 0x68000100);  
}

BOOST_AUTO_TEST_CASE( testPositionCompareInterruptData ){
  BOOST_CHECK( createDataWordUsingSetterFunction<PositionCompareInterruptData>(&PositionCompareInterruptData::setInterruptFlag)  == 0x6C000001);  
  BOOST_CHECK( createDataWordUsingSetterFunction<PositionCompareInterruptData>(&PositionCompareInterruptData::setInterruptMask)  == 0x6C000100);  
}

BOOST_AUTO_TEST_CASE( testAccelerationThresholdData ){
  BOOST_CHECK( createDataWordUsingSetterFunction<AccelerationThresholdData>(&AccelerationThresholdData::setAccelerationThreshold)        == 0x100007FF);
  BOOST_CHECK( createDataWordUsingSetterFunction<AccelerationThresholdData>(&AccelerationThresholdData::setCurrentScalingAtRest)         == 0x10007000);
  BOOST_CHECK( createDataWordUsingSetterFunction<AccelerationThresholdData>(&AccelerationThresholdData::setCurrentScalingBelowThreshold) == 0x10070000);
  BOOST_CHECK( createDataWordUsingSetterFunction<AccelerationThresholdData>(&AccelerationThresholdData::setCurrentScalingAboveThreshold) == 0x10700000);
}

BOOST_AUTO_TEST_CASE( testProportionalityFactorData ){
  BOOST_CHECK( createDataWordUsingSetterFunction<ProportionalityFactorData>(&ProportionalityFactorData::setDivisionParameter)       == 0x1200000F);
  BOOST_CHECK( createDataWordUsingSetterFunction<ProportionalityFactorData>(&ProportionalityFactorData::setMultiplicationParameter) == 0x12007F00);
}

BOOST_AUTO_TEST_CASE( testReferenceConfigAndRampModeData ){
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceConfigAndRampModeData>(
		     &ReferenceConfigAndRampModeData::setRampMode)        == 0x14000003);
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceConfigAndRampModeData>(
		     &ReferenceConfigAndRampModeData::setReferenceConfig) == 0x14000F00);
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceConfigAndRampModeData>(
		     &ReferenceConfigAndRampModeData::setDISABLE_STOP_L)  == 0x14000100);
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceConfigAndRampModeData>(
                     &ReferenceConfigAndRampModeData::setDISABLE_STOP_R)  == 0x14000200);
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceConfigAndRampModeData>(
		     &ReferenceConfigAndRampModeData::setSOFT_STOP)       == 0x14000400);
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceConfigAndRampModeData>(
                     &ReferenceConfigAndRampModeData::setREF_RnL)         == 0x14000800);
  BOOST_CHECK( createDataWordUsingSetterFunction<ReferenceConfigAndRampModeData>(
                     &ReferenceConfigAndRampModeData::setLatchedPosition) == 0x14010000);
}

BOOST_AUTO_TEST_CASE( testInterruptData ){
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setInterruptFlags)      == 0x160000FF);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setINT_POS_END)         == 0x16000001);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setINT_REF_WRONG)       == 0x16000002);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setINT_REF_MISS)        == 0x16000004);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setINT_STOP)            == 0x16000008);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setINT_STOP_LEFT_LOW)   == 0x16000010);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setINT_STOP_RIGHT_LOW)  == 0x16000020);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setINT_STOP_LEFT_HIGH)  == 0x16000040);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setINT_STOP_RIGHT_HIGH) == 0x16000080);

  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setMaskFlags)            == 0x1600FF00);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setMASK_POS_END)         == 0x16000100);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setMASK_REF_WRONG)       == 0x16000200);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setMASK_REF_MISS)        == 0x16000400);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setMASK_STOP)            == 0x16000800);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setMASK_STOP_LEFT_LOW)   == 0x16001000);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setMASK_STOP_RIGHT_LOW)  == 0x16002000);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setMASK_STOP_LEFT_HIGH)  == 0x16004000);
  BOOST_CHECK( createDataWordUsingSetterFunction<InterruptData>(&InterruptData::setMASK_STOP_RIGHT_HIGH) == 0x16008000);
}

BOOST_AUTO_TEST_CASE( testDividersAndMicroStepResolutionData ){
  BOOST_CHECK( createDataWordUsingSetterFunction<DividersAndMicroStepResolutionData>(&DividersAndMicroStepResolutionData::setMicroStepResolution) == 0x18000007);
  BOOST_CHECK( createDataWordUsingSetterFunction<DividersAndMicroStepResolutionData>(&DividersAndMicroStepResolutionData::setRampDivider)         == 0x18000F00);
  BOOST_CHECK( createDataWordUsingSetterFunction<DividersAndMicroStepResolutionData>(&DividersAndMicroStepResolutionData::setPulseDivider)        == 0x1800F000);
}

/**
 * For those classes which set IDX / JDX in their constructor these constants are automatically testest with the tests above.
 * For the other constants do the tests here.
 */
BOOST_AUTO_TEST_CASE( testRemainingJDX ){
  BOOST_CHECK( createDataWordFromIdxJdx( JDX_DATAGRAM_LOW_WORD ) == 0x00000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( JDX_DATAGRAM_HIGH_WORD ) == 0x02000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( JDX_COVER_DATAGRAM ) == 0x06000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( JDX_POSITION_COMPARE ) == 0x0A000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( JDX_POWER_DOWN ) == 0x10000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( JDX_CHIP_VERSION ) == 0x12000000 );
}

BOOST_AUTO_TEST_CASE( testRemainingIDX ){
  BOOST_CHECK( createDataWordFromIdxJdx( IDX_TARGET_POSITION ) == 0x00000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( IDX_ACTUAL_POSITION ) == 0x02000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( IDX_MINIMUM_VELOCITY ) == 0x04000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( IDX_MAXIMUM_VELOCITY )== 0x06000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( IDX_TARGET_VELOCITY ) == 0x08000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( IDX_ACTUAL_VELOCITY ) == 0x0A000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( IDX_MAXIMUM_ACCELERATION ) == 0x0C000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( IDX_ACTUAL_ACCELERATION ) == 0x0E000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( IDX_DELTA_X_REFERENCE_TOLERANCE ) == 0x1A000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( IDX_POSITION_LATCHED ) == 0x1C000000 );
  BOOST_CHECK( createDataWordFromIdxJdx( IDX_MICRO_STEP_COUNT ) == 0x1E000000 );
}

BOOST_AUTO_TEST_SUITE_END()
