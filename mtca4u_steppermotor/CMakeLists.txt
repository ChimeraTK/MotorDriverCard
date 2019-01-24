
set(HEADERS StepperMotor.h LinearStepperMotor.h StepperMotorError.h StepperMotorStatus.h StepperMotorCalibrationStatus.h)
foreach(HEADER ${HEADERS})
  set(MTCA4U_STEPPERMOTOR_HEADERS ${MTCA4U_STEPPERMOTOR_HEADERS} mtca4u_steppermotor/include/${HEADER})
endforeach()
set(MTCA4U_STEPPERMOTOR_HEADERS ${MTCA4U_STEPPERMOTOR_HEADERS} PARENT_SCOPE)

set(SRC StepperMotor.cc LinearStepperMotor.cc  StepperMotorStatus.cc StepperMotorError.cc StepperMotorCalibrationStatus.cc)
foreach(SOURCE ${SRC})
  set(MTCA4U_STEPPERMOTOR_SOURCES ${MTCA4U_STEPPERMOTOR_SOURCES} mtca4u_steppermotor/src/${SOURCE})
endforeach()
set(MTCA4U_STEPPERMOTOR_SOURCES ${MTCA4U_STEPPERMOTOR_SOURCES} PARENT_SCOPE)

