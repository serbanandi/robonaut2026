#ifndef CAMERA_H_
#define CAMERA_H_

#include <stdbool.h>
#include <stdint.h>

// Types
typedef enum {
    CAM_STATE_RESET = 0,
    CAM_STATE_READY,
    CAM_STATE_WAITING_CAPTURE,
	CAM_STATE_WAITING_COPY,
	CAM_STATE_WAITING_INFERENCE,
	CAM_STATE_WAITING_RENDERING,
    CAM_STATE_ERROR
} CAM_State_Type;

bool CAM_Init(void);
bool CAM_Start(void);
bool CAM_Stop(void);
void CAM_Process(void);
CAM_State_Type CAM_GetState(void);

#endif /* CAMERA_H_ */
