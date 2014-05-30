#ifndef MTCA4U_MOTOR_DRIVER_CARD_FACTORY_H
#define MTCA4U_MOTOR_DRIVER_CARD_FACTORY_H

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

namespace mtca4u{

  class MotorDriverCard;

/** The MotorDriverCardFactory is used to create instances
 *  of the MotorDriverCard class. It assures that each device is only
 *  opened once and all requesting threads get the same instance
 *  of MotorDriverClass which access the same device. Like this
 *  thread safety for the SPI communication to the drivers is ensured.
 *
 *  If deviceFileName and mapFileName are the same (both have to
 *  be the map file name) a DFMC_MD22 is used in place of a real device.
 */
class MotorDriverCardFactory{
  /** The constructor is private because this class is a singleton.
   */
  MotorDriverCardFactory();
  MotorDriverCardFactory(MotorDriverCardFactory const &);
  
  /// Mutex to allow access from multiple threads.
  boost::mutex _factoryMutex;

  /// Map of all devices which have ever been requested.
  std::map<std::string, boost::shared_ptr<MotorDriverCard> > _motorDriverCards;

 public:
  /** Access the single instance of the factory.
   */
  static MotorDriverCardFactory & instance();

  /** Create a motor driver card from the device and map file name.
   */
  boost::shared_ptr<MotorDriverCard> createMotorDriverCard(
     std::string deviceFileName, std::string mapFileName,
     std::string motorConfigFileName);
};

}// namespace mtca4u

#endif// MTCA4U_MOTOR_DRIVER_CARD_FACTORY_H
