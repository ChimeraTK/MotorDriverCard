
set(HEADERS StepperMotor.h BasicStepperMotor.h StepperMotorStateMachine.h StepperMotorWithReference.h StepperMotorWithReferenceStateMachine.h)
foreach(HEADER ${HEADERS})
  set(CTK_HEADERS ${CTK_HEADERS} include/${HEADER})
endforeach()
#set(CTK_STEPPERMOTOR_HEADERS ${STEPPERMOTOR_HEADERS}  PARENT_SCOPE)
install(DIRECTORY include DESTINATION include/ChimeraTK/MotorDriverCard/)


set(SRC BasicStepperMotor.cc StepperMotorWithReference.cc  StepperMotorStateMachine.cc StepperMotorWithReferenceStateMachine.cc StepperMotorFactory.cc)
foreach(SOURCE ${SRC})
#  get_filename_component(SOURCE_NAME ${SOURCE} NAME_WE)
#  add_library(${SOURCE_NAME} OBJECT src/${SOURCE})
#  set(OBJECTS ${OBJECTS} ${SOURCE_NAME})
  set(SOURCES ${SOURCES} src/${SOURCE})
#  target_include_directories(${SOURCE_NAME} PUBLIC include ${COMMON_HEADERS})
endforeach()


add_library(CTK_OBJECTS OBJECT ${SOURCES})# ${CTK_HEADERS})
target_include_directories(CTK_OBJECTS PUBLIC include ${COMMON_HEADER_DIRS})
#set(CTK_OBJECTS ${OBJECTS} PARENT_SCOPE)
