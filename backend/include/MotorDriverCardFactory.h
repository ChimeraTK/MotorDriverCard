#ifndef MTCA4U_MOTOR_DRIVER_CARD_FACTORY_H
#define MTCA4U_MOTOR_DRIVER_CARD_FACTORY_H

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <string>

namespace mtca4u {

  class MotorDriverCard;

  /** The MotorDriverCardFactory is used to create instances
   *  of the MotorDriverCard class. It assures that each device is only
   *  opened once and all requesting threads get the same instance
   *  of MotorDriverClass which access the same device. Like this
   *  thread safety for the SPI communication to the drivers is ensured.
   *
   *  If deviceFileName and mapFileName are the same (both have to
   *  be the map file name) a DFMC_MD22 is used in place of a real device.
   *
   *  If deviceFileName is '/dummy/MotorDriverCard' a MotorDriverCardDummy is
   * created. As ususal there will be only one instance also for this special
   * device name.
   */
  class MotorDriverCardFactory {
    /** The constructor is private because this class is a singleton.
     */
    MotorDriverCardFactory();
    MotorDriverCardFactory(MotorDriverCardFactory const&) = delete;

    /// Mutex to allow access from multiple threads.
    boost::mutex _factoryMutex;

    /// Map of all devices which have ever been requested.
    std::map<std::pair<std::string, std::string>, boost::weak_ptr<MotorDriverCard>> _motorDriverCards;
    bool _dummyMode;

   public:
    /** Access the single instance of the factory.
     */
    static MotorDriverCardFactory& instance();
    bool getDummyMode();
    void setDummyMode(bool dummyMode = true);
    /** Create a motor driver card from the device alias, the module name in the
     * map file (there mighte be more than one MD22 on the carrier), and the file
     * name for the motor config.
     */
    boost::shared_ptr<MotorDriverCard> createMotorDriverCard(
        std::string alias, std::string mapModuleName, std::string motorConfigFileName);

    /** Set the application wide dmap file to use.
     *  \attention This static function is setting the dmap file for ALL devices
     * of the deviceaccess library, not only for the MotorDriverCard. The
     * DMapFilePath is a global setting for the whole application!
     *
     *  This function has been introduced to allow using the motor driver card
     *  without having to include any header of the underlying deviceaccess
     * library in case the application is not using it elsewhere. If your
     * application has already set the DMap file path you will not need this
     * function.
     *
     *
     *  This function is not thread safe.
     *
     *  @param dmapFileName The (absolute or relative) path to the DMap file,
     * incl. the file name.
     */
    static void setDeviceaccessDMapFilePath(std::string dmapFileName);

    /** Get the dmap file name that is currently set for mtca4u::deviceaccess.
     *  This is a global setting for the whole application.
     *
     *  This function has been introduced to allow using the motor driver card
     *  without having to include any header of the underlying deviceaccess
     * library in case the application is not using it elsewhere.
     */
    static std::string getDeviceaccessDMapFilePath();
  };
} // namespace mtca4u

#endif // MTCA4U_MOTOR_DRIVER_CARD_FACTORY_H
