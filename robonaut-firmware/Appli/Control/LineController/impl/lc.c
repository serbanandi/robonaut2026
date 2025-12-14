#include "main.h"
#include "math.h"
#include "../lc_interface.h"

#define P_0 0.5

static float error, last_error, int_error;

void lc_Init()
{
	error = 0;
	last_error = 0;
	int_error = 0;
}

float lc_Compute(float pos, float P, float I, float D, float Ts)
{
	float control_signal;
	float d_error;

	last_error = error; 						//save previous error value

	error = pos - P_0;							//calculate new error from received position

	d_error = (error-last_error)/Ts;			//calculate d/dt e
	int_error += error*Ts; 						//increase S e dt by e*dt


	control_signal = P*error + I*int_error + D*d_error;	//calculate control signal


	return control_signal;
}