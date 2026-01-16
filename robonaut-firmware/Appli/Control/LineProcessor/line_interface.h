#ifndef LINE_INTERFACE_H
#define LINE_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    float detectedLinePos;
    bool lineDetected;
    bool allBlack;
} line_DetectionResultType;

typedef enum
{
    LINE_SPLIT_RIGHT = 0,
    LINE_SPLIT_STRAIGHT = 1,
    LINE_SPLIT_LEFT = 2,
} line_SplitDirectionType;

typedef line_SplitDirectionType (*line_ChooseLineFunc)(int lineCount);

void line_Init();

void line_ResetInternalState();

line_DetectionResultType line_Process(line_ChooseLineFunc chooseLineFunc);

#endif // LINE_INTERFACE_H
