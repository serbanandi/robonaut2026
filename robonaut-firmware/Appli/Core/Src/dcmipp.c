/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dcmipp.c
  * @brief   This file provides code for the configuration
  *          of the DCMIPP instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "dcmipp.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

DCMIPP_HandleTypeDef hdcmipp;

/* DCMIPP init function */
void MX_DCMIPP_Init(void)
{

  /* USER CODE BEGIN DCMIPP_Init 0 */

  /* USER CODE END DCMIPP_Init 0 */

  DCMIPP_ParallelConfTypeDef pParallelConfig = {0};
  DCMIPP_PipeConfTypeDef pPipeConfig = {0};

  /* USER CODE BEGIN DCMIPP_Init 1 */

  /* USER CODE END DCMIPP_Init 1 */

  /** Parallel Config
  */
  pParallelConfig.PCKPolarity = DCMIPP_PCKPOLARITY_RISING ;
  pParallelConfig.HSPolarity = DCMIPP_HSPOLARITY_LOW ;
  pParallelConfig.VSPolarity = DCMIPP_VSPOLARITY_LOW ;
  pParallelConfig.ExtendedDataMode = DCMIPP_INTERFACE_8BITS;
  pParallelConfig.Format = DCMIPP_FORMAT_RGB565;
  pParallelConfig.SwapBits = DCMIPP_SWAPBITS_DISABLE;
  pParallelConfig.SwapCycles = DCMIPP_SWAPCYCLES_DISABLE;
  pParallelConfig.SynchroMode = DCMIPP_SYNCHRO_HARDWARE;
  HAL_DCMIPP_PARALLEL_SetConfig(&hdcmipp, &pParallelConfig);

  /** Pipe 0 Config
  */
  pPipeConfig.FrameRate = DCMIPP_FRAME_RATE_ALL;
  pPipeConfig.PixelPipePitch = 800;
  pPipeConfig.PixelPackerFormat = DCMIPP_PIXEL_PACKER_FORMAT_RGB565_1;
  if (HAL_DCMIPP_PIPE_SetConfig(&hdcmipp, DCMIPP_PIPE0, &pPipeConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Pipe 1 Config
  */
  pPipeConfig.PixelPipePitch = 768;
  pPipeConfig.PixelPackerFormat = DCMIPP_PIXEL_PACKER_FORMAT_RGB888_YUV444_1;
  if (HAL_DCMIPP_PIPE_SetConfig(&hdcmipp, DCMIPP_PIPE1, &pPipeConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Pipe 2 Config
  */
  pPipeConfig.PixelPipePitch = 800;
  pPipeConfig.PixelPackerFormat = DCMIPP_PIXEL_PACKER_FORMAT_RGB565_1;
  if (HAL_DCMIPP_PIPE_SetConfig(&hdcmipp, DCMIPP_PIPE2, &pPipeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DCMIPP_Init 2 */

  /* USER CODE END DCMIPP_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
