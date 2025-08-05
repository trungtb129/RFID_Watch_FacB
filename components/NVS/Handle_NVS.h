/**
  ******************************************************************************
  * @file    Handle_NVS.h
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
#ifndef __HANDLE_NVS_H__
#define __HANDLE_NVS_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"


/* Private define ------------------------------------------------------------*/
#define HANDLE_NVS_LOG (0)


/* Exported functions --------------------------------------------------------*/  
/* Initialize NVS functions *********************************************/
esp_err_t nvs_init(void);

/* Deinitialize NVS functions *********************************************/
esp_err_t nvs_deinit(void);

/* Read Struct from NVS functions *********************************************/
esp_err_t read_nvs_data_struct(const char *key, void *data_out, size_t structSize);

/* Write Struct from NVS functions *********************************************/
esp_err_t write_nvs_data_struct(const char *key, void *data_in, size_t structSize);

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