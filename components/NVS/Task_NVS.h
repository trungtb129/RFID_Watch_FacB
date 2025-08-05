/**
  ******************************************************************************
  * @file    Task_NVS.h
  * @author  Ngoc Trung
  * @version V1.0.0
  * @date    25-July-2025
  * @brief   This file contains all the functions prototypes for the NVS Handles
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 Aubot
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TASK_NVS__
#define __TASK_NVS__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "Handle_NVS.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "ESP32_Infor.h"


/* Private define ------------------------------------------------------------*/
#define NVS_TASK_DELAY (200)


/* Exported functions --------------------------------------------------------*/  
/* Initialize NVS task functions *********************************************/
esp_err_t NVS_Config();

#ifdef __cplusplus
}
#endif

#endif /* __HANDLE_NVS_H__ */

/**
  * @}
  */

/**
  * @}
  */