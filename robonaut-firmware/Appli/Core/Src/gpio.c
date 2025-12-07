/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PF3   ------> ADC1_INP16
     PA1   ------> ADC2_INP1
     PG15   ------> ADC1_INP7
     PA10   ------> ADC1_INP11
     PA14(JTCK/SWCLK)   ------> DEBUG_JTCK-SWCLK
     PA2   ------> ADC2_INP14
     PA13(JTMS/SWDIO)   ------> DEBUG_JTMS-SWDIO
     PA12   ------> ADC2_INP13
     PA8   ------> ADC1_INP5
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOP_CLK_ENABLE();
  __HAL_RCC_GPIOO_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PWR_EN_GPIO_Port, PWR_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(_AD_CS4_GPIO_Port, _AD_CS4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OV_FLASH_GPIO_Port, OV_FLASH_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, OV_FLASHH2_Pin|OV_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MOT_EN_GPIO_Port, MOT_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOO, _LINE_LED_OE_Pin|LINE_INF_LE_Pin|_LINE_INF_OE_Pin|LINE_LED_LE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, _AD_CS3_Pin|_AD_CS2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(_AD_CS1_GPIO_Port, _AD_CS1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PWR_EN_Pin */
  GPIO_InitStruct.Pin = PWR_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PWR_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OV_VSYNC_Pin */
  GPIO_InitStruct.Pin = OV_VSYNC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OV_VSYNC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : _AD_CS4_Pin */
  GPIO_InitStruct.Pin = _AD_CS4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(_AD_CS4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OV_FLASH_Pin */
  GPIO_InitStruct.Pin = OV_FLASH_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OV_FLASH_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OV_FLASHH2_Pin OV_RST_Pin */
  GPIO_InitStruct.Pin = OV_FLASHH2_Pin|OV_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : GEN_PIN1_Pin UI_CONF_Pin UI_B_Pin MOT_STATUS_Pin
                           UI_PUSH_Pin UI_A_Pin UI_BACK_Pin GEN_PIN2_Pin */
  GPIO_InitStruct.Pin = GEN_PIN1_Pin|UI_CONF_Pin|UI_B_Pin|MOT_STATUS_Pin
                          |UI_PUSH_Pin|UI_A_Pin|UI_BACK_Pin|GEN_PIN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOP, &GPIO_InitStruct);

  /*Configure GPIO pin : MOT_EN_Pin */
  GPIO_InitStruct.Pin = MOT_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MOT_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : _LINE_LED_OE_Pin LINE_INF_LE_Pin _LINE_INF_OE_Pin LINE_LED_LE_Pin */
  GPIO_InitStruct.Pin = _LINE_LED_OE_Pin|LINE_INF_LE_Pin|_LINE_INF_OE_Pin|LINE_LED_LE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOO, &GPIO_InitStruct);

  /*Configure GPIO pins : _AD_CS3_Pin _AD_CS2_Pin */
  GPIO_InitStruct.Pin = _AD_CS3_Pin|_AD_CS2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : IR_3_Pin */
  GPIO_InitStruct.Pin = IR_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IR_3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BAT_U_Pin IR_2_Pin MOT_I_Pin MOT_U_Pin
                           IR_1_Pin */
  GPIO_InitStruct.Pin = BAT_U_Pin|IR_2_Pin|MOT_I_Pin|MOT_U_Pin
                          |IR_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : IR_4_Pin */
  GPIO_InitStruct.Pin = IR_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IR_4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : _AD_CS1_Pin */
  GPIO_InitStruct.Pin = _AD_CS1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(_AD_CS1_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
