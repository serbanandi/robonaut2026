/*
 * NeuralNetwork.c
 *
 *  Created on: Sep 14, 2025
 *      Author: Nagy Ákos
 */

#include "NeuralNetwork.h"
#include "ll_aton_runtime.h"
#include "app_config.h"
#include <stddef.h>

#define MAX_NUMBER_OUTPUT (5)

//Model vars
static uint8_t* model_in_buf;
static uint32_t model_in_len;
static float* model_out[MAX_NUMBER_OUTPUT];
static int32_t model_out_len[MAX_NUMBER_OUTPUT];
static int number_output = 0;

//Post-processing vars
static yolov8_pp_static_param_t pp_params;
static od_pp_out_t pp_output;

CLASSES_TABLE;
LL_ATON_DECLARE_NAMED_NN_INSTANCE_AND_INTERFACE(Default);

static void Model_Init(float *nn_out[], int *number_output, int32_t nn_out_len[])
{
  const LL_Buffer_InfoTypeDef *nn_in_info = LL_ATON_Input_Buffers_Info_Default();
  const LL_Buffer_InfoTypeDef *nn_out_info = LL_ATON_Output_Buffers_Info_Default();

  // Get the input buffer address
  model_in_buf = (uint8_t *) LL_Buffer_addr_start(&nn_in_info[0]);

  /* Count number of outputs */
  while (nn_out_info[*number_output].name != NULL)
  {
    (*number_output)++;
  }
  assert(*number_output <= MAX_NUMBER_OUTPUT);

  for (int i = 0; i < *number_output; i++)
  {
    // Get the output buffers address
    nn_out[i] = (float *) LL_Buffer_addr_start(&nn_out_info[i]);
    nn_out_len[i] = LL_Buffer_len(&nn_out_info[i]);
  }

  model_in_len = LL_Buffer_len(&nn_in_info[0]);
}

bool NN_Init(void)
{
	Model_Init(model_out, &number_output, model_out_len);
	return (app_postprocess_init(&pp_params) == AI_OD_POSTPROCESS_ERROR_NO);
}

od_pp_out_t* NN_Run(uint8_t* img_buf, uint32_t img_len)
{
  if (img_len != model_in_len)
	  return NULL;

  memcpy(model_in_buf, img_buf, sizeof(img_len));
  SCB_CleanInvalidateDCache_by_Addr(model_in_buf, model_in_len);

  /* run ATON inference */
  //uint32_t start_tick = HAL_GetTick();
  LL_ATON_RT_Main(&NN_Instance_Default);

  int32_t ret = app_postprocess_run((void **) model_out, number_output, &pp_output, &pp_params);
  if (ret != 0)
	  return NULL;

  //uint32_t dur_ms = HAL_GetTick() - start_tick;
  return &pp_output;
}
