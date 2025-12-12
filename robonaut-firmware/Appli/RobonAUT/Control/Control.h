#ifndef ROBONAUT_CONTROL_CONTROL_H_
#define ROBONAUT_CONTROL_CONTROL_H_

void CTRL_InitLoop();

float CTRL_RunLoop(float P, float I, float D, float Ts);

float CTRL_RunLoop_TUNING(float pos, float P, float I, float D, float Ts);

#endif /* ROBONAUT_CONTROL_CONTROL_H_ */
