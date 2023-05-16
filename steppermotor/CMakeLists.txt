cmake_minimum_required(VERSION 3.16)

set(HEADERS StepperMotor.h BasicStepperMotor.h LinearStepperMotor.h)

foreach(HEADER ${HEADERS})
  set(CTK_HEADERS ${CTK_HEADERS} include/${HEADER})
  set(SOURCES ${SOURCES} include/${HEADER})
endforeach()
install(DIRECTORY include/ DESTINATION include/ChimeraTK/MotorDriverCard)

set(SRC BasicStepperMotor.cc LinearStepperMotor.cc  StepperMotorStateMachine.cc LinearStepperMotorStateMachine.cc StepperMotorFactory.cc)
foreach(SOURCE ${SRC})
  set(SOURCES ${SOURCES} src/${SOURCE})
endforeach()

add_library(CTK_OBJECTS OBJECT ${SOURCES})
target_include_directories(CTK_OBJECTS PRIVATE include ${COMMON_INCLUDE_DIRS})
set_property(TARGET CTK_OBJECTS PROPERTY POSITION_INDEPENDENT_CODE ON)
add_library(CTK_OBJECTS_TARGET INTERFACE)
target_sources(CTK_OBJECTS_TARGET INTERFACE $<TARGET_OBJECTS:CTK_OBJECTS>)
