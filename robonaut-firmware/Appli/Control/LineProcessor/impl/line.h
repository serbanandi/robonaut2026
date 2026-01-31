#ifndef LINE_IMPL_H
#define LINE_IMPL_H

#include "../line_interface.h"

// --- TUNING PARAMETERS ---
#define TRACKING_WINDOW 4.0f // +/- sensors to look for the line
#define MIN_CHUNK_WIDTH 2    // Minimum sensors to count as a line
#define MAX_CHUNKS 8         // Allow 4 lines + noise

typedef struct
{
    uint16_t adcThreshold;
    bool useSingleLineDetection;
} line_ParamSettingsType;

typedef enum
{
    LINE_STATE_NO_LINE = 0,       // Logic: No line detected. Stop motors.
    LINE_STATE_FOLLOW_LINE,       // Logic: Standard PID, follow the line closest to 'lastKnownPosition'.
    LINE_STATE_JUNCTION_DETECTED, // Logic: Junction detected, prepare for split, query strategy for next move.
    LINE_STATE_SPLIT_MANEUVER     // Logic: Execute the split maneuver as per strategy instruction.
} line_InternalStateType;

typedef struct
{
    int startPos, endPos;
} line_DetectionChunkType;

#endif // LINE_IMPL_H
