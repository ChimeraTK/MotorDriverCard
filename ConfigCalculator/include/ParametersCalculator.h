#ifndef MTCA4U_TMC429_PARAMETERS_CALCULATOR_H
#define MTCA4U_TMC429_PARAMETERS_CALCULATOR_H

#include <list>
#include <string>

class ParametersCalculator{
 public:
  
  /** The physical parameters from the motor's data sheet.
   */
  class PhysicalParameters{
  public:
    /** The system clock in MHz has to be in the rage 1 .. 32 MHz
     */
    double systemClock;

    /** The number of microsteps has to be 2^i with i being an integer
     *  in the range 0..6.
     */
    double microsteps;

    /** Full steps per turn.
     */
    double nStepsPerTurn;
    
    /** Maximum velocity in rounds per minute
     */
    double maxRPM;

    /** Time to reach the maximum velocity in seconds.
     */
    double timeToVMax;

    /** maximum coil current in A
     */
    double iMax;

    /** convenience constructor to set all values
     */
    PhysicalParameters(double systemClock_, double microsteps_,
		       double nStepsPerTurn_, double maxRPM_,
		       double timeToVMax_, double iMax_) :
    systemClock(systemClock_), microsteps(microsteps_),
    nStepsPerTurn(nStepsPerTurn_),  maxRPM(maxRPM_),
    timeToVMax(timeToVMax_), iMax(iMax_){}
  };

  /** The parameters for the TMC426 controller and TMC260 driver chips.
   *  It contains a list of warnings which are generated during the 
   *  conversion.
   */
  struct TMC429Parameters{
    unsigned int pulseDiv;
    unsigned int rampDiv;
    unsigned int aMax;
    unsigned int vMax;
    unsigned int pDiv;
    unsigned int pMul;
    unsigned int controllerMicroStepValue;
    unsigned int driverMicroStepValue;
    unsigned int currentScale;

    std::list<std::string> warnings;
    
    /** Convenience constructor to set all values
     */
    TMC429Parameters(unsigned int pulseDiv_,
		     unsigned int rampDiv_,
		     unsigned int aMax_,
		     unsigned int vMax_,
		     unsigned int pDiv_,
		     unsigned int pMul_,
		     unsigned int controllerMicroStepValue_,
		     unsigned int driverMicroStepValue_,
		     unsigned int currentScale_,
		     std::list<std::string> const & warnings_) :
    pulseDiv(pulseDiv_), rampDiv(rampDiv_), aMax(aMax_), vMax(vMax_),
    pDiv(pDiv_), pMul(pMul_), 
    controllerMicroStepValue(controllerMicroStepValue_),
    driverMicroStepValue(driverMicroStepValue_),
    currentScale(currentScale_), warnings(warnings_){}
  };

//  /** The (hex) values which have to be written to the registers.
//   */
//  struct RegisterValues{
//    uint32_t driverControlData;
//    uint32_t stallGuardControlData;
//    uint32_t referenceConfigAndRampModeData;
//    uint32_t proportionalityFactorData;
//    uint32_t dividersAndMicroStepResolutionData;
//    uint32_t maximumVelocity;
//    uint32_t maximumAcceleration;
//  };

    /** Calculate the TMC429 parameters from the physical parameters
     */
  static TMC429Parameters
    calculateParameters(PhysicalParameters physicalParameters);
  
//  static RegisterValues
//    calculateRegisterValues(TMC429Parameters TMC429Parameters);
  
 private:
  static const double secondsPerMinute;
  static const double nVelocityValues;
  static const double maxVelocityValue;
  static const double nAccelerationValues;
  static const double maxAccelerationValue;
  // see page 23, block clk_div32
  static const double systemClockPreDivider;
  // see section 9.14, page 29 (part of the 2^29)
  static const double velocityAccumulationClockPreDivider;
  static const double pMulMin;
  static const double pMulMax;

  static const double reductionValue;

  /// The maximum possible current on the MD22 board in A
  static const double iMaxMD22;
  static const double nCurrentScaleValues;

  /** Performs the calculation of the parameter and does a validity check
   */
  static unsigned int calculateDriverMicroStepValue(double microsteps);
  
  static std::list<std::string> inputcheck(PhysicalParameters physicalParameters );
};

#endif// MTCA4U_TMC429_PARAMETERS_CALCULATOR_H
