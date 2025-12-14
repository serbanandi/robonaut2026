#include "line.h"
#include "LineSensor/LineSensor.h"
#include <stdio.h>
#include <stdarg.h>
#include "Telemetry/tel_interface.h"


static line_InternalStateType currentState;
static line_ParamSettingsType currentParams = 
{
    .adcThreshold = 900,
    .useSingleLineDetection = false,
};

static unsigned int tripleLineDetectionTimeMs = 0;
static unsigned int lastLineDetectionTime = 0;
static float lastDetectedLinePos = 0.0f;
static bool logLineAdcValues = false;
static bool logLineThresholds = false;


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


static int _line_DetectLineChunks(line_DetectionChunkType chunks[3], uint16_t adcValues[32])
{
    int chunkNum = 0;
    int chunkActivationCnt = 0;
    bool chunkIsActive = false;

    for (int p = 0; p < 32; p++)
    {
        if (!chunkIsActive)
        {
            if (adcValues[p] >= currentParams.adcThreshold)
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
            if (adcValues[p] < currentParams.adcThreshold)
            {
                chunkActivationCnt++;
                if (chunkActivationCnt == 2)
                {
                	chunks[chunkNum].endPos = p + 1 - chunkActivationCnt;
                    chunkActivationCnt = 0;
                    chunkIsActive = false;
                    chunkNum++;
                    if (chunkNum == 3)
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
        led_values.front_led[i] = !(adcValues[i] < currentParams.adcThreshold);
        led_values.rear_led[i]  = !(adcValues[i]  < currentParams.adcThreshold);
    }
    LS_SetFbLEDs(&led_values);
}


static float _line_DetectMainLine(uint16_t adcValues[32], line_DetectionChunkType lineChunks[3], int lineChunkNum)
{
    int selectedChunk = 0;
    if (lineChunkNum == 0)
        return 0.0f;
    if (lineChunkNum == 1 || currentParams.useSingleLineDetection)
    {
        selectedChunk = 0;
        //return _line_RunLineDetectionOnPartialData(0, 32, adcValues);
    }

    else if (lineChunkNum == 2)      // TODO: remove this useless overcomplicated piece of shit
    {
        float c1Mid = (lineChunks[0].startPos + lineChunks[0].endPos - 31) * 0.5f;
        float c2Mid = (lineChunks[1].startPos + lineChunks[1].endPos - 31) * 0.5f;
        if (fabs(c1Mid - lastDetectedLinePos) < fabs(c2Mid - lastDetectedLinePos))
            selectedChunk = 0;
        else
            selectedChunk = 1;
    }
    else
    {
        selectedChunk = 1;
    }
    int start = fmax(lineChunks[selectedChunk].startPos - 1, 0);
    int length = fmin(lineChunks[selectedChunk].endPos + 1, 31) - lineChunks[selectedChunk].startPos;
    return _line_RunLineDetectionOnPartialData(start, length, adcValues);
}


void line_Init() 
{
    line_ResetInternalState();
    tel_RegisterRW(&currentParams.adcThreshold, TEL_UINT16, "line_adcThreshold", 1000);
    tel_RegisterRW(&currentParams.useSingleLineDetection, TEL_UINT8, "line_singleLineMode", 1000);
    tel_RegisterRW(&logLineAdcValues, TEL_UINT8, "line_logAdcValues", 1000);
    tel_RegisterRW(&logLineThresholds, TEL_UINT8, "line_logThresholds", 1000);
}


void line_SetParams(const line_ParamSettingsType* params) 
{
    currentParams = *params;
}


void line_ResetInternalState() 
{
    currentState = LINE_STATE_NO_LINE;
    lastLineDetectionTime = 0;
    tripleLineDetectionTimeMs = 0;
}

void line_Process()
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
            len += snprintf(logBuffer + len, sizeof(logBuffer) - len, "%d ", adcValues.front_adc[i] > currentParams.adcThreshold);
        }
        tel_Log(TEL_LOG_INFO, "%s", logBuffer);
    }

    line_DetectionChunkType lineChunks[3];
    int detectedLineChunkNum = _line_DetectLineChunks(lineChunks, adcValues.front_adc);

    if (detectedLineChunkNum > 0)
        lastLineDetectionTime = HAL_GetTick();
    if (detectedLineChunkNum == 0 && HAL_GetTick() - lastLineDetectionTime > 1000)
        currentState = LINE_STATE_NO_LINE;

    uint32_t now = HAL_GetTick();

    switch (currentState)
    {
        case LINE_STATE_NO_LINE:
        {
            if (detectedLineChunkNum > 0)
            {
                currentState = LINE_STATE_SINGLE_LINE;
                lastDetectedLinePos = _line_RunLineDetectionOnPartialData(0, 32, adcValues.front_adc);
            }
            break;
        }
        case LINE_STATE_SINGLE_LINE:
        {
            if (detectedLineChunkNum == 0)
                break;

            lastDetectedLinePos = _line_DetectMainLine(adcValues.front_adc, lineChunks, detectedLineChunkNum);
            if (detectedLineChunkNum == 3 && !currentParams.useSingleLineDetection)
            {
                currentState = LINE_STATE_MULTI_LINE_DETECTED;
                tripleLineDetectionTimeMs = HAL_GetTick();
            }
            break;
        }
        case LINE_STATE_MULTI_LINE_DETECTED:
        {
            if (detectedLineChunkNum == 0)
            {
                currentState = LINE_STATE_SINGLE_LINE;
                break;
            }

            lastDetectedLinePos = _line_DetectMainLine(adcValues.front_adc, lineChunks, detectedLineChunkNum);
            if (detectedLineChunkNum == 1)
            {
                if (now - tripleLineDetectionTimeMs < 50)
                {
                    currentState = LINE_STATE_SINGLE_LINE;
                }
                else
                {
                    currentState = LINE_STATE_SINGLE_AFTER_MULTI;
                    tripleLineDetectionTimeMs = now;
                }                
            }
            else if (now - tripleLineDetectionTimeMs > 80)
            {
                currentState = LINE_STATE_TRIPLE_LINE;
            }
            break;
        }
        case LINE_STATE_SINGLE_AFTER_MULTI:
        {
            if (detectedLineChunkNum == 0)
            {
                currentState = LINE_STATE_SINGLE_LINE;
                break;
            }

            lastDetectedLinePos = _line_DetectMainLine(adcValues.front_adc, lineChunks, detectedLineChunkNum);
            if (detectedLineChunkNum == 3)
            {
                if (now - tripleLineDetectionTimeMs < 50)
                {
                    currentState = LINE_STATE_MULTI_LINE_DETECTED;
                }
                else
                {
                    currentState = LINE_STATE_TRIPLE_LINE_DASHED;
                }                
                tripleLineDetectionTimeMs = now;
            }
            else if (now - tripleLineDetectionTimeMs > 300)
            {
                currentState = LINE_STATE_SINGLE_LINE;
            }
            break;
        }
        case LINE_STATE_TRIPLE_LINE:
        case LINE_STATE_TRIPLE_LINE_DASHED:
        {
            if (detectedLineChunkNum == 0)
            {
                currentState = LINE_STATE_SINGLE_LINE;
                break;
            }

            lastDetectedLinePos = _line_DetectMainLine(adcValues.front_adc, lineChunks, detectedLineChunkNum);
            if (detectedLineChunkNum > 1)
            {
                tripleLineDetectionTimeMs = now;
            }
            else
            {
                if (now - tripleLineDetectionTimeMs > 300)
                {
                    currentState = LINE_STATE_SINGLE_LINE;
                }
            }
        }
        break;
    }
}

void line_GetDetectionResult(line_DetectionResultType* result)
{
    switch (currentState)
    {
        case LINE_STATE_NO_LINE:
            result->lineType = LINE_NO_LINE;
            break;
        case LINE_STATE_SINGLE_LINE:
        case LINE_STATE_MULTI_LINE_DETECTED:
        case LINE_STATE_SINGLE_AFTER_MULTI:
            result->lineType = LINE_SINGLE_LINE;
            break;
        case LINE_STATE_TRIPLE_LINE:
            result->lineType = LINE_TRIPLE_LINE;
            break;
        case LINE_STATE_TRIPLE_LINE_DASHED:
            result->lineType = LINE_TRIPLE_LINE_DASHED;
            break;

    }
    result->position = lastDetectedLinePos;
}
