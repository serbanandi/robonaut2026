/*
 * NeuralNetwork.c
 *
 *  Created on: Sep 14, 2025
 *      Author: Nagy Ákos
 */

#include "NeuralNetwork.h"
#include <stddef.h>
#include "SimpleLogger/SimpleLogger.h"
#include "app_config.h"
#include "ll_aton_runtime.h"

#define MAX_NUMBER_OUTPUT (5)

// Model vars
static uint8_t* model_in_buf;
static uint32_t model_in_len;
static float* model_out[MAX_NUMBER_OUTPUT];
static int32_t model_out_len[MAX_NUMBER_OUTPUT];
static int number_output = 0;

// Post-processing vars
static yolov8_pp_static_param_t pp_params;
static od_pp_out_t pp_output;

static NN_State nn_state = NN_STATE_IDLE;

CLASSES_TABLE;
LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(Default);

static void Model_Init(float* nn_out[], int* number_output, int32_t nn_out_len[])
{
    assert(nn_out != NULL);
    assert(number_output != NULL);
    assert(nn_out_len != NULL);

    const LL_Buffer_InfoTypeDef* nn_in_info = LL_ATON_Input_Buffers_Info_Default();
    const LL_Buffer_InfoTypeDef* nn_out_info = LL_ATON_Output_Buffers_Info_Default();

    // Get the input buffer address
    model_in_buf = (uint8_t*) LL_Buffer_addr_start(&nn_in_info[0]);

    /* Count number of outputs */
    while (nn_out_info[*number_output].name != NULL)
    {
        (*number_output)++;
    }
    assert(*number_output <= MAX_NUMBER_OUTPUT);

    for (int i = 0; i < *number_output; i++)
    {
        // Get the output buffers address
        nn_out[i] = (float*) LL_Buffer_addr_start(&nn_out_info[i]);
        nn_out_len[i] = LL_Buffer_len(&nn_out_info[i]);
    }

    model_in_len = LL_Buffer_len(&nn_in_info[0]);
}

bool NN_Init(void)
{
    Model_Init(model_out, &number_output, model_out_len);
    bool ret = (app_postprocess_init(&pp_params) == AI_OD_POSTPROCESS_ERROR_NO);
    if (ret)
        nn_state = NN_STATE_READY;

    LL_ATON_RT_RuntimeInit();

    // Enable RISAF for NPU RAMs
    __HAL_RCC_RISAF_CLK_ENABLE();

    RISAF4->REG[0].CFGR = 0x00000000;
    RISAF4->REG[1].CFGR = 0x00000000;
    RISAF4->REG[0].CIDCFGR = 0x000F000F; /* RW for everyone */
    RISAF4->REG[0].ENDR = 0xFFFFFFFF;    /* all-encompassing */
    RISAF4->REG[0].CFGR = 0x00000101;    /* enabled, secure, unprivileged for everyone */
    RISAF4->REG[1].CIDCFGR = 0x00FF00FF; /* RW for everyone */
    RISAF4->REG[1].ENDR = 0xFFFFFFFF;    /* all-encompassing */
    RISAF4->REG[1].CFGR = 0x00000001;    /* enabled, non-secure, unprivileged*/

    RISAF5->REG[0].CFGR = 0x00000000;
    RISAF5->REG[1].CFGR = 0x00000000;
    RISAF5->REG[0].CIDCFGR = 0x000F000F; /* RW for everyone */
    RISAF5->REG[0].ENDR = 0xFFFFFFFF;    /* all-encompassing */
    RISAF5->REG[0].CFGR = 0x00000101;    /* enabled, secure, unprivileged for everyone */
    RISAF5->REG[1].CIDCFGR = 0x00FF00FF; /* RW for everyone */
    RISAF5->REG[1].ENDR = 0xFFFFFFFF;    /* all-encompassing */
    RISAF5->REG[1].CFGR = 0x00000001;    /* enabled, non-secure, unprivileged*/

    RISAF6->REG[0].CFGR = 0x00000000;
    RISAF6->REG[1].CFGR = 0x00000000;
    RISAF6->REG[0].CIDCFGR = 0x000F000F; /* RW for everyone */
    RISAF6->REG[0].ENDR = 0xFFFFFFFF;    /* all-encompassing */
    RISAF6->REG[0].CFGR = 0x00000101;    /* enabled, secure, unprivileged for everyone */
    RISAF6->REG[1].CIDCFGR = 0x00FF00FF; /* RW for everyone */
    RISAF6->REG[1].ENDR = 0xFFFFFFFF;    /* all-encompassing */
    RISAF6->REG[1].CFGR = 0x00000001;    /* enabled, non-secure, unprivileged*/

    return ret;
}

uint8_t* NN_GetInputAddres(void)
{
    return model_in_buf;
}

od_pp_out_t* NN_GetOutput(void)
{
    return &pp_output;
}

NN_State NN_Run(void)
{
    switch (nn_state)
    {
        case NN_STATE_IDLE: Log("NN", "NN is not ready, cannot run inference"); break;

        case NN_STATE_READY:
            // Initialize runtime + network
            LL_ATON_RT_Init_Network(&NN_Instance_Default);
            nn_state = NN_STATE_RUNNING;
            break;

        case NN_STATE_RUNNING:
            // Perform a single epoch-block step
            LL_ATON_RT_RetValues_t rt = LL_ATON_RT_RunEpochBlock(&NN_Instance_Default);

            if (rt == LL_ATON_RT_DONE)
            {
                LL_ATON_RT_DeInit_Network(&NN_Instance_Default);

                // Post-process outputs
                int32_t ret = app_postprocess_run(model_out, number_output, &pp_output, &pp_params);
                if (ret != 0)
                {
                    Log("NN", "NN post-process failed.");
                    pp_output.nb_detect = 0;
                }
                else
                {
                    for (int i = 0; i < number_output; i++)
                    {
                        float32_t* tmp = model_out[i];
                        SCB_InvalidateDCache_by_Addr(tmp, model_out_len[i]);
                    }
                }

                nn_state = NN_STATE_READY;
            }
            break;
    }

    return nn_state;
}
