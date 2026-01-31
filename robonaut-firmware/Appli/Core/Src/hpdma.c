/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    hpdma.c
 * @brief   This file provides code for the configuration
 *          of the HPDMA instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include "hpdma.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

DMA_HandleTypeDef handle_HPDMA1_Channel15;

/* HPDMA1 init function */
void MX_HPDMA1_Init(void)
{

  /* USER CODE BEGIN HPDMA1_Init 0 */

    __HAL_RCC_HPDMA1_CLK_ENABLE();

    /* HPDMA1 interrupt Init */
    HAL_NVIC_SetPriority(HPDMA1_Channel15_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(HPDMA1_Channel15_IRQn);

  /* USER CODE END HPDMA1_Init 0 */

  DMA_IsolationConfigTypeDef IsolationConfiginput = {0};

  /* USER CODE BEGIN HPDMA1_Init 1 */

  /* USER CODE END HPDMA1_Init 1 */
  handle_HPDMA1_Channel15.Instance = HPDMA1_Channel15;
  handle_HPDMA1_Channel15.Init.Request = DMA_REQUEST_SW;
  handle_HPDMA1_Channel15.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
  handle_HPDMA1_Channel15.Init.Direction = DMA_MEMORY_TO_MEMORY;
  handle_HPDMA1_Channel15.Init.SrcInc = DMA_SINC_INCREMENTED;
  handle_HPDMA1_Channel15.Init.DestInc = DMA_DINC_INCREMENTED;
  handle_HPDMA1_Channel15.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_WORD;
  handle_HPDMA1_Channel15.Init.DestDataWidth = DMA_DEST_DATAWIDTH_WORD;
  handle_HPDMA1_Channel15.Init.Priority = DMA_LOW_PRIORITY_HIGH_WEIGHT;
  handle_HPDMA1_Channel15.Init.SrcBurstLength = 4;
  handle_HPDMA1_Channel15.Init.DestBurstLength = 4;
  handle_HPDMA1_Channel15.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT0;
  handle_HPDMA1_Channel15.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
  handle_HPDMA1_Channel15.Init.Mode = DMA_NORMAL;
  if (HAL_DMA_Init(&handle_HPDMA1_Channel15) != HAL_OK)
  {
    Error_Handler();
  }
  IsolationConfiginput.CidFiltering = DMA_ISOLATION_ON;
  IsolationConfiginput.StaticCid = DMA_CHANNEL_STATIC_CID_1;
  if (HAL_DMA_SetIsolationAttributes(&handle_HPDMA1_Channel15, &IsolationConfiginput) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN HPDMA1_Init 2 */

  /* USER CODE END HPDMA1_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
