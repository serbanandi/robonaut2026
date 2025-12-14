#ifndef LINE_IMPL_H
#define LINE_IMPL_H

#include "../line_interface.h"

typedef struct {
    uint16_t adcThreshold;
    bool useSingleLineDetection;
} line_ParamSettingsType;

typedef enum 
{
    LINE_STATE_NO_LINE,
    LINE_STATE_SINGLE_LINE,
    LINE_STATE_MULTI_LINE_DETECTED,
    LINE_STATE_SINGLE_AFTER_MULTI,
    LINE_STATE_TRIPLE_LINE,
    LINE_STATE_TRIPLE_LINE_DASHED
} line_InternalStateType;

typedef struct
{
    int startPos, endPos;
} line_DetectionChunkType;

#endif // LINE_IMPL_H