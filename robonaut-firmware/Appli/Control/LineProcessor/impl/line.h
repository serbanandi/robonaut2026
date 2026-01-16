#ifndef LINE_IMPL_H
#define LINE_IMPL_H

#include "../line_interface.h"

typedef struct
{
    uint16_t adcThreshold;
    bool useSingleLineDetection;
} line_ParamSettingsType;

typedef enum
{
    LINE_STATE_NO_LINE = 0,
    LINE_STATE_SINGLE_LINE,
    LINE_STATE_POTENTIAL_QUAD_LINE,
    LINE_STATE_QUAD_LINE,
    LINE_STATE_SINGLE_LINE_AFTER_QUAD,
    LINE_STATE_WAIT_FOR_LINE_SPLIT_START,
    LINE_STATE_WAIT_FOR_LINE_SPLIT_STABILIZATION,
    LINE_STATE_HANDLE_LINE_SPLIT
} line_InternalStateType;

typedef struct
{
    int startPos, endPos;
} line_DetectionChunkType;

#endif // LINE_IMPL_H
