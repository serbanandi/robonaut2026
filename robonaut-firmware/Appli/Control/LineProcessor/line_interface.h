#ifndef LINE_INTERFACE_H
#define LINE_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float detectedLinePos;
    bool lineDetected;
} line_DetectionResultType;

typedef enum {
    LINE_SPLIT_RIGHT = 0,
    LINE_SPLIT_STRAIGHT = 1,
    LINE_SPLIT_LEFT = 2
} line_SplitDirectionType;

typedef struct {
    line_SplitDirectionType splitDirection;
    int lineOutCount;
} line_SplitInfoType;

typedef line_SplitInfoType (*line_ChooseLineFunc)(int lineCount);

void line_Init();

void line_ResetInternalState();

line_DetectionResultType line_Process(line_ChooseLineFunc chooseLineFunc);

#endif // LINE_INTERFACE_H
