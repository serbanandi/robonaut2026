#ifndef NEURAL_NETWORK_H_
#define NEURAL_NETWORK_H_

#include <stdbool.h>
#include "app_postprocess.h"

bool NN_Init(void);
od_pp_out_t* NN_Run(uint8_t* img_buf, uint32_t img_len);

#endif /* NEURAL_NETWORK_H_ */
