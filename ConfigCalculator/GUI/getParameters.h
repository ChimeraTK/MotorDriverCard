#ifndef _GET_MOTOR_PARAMETERS_H_
#define _GET_MOTOR_PARAMETERS_H_

class ParametersPanel;
#include "MotorControlerConfig.h"
#include "MotorDriverCardConfig.h"

/** Get the motor parameters from the motor panel.
 *  It cannot be a member funcion because the panel knows nothing
 *  about being a motor panel. It just has named widgets.
 */
mtca4u::MotorControlerConfig getMotorParameters(ParametersPanel *parametersPanel);


/** Get the motor driver card parameters. The object is returning with default MotorCotrolerConfigs (FIXME: does this make sense?)
 */
mtca4u::MotorDriverCardConfig getMotorDriverCardParameters(ParametersPanel *parametersPanel);

#endif /* _GET_MOTOR_PARAMETERS_H_ */
