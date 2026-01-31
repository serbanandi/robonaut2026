#ifndef NEURAL_NETWORK_H_
#define NEURAL_NETWORK_H_

#include <stdbool.h>
#include "app_postprocess.h"

typedef enum {
  NN_STATE_IDLE = 0,
  NN_STATE_READY,
  NN_STATE_RUNNING,
} NN_State;

bool NN_Init(void);
uint8_t* NN_GetInputAddres(void);
NN_State NN_Run(void);
od_pp_out_t* NN_GetOutput(void);

#endif /* NEURAL_NETWORK_H_ */
