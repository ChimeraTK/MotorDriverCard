
set(HEADERS StepperMotor.h BasicStepperMotor.h StepperMotorWithReference.h)
foreach(HEADER ${HEADERS})
  set(CTK_HEADERS ${CTK_HEADERS} include/${HEADER})
endforeach()
install(DIRECTORY include/ DESTINATION include/ChimeraTK/MotorDriverCard)

set(SRC BasicStepperMotor.cc StepperMotorWithReference.cc  StepperMotorStateMachine.cc StepperMotorWithReferenceStateMachine.cc StepperMotorFactory.cc)
foreach(SOURCE ${SRC})
  set(SOURCES ${SOURCES} src/${SOURCE})
endforeach()

add_library(CTK_OBJECTS OBJECT ${SOURCES})
target_include_directories(CTK_OBJECTS PUBLIC include ${COMMON_INCLUDE_DIRS})
