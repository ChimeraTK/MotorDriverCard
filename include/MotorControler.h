#ifndef MTCA4U_MOTOR_CONTROLER_H
#define MTCA4U_MOTOR_CONTROLER_H

#include <MtcaMappedDevice/devMap.h>

#include "TMC260Words.h"
#include "TMC429Words.h"
#include "MotorControlerConfig.h"
#include "MotorReferenceSwitchData.h"
#include "SignedIntConverter.h"
#include "SPIviaPCIe.h"
#include "TMC429SPI.h"

#define DECLARE_SET_GET_VALUE( NAME, VARIABLE_IN_UNITS )\
  void set ## NAME (unsigned int VARIABLE_IN_UNITS );	\
  unsigned int get ## NAME ()

#define DECLARE_SIGNED_SET_GET_VALUE( NAME, VARIABLE_IN_UNITS )\
  void set ## NAME (int VARIABLE_IN_UNITS );	\
  int get ## NAME ()

#define DECLARE_SET_GET_TYPED_REGISTER( NAME, VARIABLE_NAME )\
  void set ## NAME ( NAME const & VARIABLE_NAME );\
  NAME get ## NAME ()

namespace mtca4u
{
  class MotorDriverCardImpl;

  /**
   * A class to controll stepper motors using the DFMC-MD22 card.
   * This class represents all the functionality of one motor. 
   * 
   * Note: Technically this controler class addresses registers of the
   * TMC 429 motor controler chip (which can handle up to 3 motors) and
   * the TMC 260 motor driver, which is indiviual for each motor.
   * On the software side these implementation details are not important.
   * Think of this class as a virtual controler with integrated driver.
   * 
   * @author Martin Killenberg <martin.killenberg@desy.de>
   */
  class MotorControler{

  public:
    
    /// Helper struct to hold the readout modes.
    struct DecoderReadoutMode
    {
      static const unsigned int HEIDENHAIN = 0;
      static const unsigned int INCREMENTAL = 1;
    };

    /** The constructor gets references shared pointers which it copies internally, so the
     *  object stays valid even if the original shared pointer goes out of scope.
     *  The config is only used in the constuctor, no reference is kept in the class.
     */
    MotorControler( unsigned int ID, boost::shared_ptr< devMap<devBase> > const & mappedDevice,
		     boost::shared_ptr< TMC429SPI > const & controlerSPI,
		     MotorControlerConfig const & motorControlerConfig );

    /// Get the ID of the motor controler on the FMC board (0 or 1).
    unsigned int getID();

    // via direct register access in the fpga
    // Note: The getters which require hardware accress are not const because they require an SPI write,
    // and the SPI write cannot be const because it is also used to modify.

    int getActualPosition(); ///< Get the actual position in steps
    int getActualVelocity(); ///< Get the actual velocity in steps per FIXME
    unsigned int getActualAcceleration(); ///< Get the actual acceleration in steps per square FIXME
    unsigned int getMicroStepCount(); ///< Get the micro step value (which units? what does it mean?)
    unsigned int getStallGuardValue(); ///< Get the stall guard value  (which units? what does it mean? Expert?)
    unsigned int getCoolStepValue(); ///< Get the CoolStepValue (in units?, what does it mean? Expert?)
    DriverStatusData getStatus(); ///< Get the status of the motor driver
    unsigned int getDecoderReadoutMode(); ///< Get the decoder readout mode
    unsigned int getDecoderPosition(); ///< Get the decoder position (which units?)
    
    void setActualVelocity(int stepsPerFIXME); //FIXME = read only?
    void setActualAcceleration(unsigned int stepsPerSquareFIXME);
    //    void setAccelerationThreshold(unsigned int accelerationTreshold);
    void setMicroStepCount(unsigned int microStepCount);
    void setEnabled(bool enable=true); ///< Enable or disable the motor driver chip
    void setDecoderReadoutMode(unsigned int decoderReadoutMode);

    bool isEnabled(); ///< Check whether the motor driver chip is enabled
    
    MotorReferenceSwitchData getReferenceSwitchData(); ///< Get information about both reference switches of this Motor

    /** Enable or disable the positive reference switch. (true=enabled)*/
    void setPositiveReferenceSwitchEnabled(bool enableStatus);
    /** Enable or disable the negative reference switch. (true=enabled)*/
    void setNegativeReferenceSwitchEnabled(bool enableStatus);

    // via spi to the tmc429 motor controler
    /// Set the actual position counter for a recalibration of the actual position
    void setActualPosition(int steps);
    DECLARE_SIGNED_SET_GET_VALUE( TargetPosition, steps ); ///< Get the target position in steps
    DECLARE_SET_GET_VALUE( MinimumVelocity, stepsPerFIXME );///< Get minimum velocity in FIXME
    DECLARE_SET_GET_VALUE( MaximumVelocity, stepsPerFIXME );///< Get maximum velocity in FIXME
    DECLARE_SIGNED_SET_GET_VALUE( TargetVelocity, stepsPerFIXME );///< Get target velocity in FIXME
    DECLARE_SET_GET_VALUE( MaximumAcceleration, stepsPerSquareFIXME );///< Get the maximim acceleration in FIXME
    DECLARE_SET_GET_VALUE( PositionTolerance, steps );///< Get the position tolerance in steps
    DECLARE_SET_GET_VALUE( PositionLatched, steps );///< Get the latched position in steps

    DECLARE_SET_GET_TYPED_REGISTER(AccelerationThresholdData, accelerationThresholdData);
    DECLARE_SET_GET_TYPED_REGISTER(ProportionalityFactorData, proportionalityFactorData);
    DECLARE_SET_GET_TYPED_REGISTER(ReferenceConfigAndRampModeData, referenceConfigAndRampModeData);
    DECLARE_SET_GET_TYPED_REGISTER(InterruptData, interruptData);
    DECLARE_SET_GET_TYPED_REGISTER(DividersAndMicroStepResolutionData, dividersAndMicroStepResolutionData);

     void setDriverControlData(DriverControlData const & driverControlData);
     void setChopperControlData(ChopperControlData const &  chopperControlData);
     void setCoolStepControlData(CoolStepControlData const &  coolStepControlData);
     void setStallGuardControlData(StallGuardControlData const &  stallGuardControlData);
     void setDriverConfigData(DriverConfigData const &  driverConfigData);

     DriverControlData const & getDriverControlData() const; 
     ChopperControlData const & getChopperControlData() const;
     CoolStepControlData const & getCoolStepControlData() const;
     StallGuardControlData const & getStallGuardControlData() const;
     DriverConfigData const & getDriverConfigData() const;
 
     /** Extracts the correct bit from the controler status word. */
     bool targetPositionReached();

     /** Returns the ReferenceSwitchBit for this motor as defined in section 10.1 of the TMC429 data sheet.
      *  This function the correct bit from the controler status word and stores it as the least significant bit. */
     unsigned int getReferenceSwitchBit();
 
  private:
     boost::shared_ptr< devMap<devBase> > _mappedDevice;

     unsigned int _id;

     //FIXME: These variables have to be stored uniquely inter process
     DriverControlData _driverControlData;
     ChopperControlData _chopperControlData;
     CoolStepControlData _coolStepControlData;
     StallGuardControlData _stallGuardControlData;
     DriverConfigData _driverConfigData;

     mtca4u::devMap< devBase >::regObject _controlerStatus;

     mtca4u::devMap< devBase >::regObject _actualPosition;
     mtca4u::devMap< devBase >::regObject _actualVelocity;
     mtca4u::devMap< devBase >::regObject _actualAcceleration;
     mtca4u::devMap< devBase >::regObject _microStepCount;
     mtca4u::devMap< devBase >::regObject _stallGuardValue;
     mtca4u::devMap< devBase >::regObject _coolStepValue;
     mtca4u::devMap< devBase >::regObject _status;
     mtca4u::devMap< devBase >::regObject _enabled;
     mtca4u::devMap< devBase >::regObject _decoderReadoutMode;
     mtca4u::devMap< devBase >::regObject _decoderPosition;
     
     mtca4u::SPIviaPCIe _driverSPI;
     boost::shared_ptr<mtca4u::TMC429SPI>  _controlerSPI;

     /// Simplify the syntax to read from a regObject which need call by reference.
     /// Remove this function once the regObject interface has been fixed.
     unsigned int readRegObject( 
			 mtca4u::devMap<devBase>::regObject const & registerAccessor);

     template<class T>
       T readTypedRegister();

     void writeTypedControlerRegister(TMC429InputWord inputWord);

     template <class T>
       void setTypedDriverData(T const & driverData, T & localDataInstance);

     SignedIntConverter converter24bits;
     SignedIntConverter converter12bits;
  };

}// namespace mtca4u

#endif // MTCA4U_MOTOR_CONTROLER_H
