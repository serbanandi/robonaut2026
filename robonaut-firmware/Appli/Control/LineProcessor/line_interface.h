#ifndef LINE_INTERFACE_H
#define LINE_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    LINE_NO_LINE,
    LINE_SINGLE_LINE,
    LINE_TRIPLE_LINE,
    LINE_TRIPLE_LINE_DASHED
} line_DetectedLineType;

typedef struct {
    line_DetectedLineType lineType;
    float position;
} line_DetectionResultType;

void line_Init();

void line_ResetInternalState();

void line_Process();

void line_GetDetectionResult(line_DetectionResultType* result);

#endif // LINE_INTERFACE_H