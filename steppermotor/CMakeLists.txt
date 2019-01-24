
set(HEADERS StepperMotor.h BasicStepperMotor.h StepperMotorStateMachine.h StepperMotorWithReference.h StepperMotorWithReferenceStateMachine.h)
foreach(HEADER ${HEADERS})
  set(STEPPERMOTOR_HEADERS ${STEPPERMOTOR_HEADERS} steppermotor/include/${HEADER})
endforeach()
set(STEPPERMOTOR_HEADERS ${STEPPERMOTOR_HEADERS}  PARENT_SCOPE)

#set(SRC BasicStepperMotor.cc StepperMotorWithReference.cc  StepperMotorStateMachine.cc StepperMotorWithReferenceStateMachine.cc StepperMotorFactory.cc)
set(SRC BasicStepperMotor StepperMotorWithReference StepperMotorStateMachine StepperMotorWithReferenceStateMachine StepperMotorFactory)

foreach(SOURCE ${SRC})
  set(STEPPERMOTOR_SOURCES ${STEPPERMOTOR_SOURCES} steppermotor/src/${SOURCE})
  add_library(obj${SOURCE} OBJECT src/${SOURCE}.cc ${STEPPERMOTOR_HEADERS})
endforeach()
set(STEPPERMOTOR_SOURCES ${STEPPERMOTOR_SOURCES}  PARENT_SCOPE)
