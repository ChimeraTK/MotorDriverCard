#ifndef MTCA4U_MOTOR_CONTROLER_IMPL_H
#define MTCA4U_MOTOR_CONTROLER_IMPL_H

#include "MotorControlerExpert.h"

#include <MtcaMappedDevice/devMap.h>
#include <boost/noncopyable.hpp>

#include "MotorControlerConfig.h"
#include "SignedIntConverter.h"
#include "SPIviaPCIe.h"
#include "TMC429SPI.h"

#define MCI_DECLARE_SET_GET_VALUE( NAME, VARIABLE_IN_UNITS )\
  void set ## NAME (unsigned int VARIABLE_IN_UNITS );	\
  unsigned int get ## NAME ()

#define MCI_DECLARE_SIGNED_SET_GET_VALUE( NAME, VARIABLE_IN_UNITS )\
  void set ## NAME (int VARIABLE_IN_UNITS );	\
  int get ## NAME ()

#define MCI_DECLARE_SET_GET_TYPED_REGISTER( NAME, VARIABLE_NAME )\
  void set ## NAME ( NAME const & VARIABLE_NAME );\
  NAME get ## NAME ()

namespace mtca4u
{
  class MotorDriverCardImpl;

  class MotorControlerImpl: public MotorControlerExpert, public boost::noncopyable{

  public:
    /** The constructor gets references shared pointers which it copies internally, so the
     *  object stays valid even if the original shared pointer goes out of scope.
     *  The config is only used in the constuctor, no reference is kept in the class.
     */
    MotorControlerImpl( unsigned int ID,
			boost::shared_ptr< devMap<devBase> > const & mappedDevice,
                        std::string const & moduleName,
			boost::shared_ptr< TMC429SPI > const & controlerSPI,
			MotorControlerConfig const & motorControlerConfig );

    unsigned int getID();
    int getActualPosition();
    int getActualVelocity();
    unsigned int getActualAcceleration();
    unsigned int getMicroStepCount();
    unsigned int getStallGuardValue();
    unsigned int getCoolStepValue();
    DriverStatusData getStatus();
    unsigned int getDecoderReadoutMode();
    unsigned int getDecoderPosition();
    
    void setActualVelocity(int stepsPerFIXME);
    void setActualAcceleration(unsigned int stepsPerSquareFIXME);
    void setMicroStepCount(unsigned int microStepCount);
    void setEnabled(bool enable=true);
    void setDecoderReadoutMode(unsigned int decoderReadoutMode);

    bool isEnabled();
    
    MotorReferenceSwitchData getReferenceSwitchData();

    void setPositiveReferenceSwitchEnabled(bool enableStatus);
    void setNegativeReferenceSwitchEnabled(bool enableStatus);

    void setActualPosition(int steps);
    MCI_DECLARE_SIGNED_SET_GET_VALUE( TargetPosition, steps );
    MCI_DECLARE_SET_GET_VALUE( MinimumVelocity, stepsPerFIXME );
    MCI_DECLARE_SET_GET_VALUE( MaximumVelocity, stepsPerFIXME );
    MCI_DECLARE_SIGNED_SET_GET_VALUE( TargetVelocity, stepsPerFIXME );
    MCI_DECLARE_SET_GET_VALUE( MaximumAcceleration, stepsPerSquareFIXME );
    MCI_DECLARE_SET_GET_VALUE( PositionTolerance, steps );
    MCI_DECLARE_SET_GET_VALUE( PositionLatched, steps );

    MCI_DECLARE_SET_GET_TYPED_REGISTER(AccelerationThresholdData,
				       accelerationThresholdData);
    MCI_DECLARE_SET_GET_TYPED_REGISTER(ProportionalityFactorData,
				       proportionalityFactorData);
    MCI_DECLARE_SET_GET_TYPED_REGISTER(ReferenceConfigAndRampModeData,
				       referenceConfigAndRampModeData);
    MCI_DECLARE_SET_GET_TYPED_REGISTER(InterruptData, interruptData);
    MCI_DECLARE_SET_GET_TYPED_REGISTER(DividersAndMicroStepResolutionData, 
				       dividersAndMicroStepResolutionData);

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
 
     bool targetPositionReached();

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

#endif // MTCA4U_MOTOR_CONTROLER_IMPL_H
