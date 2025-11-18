#ifndef CONTROL_CONTROLLERTUNING_TUNING_INTERFACE_H_
#define CONTROL_CONTROLLERTUNING_TUNING_INTERFACE_H_

#include <stdbool.h>

typedef struct{
    uint16_t threshold;
    float p_coeff;
    float i_coeff;
    float d_coeff;
    float speed;
    bool mode;
    bool motor_enabled;
} tuning_ParametersType;

void tuning_Init(const tuning_ParametersType* params);

void tuning_Process(void);

#endif /* CONTROL_CONTROLLERTUNING_TUNING_INTERFACE_H_ */
