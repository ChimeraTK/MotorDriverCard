#ifndef MTCA4U_MOTOR_DRIVER_CARD_DUMMY_H
#define MTCA4U_MOTOR_DRIVER_CARD_DUMMY_H

#include "MotorDriverCard.h"

#include <vector>

namespace mtca4u
{
  /**
   * A dummy implementation of the MotorDriverCard. It 
   * provides MotorControlerDummys and a PowerMonitorDummy.
   */
  class MotorDriverCardDummy : public MotorDriverCard{
  public:
    virtual boost::shared_ptr<MotorControler> getMotorControler(unsigned int motorControlerID);
    
    virtual PowerMonitor & getPowerMonitor();
    virtual ~MotorDriverCardDummy();

  private:
    // The constructor does not need to know anything. There is noting to configure
    // in the dummy, and it does not access any hardware (or hardware dummy).
    MotorDriverCardDummy();
    
    // The factory needs access to the constructor because it is the only official ways
    // to create a MotorDriverCard
    friend class MotorDriverCardFactory;

    // For testing we need access to the constructor
    friend class MotorDriverCardDummyTest;

    std::vector< boost::shared_ptr<MotorControler> > _motorControlers;

  };
  
}// namespace mtca4u

#endif //MTCA4U_MOTOR_DRIVER_CARD_DUMMY_H
