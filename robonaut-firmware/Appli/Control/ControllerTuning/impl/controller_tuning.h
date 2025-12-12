#ifndef CONTROL_CONTROLLERTUNING_IMPL_CONTROLLER_TUNING_H_
#define CONTROL_CONTROLLERTUNING_IMPL_CONTROLLER_TUNING_H_

typedef enum {
    THRESHOLD = 0,	//Threshold for peak detection
    P_COEFF,		//P coefficient for PID controller
	I_COEFF,		//I coefficient for PID controller
	D_COEFF,		//D coefficient for PID controller
	SPEED,			//Speed of car
	MODE,			//Simple or 3 line detection mode	
} tuning_ParameterType;

typedef enum{
	IDLE = 0,
	SELECT_PARAM,
	ADJUST_PARAM,
	APPLY_PARAM,
} tuning_StateType;

#endif /* CONTROL_CONTROLLERTUNING_IMPL_CONTROLLER_TUNING_H_ */
