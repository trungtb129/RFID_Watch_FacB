#include "Handle_NVS.h"

/**
  * @brief  Initializes the NVS flash storage for the ESP32.
  * @note   This function handles common initialization errors such as
  *         lack of free pages or NVS version mismatch by erasing and
  *         re-initializing the NVS partition.
  * @retval
  *         - ESP_OK: Initialization successful.
  *         - Other esp_err_t codes: Initialization failed.
  */
esp_err_t nvs_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        return ret;
    }
    return ESP_OK;
}

/**
  * @brief  Deinitializes the NVS (Non-Volatile Storage) flash system.
  * @note   This function releases any resources allocated by NVS.
  * @retval esp_err_t: 
  *         - ESP_OK on success.
  *         - Appropriate error code from esp_err_t enum otherwise.
  */
esp_err_t nvs_deinit(void)
{
    esp_err_t err = ESP_OK;
    // err = nvs_flash_deinit();
    if (err != ESP_OK) {
#if HANDLE_NVS_LOG 
        ESP_LOGE("NVS", "Failed to deinit NVS flash: %s", esp_err_to_name(err));
#endif /*HANDLE_NVS_LOG*/
    }
    return err;
}


/**
  * @brief  Read a structure from NVS with the given key.
  * @param  key:        The key under which the data is stored.
  * @param  data_out:   Pointer to the output buffer to store read data.
  * @param  structSize: Size of the expected struct to read.
  * @retval esp_err_t:
  *         - ESP_OK:        Data read successfully.
  *         - ESP_ERR_NVS_NOT_FOUND: No data found for the key.
  *         - Other:         NVS or flash error codes.
  */
esp_err_t read_nvs_data_struct(const char *key, void *data_out, size_t structSize) 
{
    nvs_handle_t handle;
    esp_err_t err;

    // Open NVS handle (non-volatile storage)
    err = nvs_open("storage", NVS_READONLY, &handle);
    if (err != ESP_OK) {
#if HANDLE_NVS_LOG 
        ESP_LOGE(TAG, "Failed to open NVS for reading: %s", esp_err_to_name(err));
#endif /*HANDLE_NVS_LOG*/
        return err;
    }

    // Get blob data from NVS
    size_t required_size = structSize;
    err = nvs_get_blob(handle, key, data_out, &required_size);
    if (err == ESP_OK) {
        if (required_size != structSize) {
#if HANDLE_NVS_LOG 
            ESP_LOGW(TAG, "Struct size mismatch: expected %d, got %d", (int)structSize, (int)required_size);
#endif /*HANDLE_NVS_LOG*/
            err = ESP_ERR_INVALID_SIZE;
        } else {
#if HANDLE_NVS_LOG 
            ESP_LOGI(TAG, "Successfully read struct from NVS");
#endif /*HANDLE_NVS_LOG*/
        }
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
#if HANDLE_NVS_LOG 
        ESP_LOGW(TAG, "Key '%s' not found in NVS", key);
#endif /*HANDLE_NVS_LOG*/
    } else {
#if HANDLE_NVS_LOG 
        ESP_LOGE(TAG, "Failed to read blob from NVS: %s", esp_err_to_name(err));
#endif /*HANDLE_NVS_LOG*/
    }
    // Close NVS
    err = nvs_commit(handle);

    nvs_close(handle);
    return err;
}


/**
  * @brief  Write a structure to NVS under the given key.
  * @param  key:        The key under which to store the data.
  * @param  data_in:    Pointer to the input struct to be saved.
  * @param  structSize: Size of the struct to write.
  * @retval esp_err_t:
  *         - ESP_OK:         Data written successfully.
  *         - Other:          NVS or flash error codes.
  */
esp_err_t write_nvs_data_struct(const char *key, void *data_in, size_t structSize)
{
    nvs_handle_t handle;
    esp_err_t err;

    // Open NVS in read-write mode
    err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
#if HANDLE_NVS_LOG
        ESP_LOGE(TAG, "Failed to open NVS for writing: %s", esp_err_to_name(err));
#endif /* HANDLE_NVS_LOG */
        return err;
    }

    // Set blob (struct) to NVS
    err = nvs_set_blob(handle, key, data_in, structSize);
    if (err != ESP_OK) {
#if HANDLE_NVS_LOG
        ESP_LOGE(TAG, "Failed to write blob to NVS: %s", esp_err_to_name(err));
#endif /* HANDLE_NVS_LOG */
        nvs_close(handle);
        return err;
    }

    // Commit changes
    err = nvs_commit(handle);
    if (err != ESP_OK) {
#if HANDLE_NVS_LOG
        ESP_LOGE(TAG, "Failed to commit changes to NVS: %s", esp_err_to_name(err));
#endif /* HANDLE_NVS_LOG */
        nvs_close(handle);
        return err;
    }

#if HANDLE_NVS_LOG
    ESP_LOGI(TAG, "Successfully wrote struct to NVS under key '%s'", key);
#endif /* HANDLE_NVS_LOG */

    // Close NVS handle
    nvs_close(handle);
    return ESP_OK;
}
