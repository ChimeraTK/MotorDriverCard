#ifndef MTCA4U_MOTOR_CONTROLER_H
#define MTCA4U_MOTOR_CONTROLER_H

#include "TMC429Words.h"

namespace mtca4u
{

  /**
   * A class to controll stepper motors using the DFMC-MD22 card.
   * This class represents all the functionality of one motor. 
   * 
   * Note: Technically this controler class addresses registers of the
   * TMC 429 motor controler chip (which can handle up to 3 motors) and
   * the TMC 260 (261?) motor driver, which is indiviual for each motor.
   * On the software side these implementation details are not important.
   * Think of this class as a virtual controler with integrated driver.
   * 
   * @author Martin Killenberg <martin.killenberg@desy.de>
   */
  class MotorControler{

  public:
    
    struct DecoderReadoutMode
    {
      static const unsigned int HEIDENHAIN = 0;
      static const unsigned int INCREMENTAL = 1;
    };


    /**
     * The constructor requires a reference to the MotorDriverCard it is
     * living on.
     */
    MotorControler( MotorDriverCard & driverCard );

    // via direct register access in the fpga

    void setActualSpeed(unsigned int stepsPerFIXME); //FIXME = read only?
    void setActualAcceleration(unsigned int stepsPerSquareFIXME);
    void setAccelerationThreshold(unsigned int accelerationTreshold); //lower or upper threshold?
    void setEnabled(bool enable=true); //< Enable or disable the motor driver chip
    void setDecoderReadoutMode(unsigned int decoderReadoutMode);

    unsigned int getActualPosition(); //< Get the actual position in steps
    unsigned int getActualSpeed(); //< Get the actual speed in steps per FIXME
    unsigned int getActualAcceleration(); //< Get the actual acceleration in steps per square FIXME
    unsigned int getAccelerationThreshold(); //< Get the acceleration threshold (upper or lower???) in steps/s^2
    unsigned int getMicroStepValue(); //< Get the micro step value (which units? what does it mean?)
    unsigned int getStallGuardValue(); //< Get the stall guard value  (which units? what does it mean?)
    unsigned int getStatus(); //< Get the actual status of the motor driver
    unsigned int getDecoderReadoutMode(); //< Get the decoder readout mode
    unsigned int getDecoderPosition(); //< Get the decoder position (which units?)
    
    bool isEnabled(); //< Check whether the motor driver chip is enabled
    
    // via spi to the tmc429 motor controler
    void setTartetPosition(unsigned int steps);
    /// Set the actual position counter for a recalibration of the actual position
    void setActualPosition(unsigned int steps);
    void setMinimalSpeed(unsigned int stepsPerFIXME);
    void setMaximalSpeed(unsigned int stepsPerFIXME);
    void setTargetSpeed(unsigned int stepsPerFIXME);

     void setMaximumAcceleration(unsigned int stepsPerSquareFIXME);
     void setTargetAcceleration(unsigned int stepsPerSquareFIXME);
     void setAccelerationThresholdRegister(AccelerationThresholdData accelerationThresholdData);
     void setProportionalityFactorRegister(ProportionalityFactorData proportionalityFactorData);
     void setReferenceConfigAndRampModeRegister(ReferenceConfigAndRampModeData referenceConfigAndRampModeData);
     void setInterruptRegister(InterruptData interruptData);
     void setDividersAndMicroStepResolutionRegister(DividersAndMicroStepResolutionData dividersAndMicroStepResolutionData);
     void setPositionTolerance(unsigned int positionTolerance);
     void setPositionLatched(unsigned int positionLatched);
     void setMicroStepCount(unsigned int microStepCount);

     // the same for alle the getters
     unsigned int getTargetAcceleration() const;//< read back the target acceleration in stepsPerSquareFIXME);
     AccelerationThresholdData readAccelerationThresholdRegister() const;// or getAccelerationThReg?
     
     void setDriverControlRegister(DriverControlData driverControlData);
     void setChopperControlRegister(ChopperControlData chopperControlData);
     void setCoolStepControlRegister(CoolStepControlData coolStepControlData);
     void setStallGuardControlRegister(StallGuardControlData stallGuardControlData);
     void setDriverConfigRegister(DriverConfigData driverConfigData);

     DriverControlData const & getDriverControlData() const; 
     ChopperControlData const & getChopperControlData() const;
     CoolStepControlData const & getCoolStepControlData() const;
     StallGuardControlData const & getStallGuardControlData() const;
     DriverConfigData const & getDriverConfigData() const;
           
 
  protected:
     /// The driver card this motor controler is living on.
     MotorDriverCard & _driverCard;

     //FIXME: These variables have to be stored uniquely inter process
     DriverControlData driverControlData;
     ChopperControlData chopperControlData;
     CoolStepControlData coolStepControlData;
     StallGuardControlData stallGuardControlData;
     
 };

}// namespace mtca4u

#endif // MTCA4U_MOTOR_CONTROLER_H
