
set(HEADERS StepperMotor.h LinearStepperMotor.h StepperMotorError.h StepperMotorStatus.h StepperMotorCalibrationStatus.h)
foreach(HEADER ${HEADERS})
  set(MTCA4U_HEADERS ${MTCA4U_HEADERS} include/${HEADER})
endforeach()
install(DIRECTORY include/ DESTINATION include/mtca4u/MotorDriverCard)
set(MTCA4U_STEPPERMOTOR_HEADERS ${MTCA4U_STEPPERMOTOR_HEADERS} PARENT_SCOPE)

set(SRC StepperMotor.cc LinearStepperMotor.cc  StepperMotorStatus.cc StepperMotorError.cc StepperMotorCalibrationStatus.cc)
foreach(SOURCE ${SRC})
  #get_filename_component(SOURCE_NAME ${SOURCE} NAME_WE)
  #add_library(${SOURCE_NAME} OBJECT src/${SOURCE})
  #set(OBJECTS ${OBJECTS} ${SOURCE_NAME})
  set(SOURCES ${SOURCES} src/${SOURCE})
endforeach()

message("  ** SOURCES: ${SOURCES}")

add_library(MTCA4U_OBJECTS OBJECT ${SOURCES})
target_include_directories(MTCA4U_OBJECTS PUBLIC include ${COMMON_INCLUDE_DIRS})
