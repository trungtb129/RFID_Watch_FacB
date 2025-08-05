#include "Task_NVS.h"

/* Private macro ---------------------------------------------------------*/
#define TAG "TASK_NVS"

/* Private variables ---------------------------------------------------------*/
center_param_typedef nvs_param;
center_param_typedef nvs_center_param;
center_infor_typedef nvs_center_infor;
QueueHandle_t x_NVS_To_Center_Queue = NULL;
const char key_list_NVS[][30] = {
  "NVS_Param",
};
center_param_typedef default_param= {.mode = CL01_MODE};

/* Private function prototypes -----------------------------------------------*/
static void NVS_Task(void *arg);

/* Private functions ---------------------------------------------------------*/
/**
 * @brief Initializes and creates the NVS task.
 * 
 * This function is responsible for starting the task that handles NVS operations.
 * It creates the task with a predefined stack size and priority. If task creation
 * fails, it returns ESP_FAIL.
 * 
 * @return esp_err_t 
 *         - ESP_OK if the task was created successfully
 *         - ESP_FAIL if task creation failed
 */
esp_err_t NVS_Config() {
    esp_err_t ret = nvs_init();
    if (ret != ESP_OK) 
        return ESP_FAIL;

    x_NVS_To_Center_Queue = xQueueCreate( 1, sizeof(center_param_typedef) );
    if (x_NVS_To_Center_Queue == NULL)
        return ESP_FAIL;

    BaseType_t result = xTaskCreate(&NVS_Task, "nvs_task", 1024*4, NULL, configMAX_PRIORITIES - 3, NULL);
    if (result != pdPASS) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

/**
 * @brief Main loop for the NVS_Task.
 * 
 * This task runs in an infinite loop with a fixed delay. It can be extended to include
 * periodic operations related to NVS interactions as needed.
 * 
 * @param arg Task parameter (unused)
 */
static void NVS_Task(void *arg)
{
    if(x_NVS_To_Center_Queue == NULL) {
        ESP_LOGE(TAG, "Failed to create x_NVS_To_Center_Queue");
        esp_restart();
    }
    if(read_nvs_data_struct(key_list_NVS[0], &nvs_param, sizeof(center_param_typedef)) == ESP_OK) {
        if(x_NVS_To_Center_Queue != NULL) {
            xQueueOverwrite(x_NVS_To_Center_Queue, &nvs_param);
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    else {
        if(write_nvs_data_struct(key_list_NVS[0], &default_param, sizeof(center_param_typedef)) == ESP_OK) {
            memcpy(&nvs_param, &default_param, sizeof(center_param_typedef));
            if(x_NVS_To_Center_Queue != NULL) {
                xQueueOverwrite(x_NVS_To_Center_Queue, &nvs_param);
            }
        }        
        else {
            ESP_LOGE(TAG, "Failed to write blob to NVS");
            esp_restart();
        }
    }
    while (1) {
            //Nhận thông tin từ Center
        if(x_Center_Param_Queue != NULL) {
            //// Peek thành công: xUHF_RFID_CenterInfor chứa thông tin mới từ Center
            if(xQueuePeek(x_Center_Param_Queue, &nvs_center_param, 10 / portTICK_PERIOD_MS) == pdPASS) { 
                if(memcmp(&nvs_center_param, &nvs_param, sizeof(center_param_typedef)) != 0) {
                    if(write_nvs_data_struct(key_list_NVS[0], &nvs_center_param, sizeof(center_param_typedef)) == ESP_OK) 
                        memcpy(&nvs_param, &nvs_center_param, sizeof(center_param_typedef));
                    else
                        ESP_LOGE(TAG, "Failed to write blob to NVS");
                }
            }
        }  

            //Nhận thông tin từ Center
        if(x_Center_Infor_Queue != NULL) {
            //// Peek thành công: xUHF_RFID_CenterInfor chứa thông tin mới từ Center
            if(xQueuePeek(x_Center_Infor_Queue, &nvs_center_infor, 10 / portTICK_PERIOD_MS) == pdPASS) { 
            }
        }  


        // Delay for a fixed period before the next execution cycle
        vTaskDelay(NVS_TASK_DELAY / portTICK_PERIOD_MS);
    }
}
