#include "ParametersCalculator.h"
#include <cmath>
#include <stdexcept>

const double ParametersCalculator::secondsPerMinute =60.;
const double ParametersCalculator::nVelocityValues = 2048;
const double ParametersCalculator::maxVelocityValue = nVelocityValues-1;
const double ParametersCalculator::nAccelerationValues = 2048;
const double ParametersCalculator::maxAccelerationValue = nAccelerationValues-1;

// see page 23, block clk_div32
const double ParametersCalculator::systemClockPreDivider = 32;

//see section 9.14, page 29 (part of the 2^29)
const double ParametersCalculator::velocityAccumulationClockPreDivider = 256;

const double ParametersCalculator::pMulMin = 128;
const double ParametersCalculator::pMulMax = 255;

// 5 % as recommended in section 9.11 page 24
const double ParametersCalculator::reductionValue = 0.05; 

const double ParametersCalculator::iMaxMD22 = 1.8;
const double ParametersCalculator::nCurrentScaleValues = 32; 

ParametersCalculator::TMC429Parameters 
ParametersCalculator::calculateParameters( ParametersCalculator::PhysicalParameters physicalParameters){
  // inputcheck();

  // this also checks the validity of microsteps
  unsigned int driverMicroStepValue = 
    calculateDriverMicroStepValue(physicalParameters.microsteps);
  
  // safe because the validity of microsteps has been checked before
  unsigned int controllerMicroStepValue = 
    static_cast<unsigned int>(log2( physicalParameters.microsteps));

  double targetMicrostepFrequency = 
    physicalParameters.nStepsPerTurn // full steps per turn
    * physicalParameters.maxRPM / secondsPerMinute // motor rotation in Hz
    * physicalParameters.microsteps; // microsteps per full step

  double dividedSystemClockInHz = 
    physicalParameters.systemClock * 1e6 / systemClockPreDivider;

  unsigned int pulseDiv = 
    static_cast<unsigned int>(log2( dividedSystemClockInHz 
				    / targetMicrostepFrequency
				    * maxVelocityValue/nVelocityValues));
  
  double maxMirostepFrequencyWithThisPulseDiv = 
    dividedSystemClockInHz * maxVelocityValue/nVelocityValues
    / pow(2, pulseDiv);

  unsigned int vMax = 
    static_cast<unsigned int>( targetMicrostepFrequency
			       / maxMirostepFrequencyWithThisPulseDiv 
			       * maxVelocityValue);

  double targetMicrostepAcceleration = 
    targetMicrostepFrequency / physicalParameters.timeToVMax;

  unsigned int rampDiv = 
    static_cast<unsigned int>(log2( dividedSystemClockInHz
				    *dividedSystemClockInHz
				    * maxAccelerationValue
				    / targetMicrostepAcceleration
				    / pow(2, pulseDiv)
				    / nAccelerationValues
				    /velocityAccumulationClockPreDivider));

  double maxMicrostepAccelerationWithThisRampDiv =
    dividedSystemClockInHz * dividedSystemClockInHz
    * maxAccelerationValue / nAccelerationValues
    /velocityAccumulationClockPreDivider / pow(2, rampDiv + pulseDiv);

  unsigned int aMax = 
    static_cast<int>( targetMicrostepAcceleration
		      / maxMicrostepAccelerationWithThisRampDiv
		      * maxAccelerationValue );

  double p = aMax/pMulMin/ pow(2, rampDiv-pulseDiv);

  double reducedP = (1.- reductionValue) * p;

  // sorry for the hard coded 3. it is log2( maxAccelerationValue / 256 )
  // where it is not clear where the 256 comes from. Yet another divider. 
  unsigned int pDiv = 
    static_cast<unsigned int>(log2( pMulMax / reducedP ) - 3); 

  unsigned int pMul =
    static_cast<unsigned int>(reducedP * pow(2, pDiv+3));

  double maxPossibleCurrent = std::min(iMaxMD22, physicalParameters.iMax);

  unsigned int currentScale = 
    static_cast<unsigned int>( std::ceil(maxPossibleCurrent/iMaxMD22
					 * nCurrentScaleValues) ) -1;

  return TMC429Parameters(pulseDiv, rampDiv, aMax, vMax,
			  pDiv, pMul,
			  controllerMicroStepValue,
			  driverMicroStepValue,
			  currentScale );
 
}

unsigned int ParametersCalculator::calculateDriverMicroStepValue(double microsteps){
  // the switch statement needs an int, not a double
  int microstepsInt = static_cast<int>(microsteps);
  if (microstepsInt != microsteps){
    throw std::invalid_argument("microsteps has to an integer");
  }

  switch (microstepsInt){
  case 1:  return 8;
  case 2:  return 7;
  case 4:  return 6;
  case 8:  return 5;
  case 16: return 4;
  case 32: return 3;
  case 64: return 2;
    // although 128 and 256 microsteps are valid for the driver
    // they are invalid here because the controller can only to 64
  default:
    throw std::invalid_argument("The number of microsteps has to be 2^i with i being an integer in the range 0..6.");
  }
}
