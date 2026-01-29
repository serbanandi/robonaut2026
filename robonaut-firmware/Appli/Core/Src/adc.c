/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    adc.c
 * @brief   This file provides code for the configuration
 *          of the ADC instances.
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
#include "adc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DMA_NodeTypeDef Node_GPDMA1_Channel2 __NON_CACHEABLE; // CRITICAL: DMA nodes must be placed in non-cacheable memory
DMA_QListTypeDef List_GPDMA1_Channel2;
DMA_HandleTypeDef handle_GPDMA1_Channel2;
DMA_NodeTypeDef Node_GPDMA1_Channel3 __NON_CACHEABLE; // CRITICAL: DMA nodes must be placed in non-cacheable memory
DMA_QListTypeDef List_GPDMA1_Channel3;
DMA_HandleTypeDef handle_GPDMA1_Channel3;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

    /* USER CODE BEGIN ADC1_Init 0 */

    RIFSC->RISC_SECCFGRx[2] |= 0x1;

    /* USER CODE END ADC1_Init 0 */

    ADC_MultiModeTypeDef multimode = { 0 };
    ADC_ChannelConfTypeDef sConfig = { 0 };
    ADC_AnalogWDGConfTypeDef AnalogWDGConfig = { 0 };

    /* USER CODE BEGIN ADC1_Init 1 */

    /* USER CODE END ADC1_Init 1 */

    /** Common config
     */
    hadc1.Instance = ADC1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.GainCompensation = 0;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.NbrOfConversion = 4;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    hadc1.Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure the ADC multi-mode
     */
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_5;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_246CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_7;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_11;
    sConfig.Rank = ADC_REGULAR_RANK_3;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_16;
    sConfig.Rank = ADC_REGULAR_RANK_4;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure the regular channel to be monitored by WatchDog 2 or 3
     */
    AnalogWDGConfig.FilteringConfig = ADC_AWD_FILTERING_NONE;

    if (HAL_ADC_AnalogWDGConfig(&hadc1, &AnalogWDGConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN ADC1_Init 2 */

    /* USER CODE END ADC1_Init 2 */
}
/* ADC2 init function */
void MX_ADC2_Init(void)
{

    /* USER CODE BEGIN ADC2_Init 0 */

    RIFSC->RISC_SECCFGRx[2] |= 0x1;

    /* USER CODE END ADC2_Init 0 */

    ADC_ChannelConfTypeDef sConfig = { 0 };
    ADC_AnalogWDGConfTypeDef AnalogWDGConfig = { 0 };

    /* USER CODE BEGIN ADC2_Init 1 */

    /* USER CODE END ADC2_Init 1 */

    /** Common config
     */
    hadc2.Instance = ADC2;
    hadc2.Init.Resolution = ADC_RESOLUTION_12B;
    hadc2.Init.GainCompensation = 0;
    hadc2.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc2.Init.LowPowerAutoWait = DISABLE;
    hadc2.Init.ContinuousConvMode = ENABLE;
    hadc2.Init.NbrOfConversion = 3;
    hadc2.Init.DiscontinuousConvMode = DISABLE;
    hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc2.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    hadc2.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc2.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    hadc2.Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(&hadc2) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_246CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_13;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure Regular Channel
     */
    sConfig.Channel = ADC_CHANNEL_14;
    sConfig.Rank = ADC_REGULAR_RANK_3;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure the regular channel to be monitored by WatchDog 2 or 3
     */
    AnalogWDGConfig.FilteringConfig = ADC_AWD_FILTERING_NONE;

    if (HAL_ADC_AnalogWDGConfig(&hadc2, &AnalogWDGConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN ADC2_Init 2 */

    /* USER CODE END ADC2_Init 2 */
}

static uint32_t HAL_RCC_ADC12_CLK_ENABLED = 0;

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    DMA_NodeConfTypeDef NodeConfig = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };
    if (adcHandle->Instance == ADC1)
    {
        /* USER CODE BEGIN ADC1_MspInit 0 */

        /* USER CODE END ADC1_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
        PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_HCLK;
        PeriphClkInitStruct.AdcDivider = 1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            Error_Handler();
        }

        /* ADC1 clock enable */
        HAL_RCC_ADC12_CLK_ENABLED++;
        if (HAL_RCC_ADC12_CLK_ENABLED == 1)
        {
            __HAL_RCC_ADC12_CLK_ENABLE();
        }

        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**ADC1 GPIO Configuration
        PF3     ------> ADC1_INP16
        PG15     ------> ADC1_INP7
        PA10     ------> ADC1_INP11
        PA8     ------> ADC1_INP5
        */
        GPIO_InitStruct.Pin = IR_3_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(IR_3_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = IR_4_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(IR_4_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = IR_2_Pin | IR_1_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* ADC1 DMA Init */
        /* GPDMA1_REQUEST_ADC1 Init */
        NodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
        NodeConfig.Init.Request = GPDMA1_REQUEST_ADC1;
        NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
        NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
        NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
        NodeConfig.Init.DestInc = DMA_DINC_INCREMENTED;
        NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_WORD;
        NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_WORD;
        NodeConfig.Init.SrcBurstLength = 1;
        NodeConfig.Init.DestBurstLength = 1;
        NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        NodeConfig.Init.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
        NodeConfig.Init.Mode = DMA_NORMAL;
        NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
        NodeConfig.TriggerConfig.TriggerSelection = GPDMA1_TRIGGER_GPDMA1_CH0_TCF;
        NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
        NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
        NodeConfig.SrcSecure = DMA_CHANNEL_SRC_SEC;
        NodeConfig.DestSecure = DMA_CHANNEL_DEST_SEC;
        if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel2) != HAL_OK)
        {
            Error_Handler();
        }

        if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel2, NULL, &Node_GPDMA1_Channel2) != HAL_OK)
        {
            Error_Handler();
        }

        if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel2) != HAL_OK)
        {
            Error_Handler();
        }

        handle_GPDMA1_Channel2.Instance = GPDMA1_Channel2;
        handle_GPDMA1_Channel2.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA1_Channel2.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
        handle_GPDMA1_Channel2.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
        handle_GPDMA1_Channel2.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
        handle_GPDMA1_Channel2.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
        if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel2) != HAL_OK)
        {
            Error_Handler();
        }

        if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel2, &List_GPDMA1_Channel2) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(adcHandle, DMA_Handle, handle_GPDMA1_Channel2);

        /* USER CODE BEGIN ADC1_MspInit 1 */

        HAL_PWREx_EnableVddA();

        /* USER CODE END ADC1_MspInit 1 */
    }
    else if (adcHandle->Instance == ADC2)
    {
        /* USER CODE BEGIN ADC2_MspInit 0 */

        /* USER CODE END ADC2_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
        PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_HCLK;
        PeriphClkInitStruct.AdcDivider = 1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            Error_Handler();
        }

        /* ADC2 clock enable */
        HAL_RCC_ADC12_CLK_ENABLED++;
        if (HAL_RCC_ADC12_CLK_ENABLED == 1)
        {
            __HAL_RCC_ADC12_CLK_ENABLE();
        }

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**ADC2 GPIO Configuration
        PA1     ------> ADC2_INP1
        PA2     ------> ADC2_INP14
        PA12     ------> ADC2_INP13
        */
        GPIO_InitStruct.Pin = BAT_U_Pin | MOT_I_Pin | MOT_U_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* ADC2 DMA Init */
        /* GPDMA1_REQUEST_ADC2 Init */
        NodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
        NodeConfig.Init.Request = GPDMA1_REQUEST_ADC2;
        NodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
        NodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
        NodeConfig.Init.SrcInc = DMA_SINC_FIXED;
        NodeConfig.Init.DestInc = DMA_DINC_INCREMENTED;
        NodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_WORD;
        NodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_WORD;
        NodeConfig.Init.SrcBurstLength = 1;
        NodeConfig.Init.DestBurstLength = 1;
        NodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        NodeConfig.Init.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
        NodeConfig.Init.Mode = DMA_NORMAL;
        NodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
        NodeConfig.TriggerConfig.TriggerSelection = GPDMA1_TRIGGER_GPDMA1_CH0_TCF;
        NodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
        NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
        NodeConfig.SrcSecure = DMA_CHANNEL_SRC_SEC;
        NodeConfig.DestSecure = DMA_CHANNEL_DEST_SEC;
        if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel3) != HAL_OK)
        {
            Error_Handler();
        }

        if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel3, NULL, &Node_GPDMA1_Channel3) != HAL_OK)
        {
            Error_Handler();
        }

        if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel3) != HAL_OK)
        {
            Error_Handler();
        }

        handle_GPDMA1_Channel3.Instance = GPDMA1_Channel3;
        handle_GPDMA1_Channel3.InitLinkedList.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA1_Channel3.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
        handle_GPDMA1_Channel3.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
        handle_GPDMA1_Channel3.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
        handle_GPDMA1_Channel3.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
        if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel3) != HAL_OK)
        {
            Error_Handler();
        }

        if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel3, &List_GPDMA1_Channel3) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(adcHandle, DMA_Handle, handle_GPDMA1_Channel3);

        /* USER CODE BEGIN ADC2_MspInit 1 */

        HAL_PWREx_EnableVddA();

        /* USER CODE END ADC2_MspInit 1 */
    }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

    if (adcHandle->Instance == ADC1)
    {
        /* USER CODE BEGIN ADC1_MspDeInit 0 */

        /* USER CODE END ADC1_MspDeInit 0 */
        /* Peripheral clock disable */
        HAL_RCC_ADC12_CLK_ENABLED--;
        if (HAL_RCC_ADC12_CLK_ENABLED == 0)
        {
            __HAL_RCC_ADC12_CLK_DISABLE();
        }

        /**ADC1 GPIO Configuration
        PF3     ------> ADC1_INP16
        PG15     ------> ADC1_INP7
        PA10     ------> ADC1_INP11
        PA8     ------> ADC1_INP5
        */
        HAL_GPIO_DeInit(IR_3_GPIO_Port, IR_3_Pin);

        HAL_GPIO_DeInit(IR_4_GPIO_Port, IR_4_Pin);

        HAL_GPIO_DeInit(GPIOA, IR_2_Pin | IR_1_Pin);

        /* ADC1 DMA DeInit */
        HAL_DMA_DeInit(adcHandle->DMA_Handle);
        /* USER CODE BEGIN ADC1_MspDeInit 1 */

        /* USER CODE END ADC1_MspDeInit 1 */
    }
    else if (adcHandle->Instance == ADC2)
    {
        /* USER CODE BEGIN ADC2_MspDeInit 0 */

        /* USER CODE END ADC2_MspDeInit 0 */
        /* Peripheral clock disable */
        HAL_RCC_ADC12_CLK_ENABLED--;
        if (HAL_RCC_ADC12_CLK_ENABLED == 0)
        {
            __HAL_RCC_ADC12_CLK_DISABLE();
        }

        /**ADC2 GPIO Configuration
        PA1     ------> ADC2_INP1
        PA2     ------> ADC2_INP14
        PA12     ------> ADC2_INP13
        */
        HAL_GPIO_DeInit(GPIOA, BAT_U_Pin | MOT_I_Pin | MOT_U_Pin);

        /* ADC2 DMA DeInit */
        HAL_DMA_DeInit(adcHandle->DMA_Handle);
        /* USER CODE BEGIN ADC2_MspDeInit 1 */

        /* USER CODE END ADC2_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
