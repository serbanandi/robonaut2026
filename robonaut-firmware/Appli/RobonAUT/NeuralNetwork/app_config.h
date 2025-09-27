/**
******************************************************************************
* @file    app_config.h
* @author  GPM Application Team
*
******************************************************************************
* @attention
*
* Copyright (c) 2023 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* ---------------    Generated code    ----------------- */
#ifndef APP_CONFIG
#define APP_CONFIG

#include "arm_math.h"

/* Postprocessing type configuration */
#define POSTPROCESS_TYPE    POSTPROCESS_OD_YOLO_V8_UF

#define NN_HEIGHT     (256)
#define NN_WIDTH      (256)
#define NN_BPP 3

/* Classes */
#define NB_CLASSES   (1)
#define CLASSES_TABLE const char* classes_table[NB_CLASSES] = {\
   "person"}\

/* Postprocessing YOLO_V8/V5u output float configuration */
#define AI_OD_YOLOV8_PP_NB_CLASSES        (1)
#define AI_OD_YOLOV8_PP_TOTAL_BOXES       (1344)
#define AI_OD_YOLOV8_PP_MAX_BOXES_LIMIT   (10)
#define AI_OD_YOLOV8_PP_CONF_THRESHOLD    (0.6)
#define AI_OD_YOLOV8_PP_IOU_THRESHOLD     (0.5)
#define AI_OD_YOLOV8_PP_SCALE             (0.0)
#define AI_OD_YOLOV8_PP_ZERO_POINT        (0)

#endif      /* APP_CONFIG */
