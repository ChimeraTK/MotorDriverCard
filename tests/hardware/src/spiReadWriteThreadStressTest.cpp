#include "MotorDriverCard.h"
#include "MotorDriverCardFactory.h"
#include "MotorControler.h"
#include "TMC429DummyConstants.h"

#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

using namespace mtca4u;

class SpiReadWriteThreadStressTest{
public:
  SpiReadWriteThreadStressTest( std::string deviceFileName, std::string mapFileName,
				std::string motorConfigFileName );
  void run();

private:  
  void preTests();
  void controllerSpiReadWriteTestLoop(unsigned int motorID, unsigned int nLoopsMax = 0);

  void testControllerSpiTwoThreads();
  void printStatus();
  
  std::string _deviceFileName;
  std::string _mapFileName;
  std::string _motorConfigFileName;
  
  unsigned int _nCycles;

  volatile unsigned int _errorsInControllerSpiTwoThreads;
  volatile bool _quitCurrentTest;
};

SpiReadWriteThreadStressTest::SpiReadWriteThreadStressTest( std::string deviceFileName,
							    std::string mapFileName,
							    std::string motorConfigFileName )
  : _deviceFileName(deviceFileName), _mapFileName(mapFileName),
    _motorConfigFileName(motorConfigFileName),
    _nCycles(0),
    _errorsInControllerSpiTwoThreads(0),
    _quitCurrentTest(false){
}

void SpiReadWriteThreadStressTest::preTests(){
  // the that the version without multi-threading works
  std::cout << "Running single-threaded tests..." << std::endl;
  controllerSpiReadWriteTestLoop(0, 10);
  std::cout << "Controller read/write  on motor 0 finished, " << _errorsInControllerSpiTwoThreads
	    << " errors until now."<< std::endl;
  _quitCurrentTest=false;
  controllerSpiReadWriteTestLoop(1, 10);
  std::cout << "Controller read/write  on motor 1 finished, " << _errorsInControllerSpiTwoThreads
	    << " errors until now."<< std::endl;
}

void SpiReadWriteThreadStressTest::run(){
  preTests();

  while (true){
    testControllerSpiTwoThreads();
    printStatus();
    // for development only
    if (_nCycles==10) break;
  }// while true
}

void SpiReadWriteThreadStressTest::testControllerSpiTwoThreads(){

  _quitCurrentTest=false;

  // run one instance of in a separate thread...
  boost::thread motor0Thread( boost::bind( &SpiReadWriteThreadStressTest::controllerSpiReadWriteTestLoop, boost::ref(*this), 0, 10000) );
  
  controllerSpiReadWriteTestLoop(1);
  
  motor0Thread.join();
}

void SpiReadWriteThreadStressTest::controllerSpiReadWriteTestLoop(unsigned int motorID,
								  unsigned int nLoopsMax){
  boost::shared_ptr<MotorDriverCard> motorDriverCard 
    = MotorDriverCardFactory::instance().createMotorDriverCard( _deviceFileName,
								_mapFileName,
								_motorConfigFileName );

   boost::shared_ptr<MotorControler> motorControler
     = motorDriverCard->getMotorControler(motorID);

   //   std::cout << "controllerSpiReadWriteTestLoop starting for motor " << motorID << std::endl;

   for (unsigned int i=0; ( (i < nLoopsMax) || (nLoopsMax==0) ); ++i ){
     //          std::cout << "Motor " << motorID << " setting target position to "<< (i & tmc429::SPI_DATA_MASK) << std::endl;
     motorControler->setTargetPosition(i & tmc429::SPI_DATA_MASK);
     if (  (motorControler->getTargetPosition() & tmc429::SPI_DATA_MASK) 
	   !=  (i & tmc429::SPI_DATA_MASK) ){
       std::cout << "Error in controllerSpiReadWriteTestLoop of FIXME " 
		 << std::endl;
       ++_errorsInControllerSpiTwoThreads;
     }
     //     std::cout << "quitCurrentTest = " <<(_quitCurrentTest?"true":"false")  << std::endl;
     if (_quitCurrentTest){
       //       std::cout << "Motor " << motorID << " detected _quitCurrentTest=true, breaking loop." << std::endl;
       break;
     }
  }
  
   //   std::cout << "Motor " << motorID << " loop done." << std::endl;
   _quitCurrentTest=true;
   //   std::cout << "quitCurrentTest = " <<(_quitCurrentTest?"true":"false")  << std::endl;
   //   std::cout << "_quitCurrentTest set true" << std::endl;
}

void SpiReadWriteThreadStressTest::printStatus(){
  std::cout << ++_nCycles << " cycle" ;
  std::cout << ( _nCycles==1 ? "" : "s ") 
	    << " finished, " << _errorsInControllerSpiTwoThreads
	    << " errors detected." << std::endl;;
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
