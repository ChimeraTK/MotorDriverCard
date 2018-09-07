#ifndef MTCA4U_MOTOR_DRIVER_CARD_IMPL_H
#define MTCA4U_MOTOR_DRIVER_CARD_IMPL_H

#include <boost/shared_ptr.hpp>

#include "MotorDriverCardExpert.h"
#include "MotorControler.h"
#include "MotorDriverCardConfig.h"
#include "PowerMonitor.h"
#include "TMC429Words.h"
#include "TMC429SPI.h"

namespace mtca4u
{
  /**
   * The implementation of the DFMC-MD22 motor driver card.
   * It has a private constructor and can only be created using the
   * MotorDriverCardFactory. This ensures that only one instance per device is created,
   * which allows thread safe access to the motors.
   */
  class MotorDriverCardImpl: public MotorDriverCardExpert{
  public:

    boost::shared_ptr<MotorControler> getMotorControler(unsigned int motorControlerID);    
    /// Get a reference to the power monitor.
    PowerMonitor & getPowerMonitor();

    unsigned int getControlerChipVersion();

    void setDatagramLowWord(unsigned int datagramLowWord);
    unsigned int getDatagramLowWord();

    void setDatagramHighWord(unsigned int datagramHighWord);
    unsigned int getDatagramHighWord();

    void setCoverPositionAndLength(
                  CoverPositionAndLength const & coverPositionAndLength);
    CoverPositionAndLength getCoverPositionAndLength();

    void setCoverDatagram(unsigned int coverDatagram);
    unsigned int getCoverDatagram();

    void setStepperMotorGlobalParameters(
		  StepperMotorGlobalParameters const & stepperMotorGlobalParameters);
    StepperMotorGlobalParameters getStepperMotorGlobalParameters();

    void setInterfaceConfiguration(
                  InterfaceConfiguration const & interfaceConfiguration);
    InterfaceConfiguration getInterfaceConfiguration();

    /// Very expert function only available in the implementation. Usually
    /// the programmer should not have to touch this.
    void setPositionCompareWord(unsigned int positionCompareWord); //??? data sheet is completely unclear
    unsigned int getPositionCompareWord(); //??? data sheet is completely unclear

    /// Very expert function only avaialble in the implementation. Usually
    /// the programmer should not have to touch this.
    void setPositionCompareInterruptData(
		  PositionCompareInterruptData const & positionCompareInterruptData);
    PositionCompareInterruptData getPositionCompareInterruptData();

    void powerDown();
    ReferenceSwitchData getReferenceSwitchData();

    /** Get the 8 bit status word which is always send a the most significant byte of the TMC429 readback word.*/
    TMC429StatusWord getStatusWord();

  private:
    // Motor controlers need dynamic allocation, so we cannot store them directly.
    // As we do not want to care about cleaning up we use shared pointers.
    std::vector< boost::shared_ptr<MotorControler> > _motorControlers;

    boost::shared_ptr< mtca4u::Device > _device;

    boost::shared_ptr<PowerMonitor> _powerMonitor;

    boost::shared_ptr<TMC429SPI> _controlerSPI;

    boost::shared_ptr< mtca4u::RegisterAccessor > _controlerStatusRegister;
    
    std::string _moduleName;

    /// Checks the firmware version and throws an exception if it is not valid (too old or too new)
    void checkFirmwareVersion();

    /// The constructor requires a working version of MtcaDevice in addition
    /// to the configuration. These are provided by the factory. The constructor is
    /// private, so the class can only be generated by the factory.
    MotorDriverCardImpl(boost::shared_ptr< mtca4u::Device > const & device,
                        std::string const & moduleName,			
                        MotorDriverCardConfig const & cardConfiguration);

    /// The class is non-copyable
    MotorDriverCardImpl(MotorDriverCardImpl const &)=delete;
    /// The class is non-assignable
    MotorDriverCardImpl & operator=(MotorDriverCardImpl const &)=delete;

    // The factory needs access to the constructor because it is the only official ways
    // to create a MotorDriverCard
    friend class MotorDriverCardFactory;
   
 // The test suite is granted access to the constructor because it needs access
    // to the dummy device below to set it to testing mode.
    friend class MotorControlerTestSuite;
    friend class MotorControlerTest;

    // The card test tests the constructor, so also here we have to make it friend
    friend class MotorDriverCardTest;
  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_IMPL_H
