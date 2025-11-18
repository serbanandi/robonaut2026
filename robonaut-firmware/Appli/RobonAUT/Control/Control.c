#include "main.h"
#include "math.h"
#include "Control.h"
#include "LineSensor/LineSensor.h"


#define ONE_LS
//#define TWO_LS

#define P_0 0.5

static float error, last_error, int_error;

static float CTRL_CalculatePosition(uint16_t adc[32]) {
	uint16_t wsum_adc=0, sum_adc=0;
	int8_t i;
	for(i=0; i<32; i++){
		sum_adc += adc[i];
		wsum_adc +=  adc[i]*(i-16);
	}
	return ((float)wsum_adc)/((float)sum_adc);
}

static float CTRL_CalculateDP(float p){
	return p-P_0;
}

static float CTRL_CalculateError(LS_ADC_Values_Type adc){
	float e;
#ifdef ONE_LS
	float p, dp;

	p = CTRL_CalculatePosition(adc.front_adc);
	dp = CTRL_CalculateDP(p);

	e = dp;
#elif defined TWO_LS
	float p_f, p_r, dp_f, dp_r;

	p_f = CTRL_CalculatePosition(adc.front_adc);
	dp_f = CTRL_CalculateDP(p_f);

	p_r = CTRL_CalculatePosition(adc.rear_adc);
	dp_r = CTRL_CalculateDP(p_r);

	e = dp_f-dp_r;
#else
#error
#endif
	return e;
}

void CTRL_InitLoop(){
	error = 0;
	last_error = 0;
	int_error = 0;
}


float CTRL_RunLoop(float P, float I, float D, float Ts){
	LS_ADC_Values_Type adc_values;
	float control_signal;
	float d_error;

	last_error = error; 						//save previous error value

	LS_GetADCValues(&adc_values); 				//read ADC

	error = CTRL_CalculateError(adc_values);	//calculate new error from ADC values

	d_error = (error-last_error)/Ts;			//calculate d/dt e
	int_error += error*Ts; 						//increase S e dt by e*dt


	control_signal = P*error + I*int_error + D*d_error;	//calculate control signal


	return control_signal;
}


float CTRL_RunLoop_TUNING(float pos, float P, float I, float D, float Ts){
	float control_signal;
	float d_error;

	last_error = error; 						//save previous error value

	error = pos - P_0;							//calculate new error from received position

	d_error = (error-last_error)/Ts;			//calculate d/dt e
	int_error += error*Ts; 						//increase S e dt by e*dt


	control_signal = P*error + I*int_error + D*d_error;	//calculate control signal


	return control_signal;
}