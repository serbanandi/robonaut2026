#include "line.h"
#include "LineSensor/LineSensor.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include "Telemetry/tel_interface.h"
#include "Drive/drv_interface.h"

static line_InternalStateType currentState;
static uint16_t adcThreshold = 850;

static bool logLineAdcValues = false;
static bool logLineThresholds = false;

static uint32_t lastLineDetectionEncoderCnt = 0;
static uint32_t lastTypeDetectionEncoderCnt = 0;
static uint32_t currentStateStartEncoderCnt = 0;
static uint32_t initialTypeDetectionEncoderCnt = 0;
static uint8_t lastLineChunkNum = 0;
static uint8_t selectedSplitLineIndex = 0;
static float lastDetectedLinePos = 0.0f;

static uint8_t lastDetectedChunkNum = 0;


static float _line_RunLineDetectionOnPartialData(int startingIndex, int length, uint16_t adcValues[32])
{
    int sum_adc = 0;
    int wsum_adc = 0;
    for (int i = startingIndex; i < startingIndex + length; i++) 
    {
        sum_adc += adcValues[i];
        wsum_adc += adcValues[i] * ((i - 16));
    }

    if (sum_adc == 0)
        return 0.0f;

    return ((float)wsum_adc) / ((float)sum_adc);
}


static int _line_DetectLineChunks(line_DetectionChunkType chunks[4], uint16_t adcValues[32])
{
    int chunkNum = 0;
    int chunkActivationCnt = 0;
    bool chunkIsActive = false;

    for (int p = 0; p < 32; p++)
    {
        if (!chunkIsActive)
        {
            if (adcValues[p] >= adcThreshold)
            {
                chunkActivationCnt++;
                if (chunkActivationCnt == 2)
                {
                	chunks[chunkNum].startPos = p + 1 - chunkActivationCnt;
                    chunkActivationCnt = 0;
                    chunkIsActive = true;
                }
            }
            else
            {
                chunkActivationCnt = 0;
            }
        }
        else
        {
            if (adcValues[p] < adcThreshold)
            {
                chunkActivationCnt++;
                if (chunkActivationCnt == 2)
                {
                	chunks[chunkNum].endPos = p + 1 - chunkActivationCnt;
                    chunkActivationCnt = 0;
                    chunkIsActive = false;
                    chunkNum++;
                    if (chunkNum == 4)
                        return chunkNum;
                }
            }
            else
            {
                chunkActivationCnt = 0;
            }
        }
    }

    if (chunkIsActive)
    {
        chunks[chunkNum].endPos = 31 - chunkActivationCnt;
        chunkNum++;
    }

    return chunkNum;
}

static void _line_ShowFbLeds(uint16_t adcValues[32])
{
    LS_LED_Values_Type led_values;
    for (int i = 0; i < 32; i++)
    {
        led_values.front_led[i] = !(adcValues[i] < adcThreshold);
        led_values.rear_led[i]  = !(adcValues[i]  < adcThreshold);
    }
    LS_SetFbLEDs(&led_values);
}


static float _line_DetectMainLine(uint16_t adcValues[32], line_DetectionChunkType lineChunks[4], int lineChunkNum, bool useSelectedSplitLineIdx)
{
    int selectedChunk = 0;
    if (lineChunkNum == 0)
        return 0.0f;
    if (lineChunkNum == 1)
    {
        selectedChunk = 0;
    }

    if (lineChunkNum == 2 )
    {
        float midPos[4];
        for (int i = 0; i < lineChunkNum; i++)
        {
            midPos[i] = (lineChunks[i].startPos + lineChunks[i].endPos - 31) * 0.5f;
        }
        int closestChunk = 0;
        float closestDist = fabs(midPos[0] - lastDetectedLinePos);
        for (int i = 1; i < lineChunkNum; i++)
        {
            float dist = fabs(midPos[i] - lastDetectedLinePos);
            if (dist < closestDist)
            {
                closestDist = dist;
                closestChunk = i;
            }
        }
        selectedChunk = closestChunk;
    }
    else if (lineChunkNum == 3)
    {
        if (useSelectedSplitLineIdx)
        {
            selectedChunk = selectedSplitLineIndex;
        }
        else 
        {
            selectedChunk = 1; // Middle line by default
        }
    }
    else if (lineChunkNum == 4)
    {
    	int start = fmax(lineChunks[1].startPos - 1, 0);
    	int length = fmin(lineChunks[2].endPos + 1, 31) - start + 1;
        return _line_RunLineDetectionOnPartialData(start, length, adcValues);
    }
    int start = fmax(lineChunks[selectedChunk].startPos - 1, 0);
    int length = fmin(lineChunks[selectedChunk].endPos + 1, 31) - start + 1;
    return _line_RunLineDetectionOnPartialData(start, length, adcValues);
}


void line_Init() 
{
    line_ResetInternalState();
    tel_RegisterRW(&adcThreshold, TEL_UINT16, "line_adcThreshold", 1000);
    tel_RegisterRW(&logLineAdcValues, TEL_UINT8, "line_logAdcValues", 1000);
    tel_RegisterRW(&logLineThresholds, TEL_UINT8, "line_logThresholds", 1000);
    tel_RegisterR(&currentState, TEL_UINT8, "line_currentState", 100);
    tel_RegisterR(&lastDetectedChunkNum, TEL_UINT8, "line_lastDetectedChunkNum", 100);
}

void line_ResetInternalState() 
{
    currentState = LINE_STATE_NO_LINE;
    lastLineDetectionEncoderCnt = 0;
}

line_DetectionResultType line_Process(line_ChooseLineFunc chooseLineFunc)
{
    LS_Process();
    LS_ADC_Values_Type adcValues;
    LS_GetADCValues(&adcValues);

    _line_ShowFbLeds(adcValues.front_adc);
    static char logBuffer[500];
    if (logLineAdcValues)
    {
        int len = 0;
        for (int i = 0; i < 32; i++)
        {
            len += snprintf(logBuffer + len, sizeof(logBuffer) - len, "%04u ", adcValues.front_adc[i]);
        }
        tel_Log(TEL_LOG_INFO, "%s", logBuffer);
    }
    if (logLineThresholds)
    {
        int len = 0;
        for (int i = 0; i < 32; i++)
        {
            len += snprintf(logBuffer + len, sizeof(logBuffer) - len, "%d ", adcValues.front_adc[i] > adcThreshold);
        }
        tel_Log(TEL_LOG_INFO, "%s", logBuffer);
    }

    line_DetectionChunkType lineChunks[4];
    int detectedLineChunkNum = _line_DetectLineChunks(lineChunks, adcValues.front_adc);
    lastDetectedChunkNum = detectedLineChunkNum;

    uint32_t currentEncoderCnt = drv_GetEncoderCount();

    if (detectedLineChunkNum > 0)
        lastLineDetectionEncoderCnt = currentEncoderCnt;
    if (detectedLineChunkNum == 0 && currentEncoderCnt - lastLineDetectionEncoderCnt > 2 * DRV_ENCODER_COUNTS_PER_CM)
        currentState = LINE_STATE_NO_LINE;

//    if (detectedLineChunkNum == 0 && currentState != LINE_STATE_NO_LINE)
//    {
//        tel_Log(TEL_LOG_WARN, "No line detected, detected chunks: 0, current state: %d", currentState);
//    }

    switch (currentState)
    {
        case LINE_STATE_NO_LINE:
        {
            if (detectedLineChunkNum > 0)
            {
                currentState = LINE_STATE_SINGLE_LINE;
                currentStateStartEncoderCnt = currentEncoderCnt;
                lastDetectedLinePos = _line_RunLineDetectionOnPartialData(0, 32, adcValues.front_adc);
            }
            else
            {
                lastDetectedLinePos = 0.0f;
            }
            break;
        }
        case LINE_STATE_SINGLE_LINE:
        {
            if (detectedLineChunkNum == 0)
                break;

            lastDetectedLinePos = _line_DetectMainLine(adcValues.front_adc, lineChunks, detectedLineChunkNum, false);
            if (detectedLineChunkNum == 4)
            {
                currentState = LINE_STATE_POTENTIAL_QUAD_LINE;
                currentStateStartEncoderCnt = currentEncoderCnt;
            }
            break;
        }
        case LINE_STATE_POTENTIAL_QUAD_LINE:
        {
            if (detectedLineChunkNum == 0)
            {
                currentState = LINE_STATE_SINGLE_LINE;
                break;
            }

            lastDetectedLinePos = _line_DetectMainLine(adcValues.front_adc, lineChunks, detectedLineChunkNum, false);
            if (detectedLineChunkNum == 4)
            {
                if (currentEncoderCnt - currentStateStartEncoderCnt > 2 * DRV_ENCODER_COUNTS_PER_CM)
                {
                    currentState = LINE_STATE_QUAD_LINE;
                    lastTypeDetectionEncoderCnt = currentEncoderCnt;
                    tel_Log(TEL_LOG_DEBUG, "%u", currentEncoderCnt);
                    tel_Log(TEL_LOG_INFO, "Quad line detected");
                    selectedSplitLineIndex = chooseLineFunc ? chooseLineFunc(detectedLineChunkNum) : 0;
                    switch (selectedSplitLineIndex)
                    {
                        case LINE_SPLIT_LEFT:
                            tel_Log(TEL_LOG_INFO, "Choosing LEFT line");
                            break;
                        case LINE_SPLIT_STRAIGHT:
                            tel_Log(TEL_LOG_INFO, "Choosing STRAIGHT line");
                            break;
                        case LINE_SPLIT_RIGHT:
                            tel_Log(TEL_LOG_INFO, "Choosing RIGHT line");
                            break;
                        default:
                            tel_Log(TEL_LOG_WARN, "Invalid line selection index %d", selectedSplitLineIndex);
                            break;
                    }
                }
            }
            else
            {
                currentState = LINE_STATE_SINGLE_LINE;
            }
            break;
        }
        case LINE_STATE_QUAD_LINE:
        {
            if (detectedLineChunkNum != 0)
            {
                lastDetectedLinePos = _line_DetectMainLine(adcValues.front_adc, lineChunks, detectedLineChunkNum, false);
            }

            if (detectedLineChunkNum == 1)
            {
                // currentState = LINE_STATE_SINGLE_LINE_AFTER_QUAD;
                // currentStateStartEncoderCnt = currentEncoderCnt;
                // tel_Log(TEL_LOG_DEBUG, "%u", currentEncoderCnt);
                // tel_Log(TEL_LOG_INFO, "Quad line reduced to single line, switching to SINGLE_LINE_AFTER_QUAD state");

                currentState = LINE_STATE_WAIT_FOR_LINE_SPLIT_START;
                tel_Log(TEL_LOG_DEBUG, "%u", currentEncoderCnt);
                tel_Log(TEL_LOG_INFO, "Switching to WAIT_FOR_LINE_SPLIT_START state");
                currentStateStartEncoderCnt = currentEncoderCnt;
            }
            break;
        }
        case LINE_STATE_SINGLE_LINE_AFTER_QUAD:
        {
            if (detectedLineChunkNum != 0)
            {
                lastDetectedLinePos = _line_DetectMainLine(adcValues.front_adc, lineChunks, detectedLineChunkNum, false);
            }

            if (currentEncoderCnt - currentStateStartEncoderCnt > 1 * DRV_ENCODER_COUNTS_PER_CM)
            {
                currentState = LINE_STATE_WAIT_FOR_LINE_SPLIT_START;
                tel_Log(TEL_LOG_DEBUG, "%u", currentEncoderCnt);
                tel_Log(TEL_LOG_INFO, "Switching to WAIT_FOR_LINE_SPLIT_START state");
                currentStateStartEncoderCnt = currentEncoderCnt;
            }
            break;
        }
        case LINE_STATE_WAIT_FOR_LINE_SPLIT_START:
        {
            if (detectedLineChunkNum != 0)
            {
                lastDetectedLinePos = _line_DetectMainLine(adcValues.front_adc, lineChunks, detectedLineChunkNum, false);
            }

            if (detectedLineChunkNum > 1)
            {
                lastLineChunkNum = detectedLineChunkNum;
                initialTypeDetectionEncoderCnt = currentEncoderCnt;
                currentStateStartEncoderCnt = currentEncoderCnt;
                currentState = LINE_STATE_WAIT_FOR_LINE_SPLIT_STABILIZATION;
                tel_Log(TEL_LOG_DEBUG, "%u", currentEncoderCnt);
                tel_Log(TEL_LOG_INFO, "Multiple lines detected, switching to WAIT_FOR_LINE_SPLIT_STABILIZATION state");
            }
            else if (currentEncoderCnt - currentStateStartEncoderCnt > 20 * DRV_ENCODER_COUNTS_PER_CM)
            {
                currentState = LINE_STATE_SINGLE_LINE;
                tel_Log(TEL_LOG_DEBUG, "%u", currentEncoderCnt);
                tel_Log(TEL_LOG_INFO, "Returning to SINGLE_LINE state as no multi line was detected in time");
            }
            break;
        }
        case LINE_STATE_WAIT_FOR_LINE_SPLIT_STABILIZATION:
        {
            if (detectedLineChunkNum != 0)
            {
                lastDetectedLinePos = _line_DetectMainLine(adcValues.front_adc, lineChunks, detectedLineChunkNum, false);
            }

            if (detectedLineChunkNum == lastLineChunkNum)
            {
                if (currentEncoderCnt - initialTypeDetectionEncoderCnt > 1 * DRV_ENCODER_COUNTS_PER_CM)
                {
                    tel_Log(TEL_LOG_DEBUG, "%u", currentEncoderCnt);
                    if (detectedLineChunkNum == 4 || detectedLineChunkNum == 0)
                    {
                        tel_Log(TEL_LOG_WARN, "Line split expected, but %d lines detected, reverting to SINGLE_LINE state", detectedLineChunkNum);
                        currentState = LINE_STATE_SINGLE_LINE;
                        break;
                    }
                    if (selectedSplitLineIndex >= detectedLineChunkNum)
                    {
                        tel_Log(TEL_LOG_WARN, "Invalid line selection index %d, defaulting to %d", selectedSplitLineIndex, detectedLineChunkNum - 1);
                        selectedSplitLineIndex = detectedLineChunkNum - 1;
                    }
                    lastDetectedLinePos = _line_RunLineDetectionOnPartialData(
                        fmax(lineChunks[selectedSplitLineIndex].startPos - 1, 0),
                        fmin(lineChunks[selectedSplitLineIndex].endPos + 1, 31) - lineChunks[selectedSplitLineIndex].startPos,
                        adcValues.front_adc);
                    currentState = LINE_STATE_HANDLE_LINE_SPLIT;
                    currentStateStartEncoderCnt = currentEncoderCnt;
                    tel_Log(TEL_LOG_INFO, "Switching to HANDLE_LINE_SPLIT state, selected line chunk: %d", selectedSplitLineIndex);
                }
            }
            else
            {
                lastLineChunkNum = detectedLineChunkNum;
                initialTypeDetectionEncoderCnt = currentEncoderCnt;
                if (currentEncoderCnt - currentStateStartEncoderCnt > 10 * DRV_ENCODER_COUNTS_PER_CM)
                {
                    currentState = LINE_STATE_SINGLE_LINE;
                    tel_Log(TEL_LOG_DEBUG, "%u", currentEncoderCnt);
                    tel_Log(TEL_LOG_WARN, "Returning to SINGLE_LINE state due to prolonged mismatch");
                }
            }
            break;
        }
        case LINE_STATE_HANDLE_LINE_SPLIT:
        {
            if (detectedLineChunkNum != 0)
            {
                lastDetectedLinePos = _line_DetectMainLine(adcValues.front_adc, lineChunks, detectedLineChunkNum, true);
            }

            if (currentEncoderCnt - currentStateStartEncoderCnt > 40 * DRV_ENCODER_COUNTS_PER_CM)
            {
                currentState = LINE_STATE_SINGLE_LINE;
                tel_Log(TEL_LOG_DEBUG, "%u", currentEncoderCnt);
                tel_Log(TEL_LOG_INFO, "Exiting HANDLE_LINE_SPLIT state after some distance traveled");
            }
            break;
        }
    }

    return (line_DetectionResultType){
        .detectedLinePos = lastDetectedLinePos,
        .lineDetected = (currentState != LINE_STATE_NO_LINE)
    };
}
