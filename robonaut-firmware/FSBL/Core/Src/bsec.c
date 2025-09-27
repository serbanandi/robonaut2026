/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsec.c
  * @brief   This file provides code for the configuration
  *          of the BSEC instances.
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
#include "bsec.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* BSEC init function */
void MX_BSEC_Init(void)
{

  /* USER CODE BEGIN BSEC_Init 0 */
  // Enable debug for everyone
  BSEC_HandleTypeDef hbsec;
  hbsec.Instance = BSEC;

  BSEC_DebugCfgTypeDef debugCfg;
  debugCfg.HDPL_Open_Dbg = HAL_BSEC_OPEN_DBG_LEVEL_0;
  debugCfg.Sec_Dbg_Auth = HAL_BSEC_SEC_DBG_AUTH;
  debugCfg.NonSec_Dbg_Auth = HAL_BSEC_NONSEC_DBG_AUTH;

  __HAL_RCC_BSEC_CLK_ENABLE();

  if (HAL_BSEC_ConfigDebug(&hbsec, &debugCfg) != HAL_OK)
  {
	  Error_Handler();
  }
  if (HAL_BSEC_UnlockDebug(&hbsec) != HAL_OK)
  {
	  Error_Handler();
  }
  /* USER CODE END BSEC_Init 0 */

  /* USER CODE BEGIN BSEC_Init 1 */

  /* USER CODE END BSEC_Init 1 */
  /* USER CODE BEGIN BSEC_Init 2 */

  /* USER CODE END BSEC_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
