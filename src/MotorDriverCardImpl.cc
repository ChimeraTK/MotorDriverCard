#include "MotorDriverCard.h"

namespace mtca4u{

  MotorDriverCard::MotorDriverCard(boost::shared_ptr<MtcaMappedDevice> & mappedDevice,
				   MotorDriverConfiguration & motorDriverConfiguration){
    // Fill the register constants from the mappedDevice
    SPIControlWriteAddress = mappedDevice.getAddress(SPIControlWriteAddressString);
    SPIControlReadbackAddress = mappedDevice.getAddress(SPIControlReadbackAddressString);
    
    // send the configuration
    
    
  }

  MotorControler & MotorDriverCard::getMotorControler(unsigned int motorControlerID){
    if (motorControlerID > 2) throw MotorDriverException("motorControlerID too large");
    
    return *(_motorControlers[motorControlerID]);
  }

  PowerMonitor & MotorDriverCard::getPowerMonitor(){
    return *_powerMonitor;
  }

  unsigned int MotorDriverCard::getControlerChipVersion(){
    return spiRead( SMDA_COMMON, JDX_CHIP_VERSION ).getDATA();
  }

  TMC429Register MotorDriverCard::spiRead( unsigned int smda, unsigned int idx_jdx )
  {
    // Although the first half is almost identical to the spiWrite,
    // the preparation of the TMC429Register is different. So we accept a little 
    // code duplication here.
    TMC429Register readRequest;
    readRequest.setSMDA(smda);
    readRequest.setIDX_JDX(idx_jdx);
    readRequest.setRW(READ);

    _psiControlWriteMarshaller.writeRaw(readRequest.dataWord());
    //FIXME: synchronise so readback value is valid
    unsigned int readbackValue;
    _psiControlReadbackMarshaller.readRaw( & readbackValue );

    return TMC429Register(readbackValue);
  }

  void MotorDriverCard::spiWrite( unsigned int smda, unsigned int idx_jdx, 
				  unsigned int data );
  {
    TMC429Register writeMe;
    readRequest.setSMDA(smda);
    readRequest.setIDX_JDX(idx_jdx);
    readRequest.setRW(WRITE);
    readRequest.setDATA(data);

    _psiControlWriteMarshaller.writeRaw( writeMe.dataWord());
  }

}// namespace mtca4u
