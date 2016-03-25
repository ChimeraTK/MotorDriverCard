#define BOOST_TEST_MODULE TMC429WordsTest
#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test_framework;

#include "createOutputMaskUsingSetterGetter.h"
#include "TMC429Words.h"
#include "MotorDriverException.h"
using namespace mtca4u;
using namespace mtca4u::tmc429;

unsigned int createDataWordFromIdxJdx( unsigned int idxJdx ){
  TMC429InputWord inputWord;
  inputWord.setIDX_JDX( idxJdx );
  return inputWord.getDataWord();
}

BOOST_AUTO_TEST_SUITE( TMC429WordsTestSuite )

BOOST_AUTO_TEST_CASE( testTMC429InputWord ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429InputWord> ( &TMC429InputWord::setRRS,
								   &TMC429InputWord::getRRS )
	      == 0x80000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429InputWord> ( &TMC429InputWord::setSMDA,
								   &TMC429InputWord::getSMDA )
	      == 0x60000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429InputWord> ( &TMC429InputWord::setIDX_JDX,
								   &TMC429InputWord::getIDX_JDX )
	      == 0x1E000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429InputWord> ( &TMC429InputWord::setADDRESS,
								   &TMC429InputWord::getADDRESS )
	      == 0x7E000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429InputWord> ( &TMC429InputWord::setRW,
								   &TMC429InputWord::getRW )
	      == 0x01000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429InputWord> ( &TMC429InputWord::setDATA,
								   &TMC429InputWord::getDATA )
	      == 0x00FFFFFF );
}

BOOST_AUTO_TEST_CASE( testTMC429OutputWord ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setSTATUS_BITS,
								    &TMC429OutputWord::getSTATUS_BITS )
	      == 0xFF000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setINT,
								    &TMC429OutputWord::getINT )
	      == 0x80000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setCDGW,
								    &TMC429OutputWord::getCDGW )
	      == 0x40000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setSM3,
								    &TMC429OutputWord::getSM3 )
	      == 0x30000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setRS3,
								    &TMC429OutputWord::getRS3 )
	      == 0x20000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setxEQt3,
								    &TMC429OutputWord::getxEQt3 )
	      == 0x10000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setSM2,
								    &TMC429OutputWord::getSM2 )
	      == 0x0C000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setRS2,
								    &TMC429OutputWord::getRS2 )
	      == 0x08000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setxEQt2,
								    &TMC429OutputWord::getxEQt2 )
	      == 0x04000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setSM1,
								    &TMC429OutputWord::getSM1 )
	      == 0x03000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setRS1,
								    &TMC429OutputWord::getRS1 )
	      == 0x02000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setxEQt1,
								    &TMC429OutputWord::getxEQt1 )
	      == 0x01000000 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429OutputWord> ( &TMC429OutputWord::setDATA,
								    &TMC429OutputWord::getDATA )
	      == 0x00FFFFFF );
}

BOOST_AUTO_TEST_CASE( testReferenceSwitchData ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceSwitchData> ( &ReferenceSwitchData::setRight1,
								       &ReferenceSwitchData::getRight1 )
	      == 0x7C000001 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceSwitchData> ( &ReferenceSwitchData::setLeft1,
								       &ReferenceSwitchData::getLeft1 )
	      == 0x7C000002 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceSwitchData> ( &ReferenceSwitchData::setRight2,
								       &ReferenceSwitchData::getRight2 )
	      == 0x7C000004 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceSwitchData> ( &ReferenceSwitchData::setLeft2,
								       &ReferenceSwitchData::getLeft2 )
	      == 0x7C000008 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceSwitchData> ( &ReferenceSwitchData::setRight3,
								       &ReferenceSwitchData::getRight3 )
	      == 0x7C000010 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceSwitchData> ( &ReferenceSwitchData::setLeft3,
								       &ReferenceSwitchData::getLeft3 )
	      == 0x7C000020 );
}

BOOST_AUTO_TEST_CASE( testCoverPositionAndLength ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<CoverPositionAndLength> ( &CoverPositionAndLength::setCoverLength,
									  &CoverPositionAndLength::getCoverLength )   == 0x6400001F );  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<CoverPositionAndLength> ( &CoverPositionAndLength::setCoverPosition,
									  &CoverPositionAndLength::getCoverPosition ) == 0x64003F00 );  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<CoverPositionAndLength> ( &CoverPositionAndLength::setCoverWaiting,
									  &CoverPositionAndLength::getCoverWaiting )  == 0x64800000 );  
}

BOOST_AUTO_TEST_CASE( testStepperMotorGlobalParameters ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setLastStepperMotorDriver,
									      &StepperMotorGlobalParameters::getLastStepperMotorDriver) == 0x7E000003);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setPolarities,
									      &StepperMotorGlobalParameters::getPolarities)             == 0x7E00007C);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setPolarity_nSCS_S,
									      &StepperMotorGlobalParameters::getPolarity_nSCS_S)        == 0x7E000004);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setPolarity_SCK_S,
									      &StepperMotorGlobalParameters::getPolarity_SCK_S)         == 0x7E000008);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setPolarity_PH_AB,
									      &StepperMotorGlobalParameters::getPolarity_PH_AB)         == 0x7E000010);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setPolarity_FD_AB,
									      &StepperMotorGlobalParameters::getPolarity_FD_AB)         == 0x7E000020);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setPolarity_DAC_AB,
									      &StepperMotorGlobalParameters::getPolarity_DAC_AB)        == 0x7E000040);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setCsCommonIndividual,
									      &StepperMotorGlobalParameters::getCsCommonIndividual)     == 0x7E000080);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setClk2_div,
									      &StepperMotorGlobalParameters::getClk2_div)               == 0x7E00FF00);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setContinuousUpdate,
									      &StepperMotorGlobalParameters::getContinuousUpdate)       == 0x7E010000);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setRefMux,
									      &StepperMotorGlobalParameters::getRefMux)                 == 0x7E100000);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<StepperMotorGlobalParameters>(&StepperMotorGlobalParameters::setMot1r,
									      &StepperMotorGlobalParameters::getMot1r)                  == 0x7E200000);  
}

BOOST_AUTO_TEST_CASE( testInterfaceConfiguration ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterfaceConfiguration>(&InterfaceConfiguration::setInv_ref,
									&InterfaceConfiguration::getInv_ref)  == 0x68000001);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterfaceConfiguration>(&InterfaceConfiguration::setSdo_int,
									&InterfaceConfiguration::getSdo_int)  == 0x68000002);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterfaceConfiguration>(&InterfaceConfiguration::setStepHalf,
									&InterfaceConfiguration::getStepHalf) == 0x68000004);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterfaceConfiguration>(&InterfaceConfiguration::setInv_stp,
									&InterfaceConfiguration::getInv_stp)  == 0x68000008);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterfaceConfiguration>(&InterfaceConfiguration::setInv_dir,
									&InterfaceConfiguration::getInv_dir)  == 0x68000010);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterfaceConfiguration>(&InterfaceConfiguration::setEs_sd,
									&InterfaceConfiguration::getEs_sd)    == 0x68000020);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterfaceConfiguration>(&InterfaceConfiguration::setPos_comp_sel_0,
									&InterfaceConfiguration::getPos_comp_sel_0) == 0x68000040);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterfaceConfiguration>(&InterfaceConfiguration::setPos_comp_sel_1,
									&InterfaceConfiguration::getPos_comp_sel_1) == 0x68000080);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterfaceConfiguration>(&InterfaceConfiguration::setEn_refr,
									&InterfaceConfiguration::getEn_refr)  == 0x68000100);  
}

BOOST_AUTO_TEST_CASE( testPositionCompareInterruptData ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<PositionCompareInterruptData>(&PositionCompareInterruptData::setInterruptFlag,
									      &PositionCompareInterruptData::getInterruptFlag)  == 0x6C000001);  
  BOOST_CHECK(createOutputMaskUsingSetterGetter<PositionCompareInterruptData>(&PositionCompareInterruptData::setInterruptMask,
									      &PositionCompareInterruptData::getInterruptMask)  == 0x6C000100);  
}

BOOST_AUTO_TEST_CASE( testAccelerationThresholdData ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<AccelerationThresholdData>(&AccelerationThresholdData::setAccelerationThreshold,
									   &AccelerationThresholdData::getAccelerationThreshold)        == 0x100007FF);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<AccelerationThresholdData>(&AccelerationThresholdData::setCurrentScalingAtRest,
									   &AccelerationThresholdData::getCurrentScalingAtRest)         == 0x10007000);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<AccelerationThresholdData>(&AccelerationThresholdData::setCurrentScalingBelowThreshold,
									   &AccelerationThresholdData::getCurrentScalingBelowThreshold) == 0x10070000);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<AccelerationThresholdData>(&AccelerationThresholdData::setCurrentScalingAboveThreshold,
									   &AccelerationThresholdData::getCurrentScalingAboveThreshold) == 0x10700000);
}

BOOST_AUTO_TEST_CASE( testProportionalityFactorData ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ProportionalityFactorData>(&ProportionalityFactorData::setDivisionParameter,
									   &ProportionalityFactorData::getDivisionParameter)       == 0x1200000F);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ProportionalityFactorData>(&ProportionalityFactorData::setMultiplicationParameter,
									   &ProportionalityFactorData::getMultiplicationParameter) == 0x12007F00);
}

BOOST_AUTO_TEST_CASE( testReferenceConfigAndRampModeData ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceConfigAndRampModeData>(
		     &ReferenceConfigAndRampModeData::setRampMode,
		     &ReferenceConfigAndRampModeData::getRampMode)        == 0x14000003);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceConfigAndRampModeData>(
		     &ReferenceConfigAndRampModeData::setReferenceConfig,
		     &ReferenceConfigAndRampModeData::getReferenceConfig) == 0x14000F00);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceConfigAndRampModeData>(
		     &ReferenceConfigAndRampModeData::setDISABLE_STOP_L,
		     &ReferenceConfigAndRampModeData::getDISABLE_STOP_L)  == 0x14000100);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceConfigAndRampModeData>(
		     &ReferenceConfigAndRampModeData::setDISABLE_STOP_R,
		     &ReferenceConfigAndRampModeData::getDISABLE_STOP_R)  == 0x14000200);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceConfigAndRampModeData>(
		     &ReferenceConfigAndRampModeData::setSOFT_STOP,
		     &ReferenceConfigAndRampModeData::getSOFT_STOP)       == 0x14000400);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceConfigAndRampModeData>(
		     &ReferenceConfigAndRampModeData::setREF_RnL,
		     &ReferenceConfigAndRampModeData::getREF_RnL)         == 0x14000800);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<ReferenceConfigAndRampModeData>(
		     &ReferenceConfigAndRampModeData::setLatchedPosition,
		     &ReferenceConfigAndRampModeData::getLatchedPosition) == 0x14010000);
}

BOOST_AUTO_TEST_CASE( testInterruptData ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setInterruptFlags,
							       &InterruptData::getInterruptFlags)      == 0x160000FF);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setINT_POS_END,
							       &InterruptData::getINT_POS_END)         == 0x16000001);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setINT_REF_WRONG,
							       &InterruptData::getINT_REF_WRONG)       == 0x16000002);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setINT_REF_MISS,
							       &InterruptData::getINT_REF_MISS)        == 0x16000004);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setINT_STOP,
							       &InterruptData::getINT_STOP)            == 0x16000008);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setINT_STOP_LEFT_LOW,
							       &InterruptData::getINT_STOP_LEFT_LOW)   == 0x16000010);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setINT_STOP_RIGHT_LOW,
							       &InterruptData::getINT_STOP_RIGHT_LOW)  == 0x16000020);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setINT_STOP_LEFT_HIGH,
							       &InterruptData::getINT_STOP_LEFT_HIGH)  == 0x16000040);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setINT_STOP_RIGHT_HIGH,
							       &InterruptData::getINT_STOP_RIGHT_HIGH) == 0x16000080);

  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setMaskFlags,
							       &InterruptData::getMaskFlags)            == 0x1600FF00);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setMASK_POS_END,
							       &InterruptData::getMASK_POS_END)         == 0x16000100);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setMASK_REF_WRONG,
							       &InterruptData::getMASK_REF_WRONG)       == 0x16000200);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setMASK_REF_MISS,
							       &InterruptData::getMASK_REF_MISS)        == 0x16000400);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setMASK_STOP,
							       &InterruptData::getMASK_STOP)            == 0x16000800);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setMASK_STOP_LEFT_LOW,
							       &InterruptData::getMASK_STOP_LEFT_LOW)   == 0x16001000);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setMASK_STOP_RIGHT_LOW,
							       &InterruptData::getMASK_STOP_RIGHT_LOW)  == 0x16002000);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setMASK_STOP_LEFT_HIGH,
							       &InterruptData::getMASK_STOP_LEFT_HIGH)  == 0x16004000);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<InterruptData>(&InterruptData::setMASK_STOP_RIGHT_HIGH,
							       &InterruptData::getMASK_STOP_RIGHT_HIGH) == 0x16008000);
}

BOOST_AUTO_TEST_CASE( testDividersAndMicroStepResolutionData ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<DividersAndMicroStepResolutionData>(&DividersAndMicroStepResolutionData::setMicroStepResolution,
										    &DividersAndMicroStepResolutionData::getMicroStepResolution)
	      == 0x18000007);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<DividersAndMicroStepResolutionData>(&DividersAndMicroStepResolutionData::setRampDivider,
										    &DividersAndMicroStepResolutionData::getRampDivider)
	      == 0x18000F00);
  BOOST_CHECK(createOutputMaskUsingSetterGetter<DividersAndMicroStepResolutionData>(&DividersAndMicroStepResolutionData::setPulseDivider,
										    &DividersAndMicroStepResolutionData::getPulseDivider)
	      == 0x1800F000);
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

BOOST_AUTO_TEST_CASE( testTMC429StatusWord ){
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429StatusWord> ( &TMC429StatusWord::setInterrupt,
								   &TMC429StatusWord::getInterrupt )
	      == 0x80 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429StatusWord> ( &TMC429StatusWord::setCoverDatagramWaiting,
								   &TMC429StatusWord::getCoverDatagramWaiting )
	      == 0x40 );

  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429StatusWord> ( &TMC429StatusWord::setStepperMotorBits3,
								   &TMC429StatusWord::getStepperMotorBits3 )
	      == 0x30 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429StatusWord> ( &TMC429StatusWord::setReferenceSwitchBit3,
								   &TMC429StatusWord::getReferenceSwitchBit3 )
	      == 0x20 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429StatusWord> ( &TMC429StatusWord::setTargetPositionReached3,
								   &TMC429StatusWord::getTargetPositionReached3 )
	      == 0x10 );

  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429StatusWord> ( &TMC429StatusWord::setStepperMotorBits2,
								   &TMC429StatusWord::getStepperMotorBits2 )
	      == 0x0C );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429StatusWord> ( &TMC429StatusWord::setReferenceSwitchBit2,
								   &TMC429StatusWord::getReferenceSwitchBit2 )
	      == 0x08 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429StatusWord> ( &TMC429StatusWord::setTargetPositionReached2,
								   &TMC429StatusWord::getTargetPositionReached2 )
	      == 0x04 );

  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429StatusWord> ( &TMC429StatusWord::setStepperMotorBits1,
								   &TMC429StatusWord::getStepperMotorBits1 )
	      == 0x03 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429StatusWord> ( &TMC429StatusWord::setReferenceSwitchBit1,
								   &TMC429StatusWord::getReferenceSwitchBit1 )
	      == 0x02 );
  BOOST_CHECK(createOutputMaskUsingSetterGetter<TMC429StatusWord> ( &TMC429StatusWord::setTargetPositionReached1,
								   &TMC429StatusWord::getTargetPositionReached1 )
	      == 0x01 );

  // Feed in words with just one bit set and try to find it back using getTargetPositionReached and
  // getReferenceSwitchBit.
  BOOST_CHECK( TMC429StatusWord(0x1).getTargetPositionReached(0) );
  BOOST_CHECK( TMC429StatusWord(0x4).getTargetPositionReached(1) );
  BOOST_CHECK( TMC429StatusWord(0x10).getTargetPositionReached(2) );
  BOOST_CHECK_THROW( TMC429StatusWord().getTargetPositionReached(3) , MotorDriverException );

  BOOST_CHECK( TMC429StatusWord(0x2).getReferenceSwitchBit(0)==0x1 );
  BOOST_CHECK( TMC429StatusWord(0x8).getReferenceSwitchBit(1)==0x1 );
  BOOST_CHECK( TMC429StatusWord(0x20).getReferenceSwitchBit(2)==0x1 );
  BOOST_CHECK_THROW( TMC429StatusWord().getReferenceSwitchBit(3) , MotorDriverException );
}

BOOST_AUTO_TEST_SUITE_END()
