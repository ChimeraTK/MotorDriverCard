#include "MotorDriverCard.h"
#include "MotorDriverCardImpl.h"
#include "MotorDriverCardFactory.h"
#include "MotorControlerExpert.h"
#include "TMC429DummyConstants.h"

#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <mtca4u/BackendFactory.h>

using namespace mtca4u;

/** The SpiReadWriteThreadStressTest performs cycles of spi communication tests, each
 *  running a sequence of different combinations of read/write access to the
 *  TMC429 controller chip or the TMC260 driver chip.
 *
 *  The building blocks are the following loops/threads:
 *  -# Write/readback/check the target position regsiter of TMC429 (motor 0 or 1)
 *  -# Write to the TMC429 PositionCompareInterrupt register of motor 0 as fast
 *     as possible to cause possible coflicts in other threads.
 *     Does not check if writing is successful.
 *  -# Read the actual position of motor 1 as fast as possible to TMC429.
 *  -# Read theTMC429 reference switch register from the global section 
 *     as fast as possible.
 *  -# Alternatingly write 0x1F and 0x7 to the current scaling sub-word of the
 *     StallGuardControl register, and check for consistency in the answer with
 *     RDSEL=0x2 (read selection word of the CoolStepControl register), for motor 0/1
 *     The loop core can be configured to have a delay between each change (even 
 *     index is without delay, uneven index with delay).
 *  -# Alternatingly write 0xF and 0x0 to the MRES (micro step resolution) word
 *     in the DriverControl register as fast as possible to test interference 
 *     with loop 5
 *
 *  There are different test blocks which are run:
 *  -# Single treaded pre-tests (only run one at the beginning)
 *    - Loop 1 for motor 0 and 1 one a after the other
 *    - Loop 5 for motor 0 and 1 without delay one after the other
 *  -# TMC429 tests
 *    - Loop 1 for motor 0 and 1 in parallel
 *    - Loop 1 for motor 0 and 1, plus loops 2,3, and 4 in parallel
 *  -# TMC260 tests
 *    - Loops 5 plus 6 for motor 0
 *    - Loops 5 plus 6 for motor 1
 *  -# Combined test
 *  -# All loops (loops 1, 5 and 6 for both motors).
 */ 

class SpiReadWriteThreadStressTest{
public:
  SpiReadWriteThreadStressTest( std::string deviceAlias, std::string moduleName,
				std::string motorConfigFileName );
  void run();

private:  
  // the components of the test block: Loops which run in individual threads

  /// The type of function pointer used as loop core (see baseLoop).
  /// The function is a member of this class, returns void and
  ///  has references to shared pointers as arguments.
  /// The first argument is the loop index.
  typedef void (SpiReadWriteThreadStressTest::* CoreFunctionPointer)(
				      unsigned int,
				      boost::shared_ptr<MotorDriverCard> &,
				      boost::shared_ptr<MotorControlerExpert> & );

  /// The base loop is always the same in order to avoid code duplication.
  /// If nLoopsMax is set to 0 the loop will run infinitely until another thread
  /// sets the quitCurrentTest flag to true.
  /// The core of the loop is given as a function pointer.
  void baseLoop(unsigned int motorID,
		unsigned int nLoopsMax,
		CoreFunctionPointer loopCore);

  /// write, read and check the results for the TMC249 controller chip
  void controllerSpiWriteReadCheck(unsigned int loopIndex,
				   boost::shared_ptr<MotorDriverCard> & motorDriverCard,
				   boost::shared_ptr<MotorControlerExpert>  & motorControler );

  void writeInterruptRegister(unsigned int loopIndex,
			      boost::shared_ptr<MotorDriverCard> & motorDriverCard,
			      boost::shared_ptr<MotorControlerExpert>  & motorControler );

  void readTargetPosition(unsigned int loopIndex,
			  boost::shared_ptr<MotorDriverCard> & motorDriverCard,
			  boost::shared_ptr<MotorControlerExpert>  & motorControler );

  void readReferenceSwitchData(unsigned int loopIndex,
			       boost::shared_ptr<MotorDriverCard> & motorDriverCard,
			       boost::shared_ptr<MotorControlerExpert>  & motorControler );

  void writeCheckStallGuard(unsigned int loopIndex,
			   boost::shared_ptr<MotorDriverCard> & motorDriverCard,
			   boost::shared_ptr<MotorControlerExpert>  & motorControler );

  void writeDriverControl(unsigned int loopIndex,
			  boost::shared_ptr<MotorDriverCard> & motorDriverCard,
			  boost::shared_ptr<MotorControlerExpert>  & motorControler );

  void preTests();
  void testControllerSpiTwoThreads();
  void testControllerSpiFiveThreads();
  void testDriverSpi(unsigned int motorID);
  void testAllThreads();
  void printStatus();
  
  std::string _deviceAlias;
  std::string _moduleName;
  std::string _motorConfigFileName;
  
  unsigned int _nCycles;

  // The following block of variables is accessed from multiple threads and protected
  // by a mutex. Don't access them without locking the mutex!
  // The counters are volatile because they are changed by the threads. The 
  // string is not modified from the threads thus does not have to be volatile 
  // FIXME: The current boost version does not know atomic yet. Change to atomic 
  // once Ubuntu 12.4 is abandoned and get rid of the error-prone mutex
  boost::mutex _errorCountingMutex;
  volatile unsigned int _errorsInThisTest;
  volatile unsigned int _errorsInThisCycle;
  volatile unsigned int _errorsTotal;
  std::string _currentTestName;
  
  /// The function name is given to the increaseErrorCount function for logging
  /// purposes.
  void increaseErrorCount(std::string functionName);

  // A flag to syncronise multiple threads. As it is a bool which is only read or set
  // (atomic) it is not protected by a mutex.
  // FIXME: The current boost version does not know atomic yet. Change to atomic 
  // once Ubuntu 12.4 is abandoned.
  volatile bool _quitCurrentTest;
};

SpiReadWriteThreadStressTest::SpiReadWriteThreadStressTest( std::string deviceAlias,
                                                            std::string moduleName,
							    std::string motorConfigFileName )
  : _deviceAlias(deviceAlias), _moduleName(moduleName),
    _motorConfigFileName(motorConfigFileName),
    _nCycles(0),
    _errorsInThisTest(0),
    _errorsInThisCycle(0),
    _errorsTotal(0),
    _quitCurrentTest(false){
}

void SpiReadWriteThreadStressTest::preTests(){
  // the that the version without multi-threading works
  std::cout << "Running single threaded pre-tests..." << std::endl;
  _currentTestName = "Controller read/write  on motor 0";
  baseLoop(0, 3000, &SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck);
  printStatus();

  _currentTestName = "Controller read/write  on motor 1";
  // no need to lock the mutex, not multi-threaded here
  _quitCurrentTest=false;
  _errorsInThisTest = 0;
  baseLoop(1, 3000, &SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck);
  printStatus();

  _currentTestName = "Driver read/write  on motor 0";
  _quitCurrentTest=false;
  _errorsInThisTest = 0;
  baseLoop(0, 500, &SpiReadWriteThreadStressTest::writeCheckStallGuard);
  printStatus();

  _currentTestName = "Driver read/write  on motor 1";
  _quitCurrentTest=false;
  _errorsInThisTest = 0;
  baseLoop(1, 500, &SpiReadWriteThreadStressTest::writeCheckStallGuard);
  printStatus();
}

void SpiReadWriteThreadStressTest::run(){
  preTests();

  while (true){
    std::cout << "Starting cycle " << _nCycles << std::endl;
    _errorsInThisCycle = 0;
  
    testControllerSpiTwoThreads();
    testControllerSpiFiveThreads();
    testDriverSpi(0);
    testDriverSpi(1);
    testAllThreads();

    std::cout << "\nCycle " << _nCycles++ << " finished with "
	      << _errorsInThisCycle << " errors, " << _errorsTotal
	      << " errors in total up to now."<< std::endl;

  }// while true
}

void SpiReadWriteThreadStressTest::testControllerSpiTwoThreads(){

  _quitCurrentTest=false;
  _currentTestName = "Controller read/write with two threads";
  _errorsInThisTest = 0;

  // run one instance of in a separate thread...
  boost::thread motor0Thread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 0, 1000, &SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck) );
  
  // run the second instance in this thread
  baseLoop(1, 0, &SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck);
  
  // this should return immediately because the separate thread triggers the end of
  // the loop in this thread, so at this point both loops are finished
  motor0Thread.join();
  
  // printing the status must only be done after all threads have returned.
  printStatus();
}

void SpiReadWriteThreadStressTest::testControllerSpiFiveThreads(){

  _quitCurrentTest=false;
  _currentTestName = "Controller read/write with five threads";
  _errorsInThisTest = 0;

  // run one instance of in a separate thread...
  boost::thread motor0Thread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 0, 1000, &SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck) );

  boost::thread motor1Thread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 1, 0, &SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck) );
  
  boost::thread writeThread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 0, 0, &SpiReadWriteThreadStressTest::writeInterruptRegister) );

  boost::thread readTargetThread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 1, 0, &SpiReadWriteThreadStressTest::readTargetPosition) );

  // run the final instance in this thread
  baseLoop(0, 0, &SpiReadWriteThreadStressTest::readReferenceSwitchData);
  
  // Wait until all threads have finished.
  motor0Thread.join();
  motor1Thread.join();
  writeThread.join();
  readTargetThread.join();
  
  // printing the status must only be done after all threads have returned.
  printStatus();
}

void SpiReadWriteThreadStressTest::testDriverSpi(unsigned int motorID){

  _quitCurrentTest=false;
  std::stringstream name;
  name << "Driver read/write for motor " << motorID;
  _currentTestName = name.str();
  _errorsInThisTest = 0;

  // run one instance of in a separate thread...
  boost::thread stallGuardThread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), motorID, 100, &SpiReadWriteThreadStressTest::writeCheckStallGuard) );
  
  // run the second instance in this thread
  baseLoop(motorID, 0, &SpiReadWriteThreadStressTest::writeDriverControl);
  
  // this should return immediately because the separate thread triggers the end of
  // the loop in this thread, so at this point both loops are finished
  stallGuardThread.join();
  
  // printing the status must only be done after all threads have returned.
  printStatus();
}

void SpiReadWriteThreadStressTest::testAllThreads(){

  _quitCurrentTest=false;
  _currentTestName = "Combined controller and driver read/write test";
  _errorsInThisTest = 0;

  // run one instance of in a separate thread...
  boost::thread motor0Thread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 0, 1000, &SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck) );

  boost::thread motor1Thread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 1, 0, &SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck) );
  
  boost::thread writeThread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 0, 0, &SpiReadWriteThreadStressTest::writeInterruptRegister) );

  boost::thread readTargetThread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 1, 0, &SpiReadWriteThreadStressTest::readTargetPosition) );

  boost::thread readReferenceSwitchThread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 1, 0, &SpiReadWriteThreadStressTest::readReferenceSwitchData) );
  
  boost::thread stallGuard0Thread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 0, 0, &SpiReadWriteThreadStressTest::writeCheckStallGuard) );

  boost::thread stallGuard1Thread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 1, 0, &SpiReadWriteThreadStressTest::writeCheckStallGuard) );

  boost::thread driverControl0Thread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 0, 0,  &SpiReadWriteThreadStressTest::writeDriverControl) );
 
  // run the second instance in this thread
  baseLoop(1, 0, &SpiReadWriteThreadStressTest::writeDriverControl);


  // Wait until all threads have finished.
  motor0Thread.join();
  motor1Thread.join();
  writeThread.join();
  readTargetThread.join();
  readReferenceSwitchThread.join();
  stallGuard0Thread.join();
  stallGuard1Thread.join();
  driverControl0Thread.join();

  // printing the status must only be done after all threads have returned.
  printStatus();
}


void SpiReadWriteThreadStressTest::baseLoop(unsigned int motorID,
					    unsigned int nLoopsMax,
					    CoreFunctionPointer loopCore){

  boost::shared_ptr<MotorDriverCard> motorDriverCard 
    = MotorDriverCardFactory::instance().createMotorDriverCard( _deviceAlias,
                                                                _moduleName,
								_motorConfigFileName );
  boost::shared_ptr<MotorControlerExpert> motorControler
     = boost::dynamic_pointer_cast<MotorControlerExpert>(motorDriverCard->getMotorControler(motorID));

   if (!motorControler){
     throw std::runtime_error("Could not cast to motorControlerExpert");
   }

   for (unsigned int i=0; ( (i < nLoopsMax) || (nLoopsMax==0) ); ++i ){
     (this->*loopCore)(i, motorDriverCard, motorControler);

     if (_quitCurrentTest){
       break;
     }
  }
  
   _quitCurrentTest=true;
}

void SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck(
		unsigned int loopIndex,
		boost::shared_ptr<MotorDriverCard> & /* motorDriverCard, not used */,
		boost::shared_ptr<MotorControlerExpert>  & motorControler ){

     motorControler->setTargetPosition(loopIndex & tmc429::SPI_DATA_MASK);
     if (  (motorControler->getTargetPosition() & tmc429::SPI_DATA_MASK) 
	   !=  (loopIndex & tmc429::SPI_DATA_MASK) ){
       increaseErrorCount("controllerSpiWriteReadCheck");
     }
}

void SpiReadWriteThreadStressTest::writeInterruptRegister(
		unsigned int loopIndex,
		boost::shared_ptr<MotorDriverCard> & /* motorDriverCard, not used */,
		boost::shared_ptr<MotorControlerExpert>  & motorControler ){

  // this is not meaningful data, just something to torture the communication channel
  motorControler->setInterruptData( loopIndex & 0xFFFF );
  // no error checking, fire and forget
}

void SpiReadWriteThreadStressTest::readTargetPosition(
		unsigned int /*loopIndex*/,
		boost::shared_ptr<MotorDriverCard> & /* motorDriverCard, not used */,
		boost::shared_ptr<MotorControlerExpert>  & motorControler ){

  (void) motorControler->getTargetPosition();
  // do nothing with it, just read
}

void SpiReadWriteThreadStressTest::readReferenceSwitchData(
		unsigned int /*loopIndex*/,
		boost::shared_ptr<MotorDriverCard> & motorDriverCard,
		boost::shared_ptr<MotorControlerExpert>  & /* motorControler, not used */ ){

  boost::dynamic_pointer_cast<MotorDriverCardImpl>(motorDriverCard)->getReferenceSwitchData();
  // do nothing with it, just read
}

void SpiReadWriteThreadStressTest::writeCheckStallGuard(
                unsigned int loopIndex,
		boost::shared_ptr<MotorDriverCard> & /* motorDriverCard, not used */,
		boost::shared_ptr<MotorControlerExpert>  & motorControler ){

  motorControler->setStallGuardControlData( 0x1F );
  // according to the data sheet the cool step value is always between 1/4 and 1/1 of the current scaling value
  // (which we just set as the last 5 bits of the StallGuardControlData word)
  // The value is interpreted as dataContent + 1, so 1F is 32, 0x7 is 8
  if( (motorControler-> getCoolStepValue() > 0x1F ) ||
      (motorControler-> getCoolStepValue() < 0x7 ) ){
    increaseErrorCount("writeCheckStallGuard");
  }
  if (loopIndex%2){
    boost::this_thread::sleep(boost::posix_time::milliseconds(1));
  }
  motorControler->setStallGuardControlData( 0x3 );
  if(motorControler-> getCoolStepValue() > 0x3){
    increaseErrorCount("writeCheckStallGuard");
  }
  if (loopIndex%2){
    boost::this_thread::sleep(boost::posix_time::milliseconds(1));
  }
}

void SpiReadWriteThreadStressTest::writeDriverControl(
                unsigned int /* loopIndex, not used */,
		boost::shared_ptr<MotorDriverCard> & /* motorDriverCard, not used */,
		boost::shared_ptr<MotorControlerExpert>  & motorControler ){
   motorControler->setDriverControlData( 0xF );
   motorControler->setDriverControlData( 0x0 );   
}

void SpiReadWriteThreadStressTest::printStatus(){
  // no need to lock the mutex, this function shall not be called in multi-threaded
  // contect.
  std::cout << "  Test \"" << _currentTestName << "\" finished with " 
	    << _errorsInThisTest << " errors, total errors until now: "
	    << _errorsTotal << std::endl;
}

void SpiReadWriteThreadStressTest::increaseErrorCount(std::string functionName){
  _errorCountingMutex.lock();
  ++_errorsInThisTest;
  ++_errorsInThisCycle;
  ++_errorsTotal;
  std::cout << "  - Error in loop " << functionName << " of test \""
	    << _currentTestName << "\", total errors in this test/cycle/total " 
	    << _errorsInThisTest << "/" << _errorsInThisCycle << "/" 
	    << _errorsTotal << std::endl;
  _errorCountingMutex.unlock();
}

int main(int argc, char* argv []){
  if (argc<=3){
    std::cout << "Usage: " << argv[0] << " deviceAlias moduleName" 
	      << " motorConfigFileName [dmapFile]" << std::endl;
    return -1;
  }

  if (argc==5){
    BackendFactory::getInstance().setDMapFilePath(argv[4]);
  }
  
  SpiReadWriteThreadStressTest stressTest(argv[1], argv[2], argv[3]);

  stressTest.run();

  return 0;
}
