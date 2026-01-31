#include "line.h"
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "Drive/drv_interface.h"
#include "LineSensor/ls_interface.h"
#include "Telemetry/tel_interface.h"

static line_InternalStateType currentState;
static line_InternalStateType lastValidState;
static uint16_t adcThreshold = 1000;   // Rising edge threshold
static uint16_t adcThresholdLow = 800; // Falling edge threshold (Hysteresis)

static bool logLineAdcValues = false;
static bool logSmoothedAdcValues = false;
static bool logLineThresholds = false;
static bool logDetectedChunks = false;

static uint32_t currentStateStartEncoderCnt = 0;
static line_SplitDirectionType selectedSplitInfo = LINE_SPLIT_STRAIGHT;
static float lastDetectedLinePos = 15.5f;

static uint8_t lastDetectedChunkNum = 0;

static float smoothedAdc[32]; // Buffer for smoothed values

// --- HELPER: GAUSSIAN SMOOTHING ---
static void _line_GaussianSmooth(uint16_t* raw, float* smooth)
{
    smooth[0] = (float) raw[0];
    smooth[31] = (float) raw[31];
    for (int i = 1; i < 31; i++)
    {
        smooth[i] = (raw[i - 1] * 0.25f) + (raw[i] * 0.5f) + (raw[i + 1] * 0.25f);
    }
}

// --- HELPER: CHUNK DETECTION ---
static uint8_t _line_DetectLineChunks(line_DetectionChunkType chunks[MAX_CHUNKS], float* adcValues)
{
    int chunkNum = 0;
    bool insideChunk = false;
    int startIndex = 0;

    for (int i = 0; i < 32; i++)
    {
        if (!insideChunk)
        {
            if (adcValues[i] > adcThreshold)
            {
                insideChunk = true;
                startIndex = i;
            }
        }
        else
        {
            if (adcValues[i] < adcThresholdLow)
            {
                insideChunk = false;
                if ((i - startIndex) >= MIN_CHUNK_WIDTH)
                {
                    chunks[chunkNum].startPos = startIndex;
                    chunks[chunkNum].endPos = i - 1;
                    chunkNum++;
                    if (chunkNum >= MAX_CHUNKS)
                        return chunkNum;
                }
            }
        }
    }
    // Check edge case (end of array)
    if (insideChunk)
    {
        if ((32 - startIndex) >= MIN_CHUNK_WIDTH)
        {
            chunks[chunkNum].startPos = startIndex;
            chunks[chunkNum].endPos = 31;
            chunkNum++;
        }
    }
    return chunkNum;
}

// --- HELPER: CENTROID CALCULATION ---
static float _line_GetCentroid(int start, int end, float* adcValues)
{
    float sum = 0, wsum = 0;
    for (int i = start; i <= end; i++)
    {
        sum += adcValues[i];
        wsum += adcValues[i] * i;
    }
    return (sum > 1.0f) ? (wsum / sum) : 0.0f;
}

// --- HELPER: JUNCTION CHECK ---
static bool _line_IsJunction(line_DetectionChunkType* chunks, int numChunks)
{
    if (numChunks != 4)
        return false;
    return true;
    /*
        // Geometry Validation:
        // 1. Outer chunks should be near the edges (Markers)
        bool leftMarker  = (chunks[0].startPos < 10);
        bool rightMarker = (chunks[3].endPos > 20);

        // 2. Inner chunks should be somewhat centered (The Track)
        float innerLeftPos  = (chunks[1].startPos + chunks[1].endPos) / 2.0f;
        float innerRightPos = (chunks[2].startPos + chunks[2].endPos) / 2.0f;

        // Check if inner lines are roughly in the middle 16 sensors
        bool innerValid = (innerLeftPos > 12.0f && innerRightPos < 18.0f);

        return (leftMarker && rightMarker && innerValid);
    */
}

// --- HELPER: CHECK FOR FULL BLACK LINE (START/FINISH) ---
static bool _line_isFullBlack(float* adcValues)
{
    int blackCount = 0;
    // Check central 20 sensors (indices 6-25) to avoid edge noise
    for (int i = 6; i < 26; i++)
    {
        if (adcValues[i] > adcThreshold) // Assuming > Threshold means BLACK
            blackCount++;
    }

    // If > 90% of the center is black, it's a cross-line
    return (blackCount > 18);
}

// --- HELPER: SHOW LEDS ---
static void _line_ShowFbLeds(float* adcValues)
{
    ls_LedValuesType led_values;
    for (int i = 0; i < 32; i++)
    {
        led_values.v[i] = !(adcValues[i] < (float) adcThreshold);
    }
    ls_SetFbLEDs(&led_values, LS_SENSOR_FRONT);
}

// --- HELPER: LOGGING ---
static void _line_LogAdcValues(float* adcValues)
{
    if (logLineAdcValues)
    {
        char buffer[256];
        int offset = 0;
        offset += sprintf(&buffer[offset], "Line ADCs: ");
        for (int i = 0; i < 32; i++)
        {
            offset += sprintf(&buffer[offset], "%d ", (int) adcValues[i]);
        }
        sprintf(&buffer[offset], "\n");
        tel_Log(TEL_LOG_DEBUG, buffer);
    }
    if (logSmoothedAdcValues)
    {
        char buffer[256];
        int offset = 0;
        offset += sprintf(&buffer[offset], "Smoothed ADCs: ");
        for (int i = 0; i < 32; i++)
        {
            offset += sprintf(&buffer[offset], "%d ", (int) smoothedAdc[i]);
        }
        sprintf(&buffer[offset], "\n");
        tel_Log(TEL_LOG_DEBUG, buffer);
    }
    if (logLineThresholds)
    {
        char buffer[256];
        int offset = 0;
        offset += sprintf(&buffer[offset], "Line Thresholds: ");
        for (int i = 0; i < 32; i++)
        {
            offset += sprintf(&buffer[offset], "%d ", adcValues[i] > adcThreshold ? 1 : 0);
        }
        sprintf(&buffer[offset], "\n");
        tel_Log(TEL_LOG_DEBUG, buffer);
    }
    if (logDetectedChunks)
    {
        line_DetectionChunkType chunks[MAX_CHUNKS];
        uint8_t chunkCount = _line_DetectLineChunks(chunks, adcValues);
        char buffer[256];
        int offset = 0;
        offset += sprintf(&buffer[offset], "Detected Chunks (%d): ", chunkCount);
        for (int i = 0; i < chunkCount; i++)
        {
            offset += sprintf(&buffer[offset], "[%d-%d] ", chunks[i].startPos, chunks[i].endPos);
        }
        sprintf(&buffer[offset], "\n");
        tel_Log(TEL_LOG_DEBUG, buffer);
    }
}

// --- CORE: SELECT LINE POSITION ---
static float _line_SelectBestLine(float* adcValues, line_DetectionChunkType* chunks, int numChunks, bool forceTurn)
{
    if (numChunks == 0)
        return lastDetectedLinePos;

    // --- CASE 1: JUNCTION DETECTED (4 Lines) ---
    // If we see the junction, we MUST track the center of the inner two lines.
    // Ignoring the outer markers prevents the robot from jerking left/right.
    if (_line_IsJunction(chunks, numChunks))
    {
        float center1 = _line_GetCentroid(chunks[1].startPos, chunks[1].endPos, adcValues);
        float center2 = _line_GetCentroid(chunks[2].startPos, chunks[2].endPos, adcValues);
        return (center1 + center2) / 2.0f;
    }

    // --- CASE 2: NORMAL TRACKING / FORK ---
    float centers[MAX_CHUNKS];
    for (int i = 0; i < numChunks; i++)
    {
        centers[i] = _line_GetCentroid(chunks[i].startPos, chunks[i].endPos, adcValues);
    }

    int bestIdx = -1;
    float minDist = 1000.0f;

    // Logic: Look for line closest to where we expect it to be
    float searchPos = lastDetectedLinePos;

    // Bias search window during turns
    if (forceTurn)
    {
        if (selectedSplitInfo == LINE_SPLIT_RIGHT)
            searchPos -= 8.0f;
        if (selectedSplitInfo == LINE_SPLIT_LEFT)
            searchPos += 8.0f;
    }

    for (int i = 0; i < numChunks; i++)
    {
        float dist = fabs(centers[i] - searchPos);
        if (dist < TRACKING_WINDOW && dist < minDist)
        {
            minDist = dist;
            bestIdx = i;
        }
    }

    // Recovery if window is empty
    if (bestIdx == -1)
    {
        for (int i = 0; i < numChunks; i++)
        {
            float dist = fabs(centers[i] - searchPos);
            if (dist < minDist)
            {
                minDist = dist;
                bestIdx = i;
            }
        }
    }
    return centers[bestIdx];
}

void line_Init()
{
    line_ResetInternalState();
    tel_RegisterRW(&adcThreshold, TEL_UINT16, "line_adcThreshold", 1000);
    tel_RegisterRW(&logLineAdcValues, TEL_UINT8, "line_logAdcValues", 1000);
    tel_RegisterRW(&logSmoothedAdcValues, TEL_UINT8, "line_logSmoothedAdcValues", 1000);
    tel_RegisterRW(&logDetectedChunks, TEL_UINT8, "line_logDetectedChunks", 1000);
    tel_RegisterRW(&logLineThresholds, TEL_UINT8, "line_logThresholds", 1000);
    tel_RegisterR(&currentState, TEL_UINT8, "line_currentState", 100);
    tel_RegisterR(&lastDetectedChunkNum, TEL_UINT8, "line_lastDetectedChunkNum", 100);
}

void line_ResetInternalState()
{
    currentState = LINE_STATE_FOLLOW_LINE;
    lastValidState = LINE_STATE_FOLLOW_LINE;
    currentStateStartEncoderCnt = 0;
}

// --- MAIN PROCESS ---
line_DetectionResultType line_Process(line_ChooseLineFunc chooseLineFunc)
{
    ls_AdcValuesType adcRaw;
    line_DetectionChunkType chunks[MAX_CHUNKS];
    uint8_t chunkCount;
    bool forceTurn = false;

    ls_GetADCValues(&adcRaw, LS_SENSOR_FRONT);
    ls_ClearNewDataFlag();

    _line_ShowFbLeds(smoothedAdc);

    _line_GaussianSmooth(adcRaw.v, smoothedAdc);

    _line_LogAdcValues(smoothedAdc);

    if (_line_isFullBlack(smoothedAdc))
    {
        return (line_DetectionResultType) { .detectedLinePos = lastDetectedLinePos,
                                            .lineDetected = true,
                                            .allBlack = true };
    }

    chunkCount = _line_DetectLineChunks(chunks, smoothedAdc);
    if (chunkCount == 0)
    {
        currentState = LINE_STATE_NO_LINE;
        tel_Log(TEL_LOG_INFO, "Line lost.");
    }

    switch (currentState)
    {
        case LINE_STATE_JUNCTION_DETECTED:
            // Prepare for split maneuver
            if (_line_IsJunction(chunks, chunkCount) == false)
            {
                currentState = LINE_STATE_SPLIT_MANEUVER;
                lastValidState = currentState;
                currentStateStartEncoderCnt = drv_GetEncoderCount();
                tel_Log(TEL_LOG_INFO, "Preparing for split maneuver.");
            }
            break;
        case LINE_STATE_SPLIT_MANEUVER:
            // During split maneuver, bias line selection
            forceTurn = true;
            // Exit maneuver once 20k encoder ticks have passed and we have 1 clean line again
            if ((drv_GetEncoderCount() - currentStateStartEncoderCnt) > 20000 && chunkCount == 1)
            {
                currentState = LINE_STATE_FOLLOW_LINE;
                lastValidState = currentState;
                tel_Log(TEL_LOG_INFO, "Split maneuver completed.");
                forceTurn = false;
            }
            break;
        case LINE_STATE_FOLLOW_LINE:
            if (_line_IsJunction(chunks, chunkCount))
            {
                currentState = LINE_STATE_JUNCTION_DETECTED;
                lastValidState = currentState;
                tel_Log(TEL_LOG_INFO, "Junction detected.");

                // Ask strategy for the NEXT move
                selectedSplitInfo = chooseLineFunc(chunkCount);
            }
            break;
        case LINE_STATE_NO_LINE:
            if (chunkCount > 0)
            {
                currentState = lastValidState;
                tel_Log(TEL_LOG_INFO, "Line re-acquired. State restored to %d.", currentState);
            }
        default: break;
    }

    if (chunkCount > 0)
    {
        lastDetectedLinePos = _line_SelectBestLine(smoothedAdc, chunks, chunkCount, forceTurn);
    }

    return (line_DetectionResultType) { .detectedLinePos = lastDetectedLinePos,
                                        .lineDetected = (chunkCount > 0),
                                        .allBlack = false };
}
