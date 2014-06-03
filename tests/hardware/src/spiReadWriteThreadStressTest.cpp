#include "MotorDriverCard.h"
#include "MotorDriverCardFactory.h"
#include "MotorControler.h"
#include "TMC429DummyConstants.h"

#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

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
 *     The loop can be configured to have a delay between each change.
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
 *    - Loop 5 with delay and loop 6 for motor 0
 *    - Loop 5 with delay and loop 6 for motor 1
 *    - Loop 5 without delay and loop 6 for motor 0
 *    - Loop 5 without delay and loop 6 for motor 1
 *  -# Combined test
 *  -# All loops (loops 1, 5 and 6 for both motors. Loop 5 with delay for motor 0,
 *     without delay for motor 1)
 */ 

class SpiReadWriteThreadStressTest{
public:
  SpiReadWriteThreadStressTest( std::string deviceFileName, std::string mapFileName,
				std::string motorConfigFileName );
  void run();

private:  
  // the components of the test block: Loops which run in individual threads

  /// The base loop is always the same in order to avoid code duplication.
  /// If nLoopsMax is set to 0 the loop will run infinitely until another thread
  /// sets the quitCurrentTest flag to true.
  /// The core of the loop is given as a function pointer.
  /// This function is a member function of this class, returns void and
  ///  has references to shared pointers as arguments.
  /// The first argument is the loop index
  void baseLoop(unsigned int motorID,
		unsigned int nLoopsMax,
		void (SpiReadWriteThreadStressTest::* loopCore)(
				      unsigned int,
				      boost::shared_ptr<MotorDriverCard> &,
				      boost::shared_ptr<MotorControler> & ) );

  /// write, read and check the results for the TMC249 controller chip
  void controllerSpiWriteReadCheck(unsigned int loopIndex,
				   boost::shared_ptr<MotorDriverCard> & motorDriverCard,
				   boost::shared_ptr<MotorControler>  & motorControler );

  void preTests();
  void testControllerSpiTwoThreads();
  void printStatus();
  
  std::string _deviceFileName;
  std::string _mapFileName;
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

SpiReadWriteThreadStressTest::SpiReadWriteThreadStressTest( std::string deviceFileName,
							    std::string mapFileName,
							    std::string motorConfigFileName )
  : _deviceFileName(deviceFileName), _mapFileName(mapFileName),
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
  baseLoop(0, 1000, &SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck);
  printStatus();

  _currentTestName = "Controller read/write  on motor 1";
  // no need to lock the mutex, not multi-threaded here
  _errorsInThisTest = 0;
  baseLoop(1, 1000, &SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck);
  printStatus();
}

void SpiReadWriteThreadStressTest::run(){
  preTests();

  while (true){
    std::cout << "Starting cycle " << _nCycles << std::endl;
    _errorsInThisCycle = 0;
  
    testControllerSpiTwoThreads();

    std::cout << "\nCycle " << _nCycles++ << " finished with "
	      << _errorsInThisCycle << " errors, " << _errorsTotal
	      << " errors in total up to now."<< std::endl;

    // for development only
   if (_nCycles==10) break;
  }// while true
}

void SpiReadWriteThreadStressTest::testControllerSpiTwoThreads(){

  _quitCurrentTest=false;
  _currentTestName = "Controller read/write with two threads";
  _errorsInThisTest = 0;

  // run one instance of in a separate thread...
  boost::thread motor0Thread( boost::bind( &SpiReadWriteThreadStressTest::baseLoop, boost::ref(*this), 0, 10000, &SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck) );
  
  // run the second instance in this thread
  baseLoop(1, 0, &SpiReadWriteThreadStressTest::controllerSpiWriteReadCheck);
  
  // this should return immediately because the separate thread triggers the end of
  // the loop in this thread, so at this point both loops are finished
  motor0Thread.join();
  
  // printing the status must only be done after all threads have returned.
  printStatus();
}

void SpiReadWriteThreadStressTest::baseLoop(unsigned int motorID,
       unsigned int nLoopsMax,
       void (SpiReadWriteThreadStressTest::* loopCore)(
		     unsigned int,
		     boost::shared_ptr<MotorDriverCard> &,
		     boost::shared_ptr<MotorControler> & ) ){

  boost::shared_ptr<MotorDriverCard> motorDriverCard 
    = MotorDriverCardFactory::instance().createMotorDriverCard( _deviceFileName,
								_mapFileName,
								_motorConfigFileName );
   boost::shared_ptr<MotorControler> motorControler
     = motorDriverCard->getMotorControler(motorID);

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
		boost::shared_ptr<MotorDriverCard> & motorDriverCard,
		boost::shared_ptr<MotorControler>  & motorControler ){

     motorControler->setTargetPosition(loopIndex & tmc429::SPI_DATA_MASK);
     if (  (motorControler->getTargetPosition() & tmc429::SPI_DATA_MASK) 
	   !=  (loopIndex & tmc429::SPI_DATA_MASK) ){
       increaseErrorCount("controllerSpiWriteReadCheck");
     }
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
  if (argc!=4){
    std::cout << "Usage: " << argv[0] << " deviceFileName mapFileName" 
	      << " motorConfigFileName" << std::endl;
    return -1;
  }
  
  SpiReadWriteThreadStressTest stressTest(argv[1], argv[2], argv[3]);

  stressTest.run();

  return 0;
}
