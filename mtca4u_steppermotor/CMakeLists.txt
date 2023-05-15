cmake_minimum_required(VERSION 2.8.0)

set(HEADERS StepperMotor.h LinearStepperMotor.h StepperMotorError.h StepperMotorStatus.h StepperMotorCalibrationStatus.h)
foreach(HEADER ${HEADERS})
  set(MTCA4U_HEADERS ${MTCA4U_HEADERS} include/${HEADER})
endforeach()
install(DIRECTORY include/ DESTINATION include/mtca4u/MotorDriverCard)

set(SRC StepperMotor.cc LinearStepperMotor.cc  StepperMotorStatus.cc StepperMotorError.cc StepperMotorCalibrationStatus.cc)
foreach(SOURCE ${SRC})
  set(SOURCES ${SOURCES} src/${SOURCE})
endforeach()

add_library(MTCA4U_OBJECTS OBJECT ${SOURCES})
target_include_directories(MTCA4U_OBJECTS PUBLIC include ${COMMON_INCLUDE_DIRS})
set_property(TARGET MTCA4U_OBJECTS PROPERTY POSITION_INDEPENDENT_CODE ON)
