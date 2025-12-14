/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined ( __ICCARM__ )
#  define CMSE_NS_CALL  __cmse_nonsecure_call
#  define CMSE_NS_ENTRY __cmse_nonsecure_entry
#else
#  define CMSE_NS_CALL  __attribute((cmse_nonsecure_call))
#  define CMSE_NS_ENTRY __attribute((cmse_nonsecure_entry))
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32n6xx_hal.h"

#include "stm32n6xx_nucleo.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* Function pointer declaration in non-secure*/
#if defined ( __ICCARM__ )
typedef void (CMSE_NS_CALL *funcptr)(void);
#else
typedef void CMSE_NS_CALL (*funcptr)(void);
#endif

/* typedef for non-secure callback functions */
typedef funcptr funcptr_NS;

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define UI_SDA_Pin GPIO_PIN_1
#define UI_SDA_GPIO_Port GPIOC
#define IMU_MISO_Pin GPIO_PIN_4
#define IMU_MISO_GPIO_Port GPIOC
#define PWR_EN_Pin GPIO_PIN_10
#define PWR_EN_GPIO_Port GPIOD
#define MOT_PWM2_Pin GPIO_PIN_9
#define MOT_PWM2_GPIO_Port GPIOE
#define SERVO_FRONT_Pin GPIO_PIN_12
#define SERVO_FRONT_GPIO_Port GPIOC
#define OV_VSYNC_Pin GPIO_PIN_2
#define OV_VSYNC_GPIO_Port GPIOB
#define ENC_A_Pin GPIO_PIN_13
#define ENC_A_GPIO_Port GPIOD
#define _AD_CS4_Pin GPIO_PIN_8
#define _AD_CS4_GPIO_Port GPIOC
#define UI_SCL_Pin GPIO_PIN_9
#define UI_SCL_GPIO_Port GPIOH
#define GEN_RX_Pin GPIO_PIN_2
#define GEN_RX_GPIO_Port GPIOC
#define SERVO_REAR_Pin GPIO_PIN_6
#define SERVO_REAR_GPIO_Port GPIOD
#define GEN_SCK_Pin GPIO_PIN_9
#define GEN_SCK_GPIO_Port GPIOB
#define OV_FLASH_Pin GPIO_PIN_15
#define OV_FLASH_GPIO_Port GPIOB
#define OV_FLASHH2_Pin GPIO_PIN_2
#define OV_FLASHH2_GPIO_Port GPIOH
#define GEN_MOSI_Pin GPIO_PIN_3
#define GEN_MOSI_GPIO_Port GPIOC
#define MOT_PWM1_Pin GPIO_PIN_11
#define MOT_PWM1_GPIO_Port GPIOE
#define TEL_TX_Pin GPIO_PIN_8
#define TEL_TX_GPIO_Port GPIOD
#define TRACED1_Pin GPIO_PIN_0
#define TRACED1_GPIO_Port GPIOB
#define TRACED3_Pin GPIO_PIN_7
#define TRACED3_GPIO_Port GPIOB
#define TRACED2_Pin GPIO_PIN_6
#define TRACED2_GPIO_Port GPIOB
#define OV_RST_Pin GPIO_PIN_4
#define OV_RST_GPIO_Port GPIOH
#define TRACECLK_Pin GPIO_PIN_3
#define TRACECLK_GPIO_Port GPIOB
#define TRACED0_Pin GPIO_PIN_3
#define TRACED0_GPIO_Port GPIOE
#define CMD_RX_Pin GPIO_PIN_0
#define CMD_RX_GPIO_Port GPIOE
#define GEN_PIN1_Pin GPIO_PIN_7
#define GEN_PIN1_GPIO_Port GPIOP
#define UI_CONF_Pin GPIO_PIN_6
#define UI_CONF_GPIO_Port GPIOP
#define MOT_EN_Pin GPIO_PIN_0
#define MOT_EN_GPIO_Port GPIOP
#define TEL_RX_Pin GPIO_PIN_9
#define TEL_RX_GPIO_Port GPIOD
#define UI_B_Pin GPIO_PIN_4
#define UI_B_GPIO_Port GPIOP
#define MOT_STATUS_Pin GPIO_PIN_1
#define MOT_STATUS_GPIO_Port GPIOP
#define UI_PUSH_Pin GPIO_PIN_5
#define UI_PUSH_GPIO_Port GPIOP
#define UI_A_Pin GPIO_PIN_3
#define UI_A_GPIO_Port GPIOP
#define UI_BACK_Pin GPIO_PIN_2
#define UI_BACK_GPIO_Port GPIOP
#define _LINE_LED_OE_Pin GPIO_PIN_1
#define _LINE_LED_OE_GPIO_Port GPIOO
#define LINE_INF_LE_Pin GPIO_PIN_2
#define LINE_INF_LE_GPIO_Port GPIOO
#define IMU_CS_Pin GPIO_PIN_7
#define IMU_CS_GPIO_Port GPIOG
#define _AD_CS3_Pin GPIO_PIN_6
#define _AD_CS3_GPIO_Port GPIOG
#define GEN_PIN2_Pin GPIO_PIN_8
#define GEN_PIN2_GPIO_Port GPIOP
#define IR_3_Pin GPIO_PIN_3
#define IR_3_GPIO_Port GPIOF
#define _LINE_INF_OE_Pin GPIO_PIN_3
#define _LINE_INF_OE_GPIO_Port GPIOO
#define LINE_LED_LE_Pin GPIO_PIN_0
#define LINE_LED_LE_GPIO_Port GPIOO
#define IMU_SCK_Pin GPIO_PIN_2
#define IMU_SCK_GPIO_Port GPIOF
#define IMU_MOSI_Pin GPIO_PIN_9
#define IMU_MOSI_GPIO_Port GPIOP
#define _AD_CS2_Pin GPIO_PIN_4
#define _AD_CS2_GPIO_Port GPIOG
#define BAT_U_Pin GPIO_PIN_1
#define BAT_U_GPIO_Port GPIOA
#define JTDI_Pin GPIO_PIN_15
#define JTDI_GPIO_Port GPIOA
#define GEN_TX_Pin GPIO_PIN_3
#define GEN_TX_GPIO_Port GPIOG
#define IR_4_Pin GPIO_PIN_15
#define IR_4_GPIO_Port GPIOG
#define IR_2_Pin GPIO_PIN_10
#define IR_2_GPIO_Port GPIOA
#define RX_PWM_Pin GPIO_PIN_1
#define RX_PWM_GPIO_Port GPIOG
#define GEN_TIM_Pin GPIO_PIN_1
#define GEN_TIM_GPIO_Port GPIOB
#define _AD_CS1_Pin GPIO_PIN_3
#define _AD_CS1_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define MOT_I_Pin GPIO_PIN_2
#define MOT_I_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define MOT_U_Pin GPIO_PIN_12
#define MOT_U_GPIO_Port GPIOA
#define IR_1_Pin GPIO_PIN_8
#define IR_1_GPIO_Port GPIOA
#define ENC_B_Pin GPIO_PIN_13
#define ENC_B_GPIO_Port GPIOG
#define SWO_Pin GPIO_PIN_5
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
