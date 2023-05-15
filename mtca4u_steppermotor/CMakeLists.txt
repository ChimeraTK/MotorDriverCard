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
target_include_directories(MTCA4U_OBJECTS PRIVATE include ${COMMON_INCLUDE_DIRS})
set_property(TARGET MTCA4U_OBJECTS PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(MTCA4U_OBJECTS PRIVATE PkgConfig::LibXML++)
add_library(MTCA4U_OBJECTS_TARGET INTERFACE)
target_sources(MTCA4U_OBJECTS_TARGET INTERFACE $<TARGET_OBJECTS:MTCA4U_OBJECTS>)
