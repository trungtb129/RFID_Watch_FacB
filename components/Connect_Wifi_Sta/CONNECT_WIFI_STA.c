/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "CONNECT_WIFI_STA.h"


#ifdef TEST_UHF_RFID_WATCH

QueueHandle_t x_WiFi_2_LCD_Task_Queue;      //Gửi dữ liệu lên màn hình LCD

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

esp_netif_ip_info_t ip_info_static;

static uint8_t isConnectedWifi = 0;

// Prototype function
static uint8_t convertStringToIPNumber(char* stringIP, uint8_t* num1, uint8_t* num2, uint8_t* num3, uint8_t* num4);

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


static const char *TAG = "wifi_station";

static int s_retry_num = 0;

wifi_ap_record_t ap_info;

/* Function to set static IP for this device */
static void set_static_IPv4(void) {
#ifdef UHF_ESP_USING_STATIC_IP
    uint8_t staticIP1, staticIP2, staticIP3, staticIP4;
    uint8_t gateWay1, gateWay2, gateWay3, gateWay4;
    uint8_t subnetMask1, subnetMask2, subnetMask3, subnetMask4;

    // // Thiết lập thông tin IP tĩnh
    // // char temp1[] = CONFIG_STATIC_DEVICE_IP_ADDR;
    // char temp1[] = "10.1.3.30";
    // // char temp2[] = "255.255.252.0";
    // // char temp3[] = "10.0.9.1";
    // char temp2[] = "255.255.0.0";
    // char temp3[] = "10.1.0.1";

    // Thiết lập thông tin IP tĩnh
    char temp1[] = UHF_ESP_STATIC_IP;
    char temp2[] = UHF_ESP_STATIC_SUBNET_MASK;
    char temp3[] = UHF_ESP_STATIC_GW;
    
    convertStringToIPNumber(temp1, &staticIP1, &staticIP2, &staticIP3, &staticIP4);
    convertStringToIPNumber(temp2, &subnetMask1, &subnetMask2, &subnetMask3, &subnetMask4);
    convertStringToIPNumber(temp3, &gateWay1, &gateWay2, &gateWay3, &gateWay4);

    IP4_ADDR(&ip_info_static.ip, staticIP1, staticIP2, staticIP3, staticIP4); // Địa chỉ IP tĩnh bạn muốn sử dụng
    IP4_ADDR(&ip_info_static.gw, gateWay1, gateWay2, gateWay3, gateWay4);   // Gateway
    IP4_ADDR(&ip_info_static.netmask, subnetMask1, subnetMask2, subnetMask3, subnetMask4); // Subnet mask
    
    esp_netif_dhcpc_stop(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"));
    esp_netif_set_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info_static);
#endif
}

/* Function to scan for available APs and connect to the one with the strongest signal */
static void scan_and_connect(void)
{
    uint16_t ap_count = 0;
    wifi_ap_record_t ap_records[MAXIMUM_SCAN_APP];

    // Scan for available APs
    // ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    if (esp_wifi_scan_start(NULL, true) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Wi-Fi scan");
        return;
    }
    // ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    if (esp_wifi_scan_get_ap_num(&ap_count) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get num ap Wi-Fi scan");
        return;
    }

    // if(ap_count > MAXIMUM_SCAN_APP) ap_count = MAXIMUM_SCAN_APP;

    // ap_records = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_count);
    // if (ap_records == NULL)
    // {
    //     ESP_LOGE(TAG, "Can't malloc Ap_Records");
    //     return ;
    // }
    // ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_records)); 
//      *    - ESP_OK: succeed
//   *    - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
//   *    - ESP_ERR_WIFI_NOT_STARTED: WiFi is not started by esp_wifi_start
//   *    - ESP_ERR_INVALID_ARG: invalid argument
//   *    - ESP_ERR_NO_MEM: out of memory
    esp_err_t scan_err;
    if(ap_count <= MAXIMUM_SCAN_APP) {
        scan_err = esp_wifi_scan_get_ap_records(&ap_count, ap_records);
        if (scan_err == ESP_OK) {
            // Display the list of available APs
            ESP_LOGI(TAG, "Found %d access points:", ap_count);
        }
        else {
            ESP_LOGE(TAG, "Fail to scan - Error: %s", esp_err_to_name(scan_err));
            return ;
        }
    }
    else {
        ESP_LOGE(TAG, "Fail to scan - Error: %s", "Over sizeof arr scan AP");
        return;
    }
    
    for (int i = 0; i < ap_count; i++) {
        ESP_LOGI(TAG, "SSID: %s, RSSI: %d, BSSID: %02x:%02x:%02x:%02x:%02x:%02x", ap_records[i].ssid, ap_records[i].rssi, 
        ap_records[i].bssid[0], ap_records[i].bssid[1], ap_records[i].bssid[2], ap_records[i].bssid[3], ap_records[i].bssid[4], ap_records[i].bssid[5]);
    }

    // Find the AP with the strongest signal
    int best_rssi = RSSI_THRESHOLD;
    int best_ap_index = -1;
    for (int i = 0; i < ap_count; i++) {
        //Kiểm tra điều kiện chất lượng RSSI lớn hơn hiện tại, trùng SSID và khác BSSID   
        // = 1 by menuconfig = 2 by software   
        if (ap_records[i].rssi > best_rssi && strcmp((char*)ap_records[i].ssid, ESP_WIFI_SSID) == 0 && memcmp(ap_records[i].bssid, ap_info.bssid, 6) != 0) {
            best_rssi = ap_records[i].rssi;
            best_ap_index = i;
        }
         
        
    }

    set_static_IPv4();

    if (best_ap_index != -1) {
        // Connect to the AP with the strongest signal
        wifi_config_t wifi_config = {
                .sta = {
                    .ssid = ESP_WIFI_SSID,
                    .password = ESP_WIFI_PASS,
                    .btm_enabled = true,
                    .ft_enabled = true,
                    .mbo_enabled = true,
                    //Kiểm tra RSSID của AP
                    .bssid_set = 1,
                    .bssid = {0, 0, 0, 0, 0, 0},
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                }
        };

        //strncpy((char *)wifi_config.sta.ssid, (char *)ap_records[best_ap_index].ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.bssid, ap_records[best_ap_index].bssid, 6);

        ESP_LOGW(TAG, "Connecting to AP with SSID: %s, RSSI: %d, BSSID: %02x:%02x:%02x:%02x:%02x:%02x", wifi_config.sta.ssid, best_rssi,
                    ap_records[best_ap_index].bssid[0], ap_records[best_ap_index].bssid[1], 
                    ap_records[best_ap_index].bssid[2], ap_records[best_ap_index].bssid[3], 
                    ap_records[best_ap_index].bssid[4], ap_records[best_ap_index].bssid[5]);
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start()); //esp_wifi_connect

    } else {
        ESP_LOGW(TAG, "No AP found with better RSSI.");
    }

    //free(ap_records);
}

//Struct nhận thông tin từ Center
center_infor_typedef xWifi_CenterInfor;

static void check_rssi_and_roam(void *arg)
{
    static bool wifi_connected = true; // Trạng thái hiện tại của kết nối Wi-Fi
    
    lcd_wifi_info_t info = {};
    while (1) {
        // ESP_LOGI(TAG, "Check  static IP: %s\n", Wifi_ESP32Parameter.static_IP);
        // ESP_LOGI(TAG, "Check Gateway IP: %s\n", Wifi_ESP32Parameter.gateWay);
        // ESP_LOGI(TAG, "Check SubnetMask: %s\n", Wifi_ESP32Parameter.subnetMask);

        //Nhận thông tin từ Center
        if(x_Center_Infor_Queue != NULL) {
            if(xQueuePeek(x_Center_Infor_Queue, &xWifi_CenterInfor, 10 / portTICK_PERIOD_MS) == pdTRUE) { 

            }
        }

        if (xWifi_CenterInfor.operationMode == SLEEP_MODE) { // sleep mode
            if (wifi_connected) {
                ESP_LOGI(TAG, "Entering sleep mode, disconnecting Wi-Fi to save power");
                esp_wifi_disconnect(); // Ngắt kết nối Wi-Fi
                esp_wifi_stop();       // Dừng Wi-Fi
                wifi_connected = false;
            }
            vTaskDelay(pdMS_TO_TICKS(5000)); // Delay 5 giây
            continue;
        } else if (xWifi_CenterInfor.operationMode != SLEEP_MODE) { // normal mode
            if (!wifi_connected) {
                ESP_LOGI(TAG, "Exiting sleep mode, reconnecting Wi-Fi");
                esp_wifi_start();      // Khởi động lại Wi-Fi
                wifi_connected = true;
            }
        }

        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            // ESP_LOGI(TAG, "RSSI: %d dBm, BSSID: %02x:%02x:%02x:%02x:%02x:%02x", 
            //          ap_info.rssi,
            //          ap_info.bssid[0], ap_info.bssid[1], ap_info.bssid[2],
            //          ap_info.bssid[3], ap_info.bssid[4], ap_info.bssid[5]);

            info.rssi = ap_info.rssi;
            info.isConnectedWifi = isConnectedWifi;
            if (x_WiFi_2_LCD_Task_Queue)
            {
                xQueueOverwrite( x_WiFi_2_LCD_Task_Queue, &info);
            }
            
            
            if (ap_info.rssi < RSSI_THRESHOLD) {
                ESP_LOGI(TAG, "RSSI below threshold (%d dBm), scanning for better AP", RSSI_THRESHOLD);
                scan_and_connect();
            }

            
        } else {
            ESP_LOGE(TAG, "Failed to get AP info");
        }

        vTaskDelay(pdMS_TO_TICKS(3000)); // Delay 3 seconds
    }
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        isConnectedWifi = 0;
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            isConnectedWifi = 0;
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        isConnectedWifi = 1;
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // // Thiết lập thông tin IP tĩnh
    // IP4_ADDR(&ip_info_static.ip, 10, 0, 12, 51); // Địa chỉ IP tĩnh bạn muốn sử dụng
    // IP4_ADDR(&ip_info_static.gw, 10, 0, 12, 1);   // Gateway
    // IP4_ADDR(&ip_info_static.netmask, 255, 255, 252, 0); // Subnet mask
    // esp_netif_dhcpc_stop(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"));
    // esp_netif_set_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info_static);

    //     // Thiết lập thông tin IP tĩnh
    // IP4_ADDR(&ip_info_static.ip, 192, 168, 43, 115); // Địa chỉ IP tĩnh bạn muốn sử dụng
    // IP4_ADDR(&ip_info_static.gw, 192, 168, 43, 1);   // Gateway
    // IP4_ADDR(&ip_info_static.netmask, 255, 255, 255, 0); // Subnet mask
    // esp_netif_dhcpc_stop(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"));
    // esp_netif_set_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info_static);

    // IP4_ADDR(&ip_info_static.ip, 10, 0, 9, 109); // Địa chỉ IP tĩnh bạn muốn sử dụng
    // IP4_ADDR(&ip_info_static.gw, 10, 0, 9, 1);   // Gateway
    // IP4_ADDR(&ip_info_static.netmask, 255, 255, 252, 0); // Subnet mask

    set_static_IPv4();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    
    wifi_config_t wifi_config;     

    wifi_config_t wifi_config_temp1 = {
    .sta = {
        .ssid = ESP_WIFI_SSID,   //RFID_Zone2    IPhone của Trung    Redmi 9T        FA_KDS
        .password = ESP_WIFI_PASS, //Shws2023  atrung123           11111111        Shws2023
        .btm_enabled = true,  // Enable BSS Transition Management
        .ft_enabled = true,   // Enable Fast Transition (802.11r)
        .mbo_enabled = true,  // Enable MBO (Management Frame Protection)
        /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
            * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
            * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
            * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
            */
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        // .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        // .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
        // .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
    },
    };  
    // printf("SSID: %s\n",wifi_config_temp1.sta.ssid);

    // printf("SSID: ");
    // for(uint8_t i = 0; i < 32 ; i++) {
    //     printf(" %d",wifi_config_temp1.sta.ssid[i]);
    // }
    // printf("\n");
    // printf("PASSWORD: ");
    // for(uint8_t i = 0; i < 64 ; i++) {
    //     printf(" %d",wifi_config_temp1.sta.password[i]);
    // }
    // printf("\n");
    wifi_config = wifi_config_temp1;
  
    printf("SSID: %s\n",wifi_config.sta.ssid);
    printf("PASSWORD: %s\n",wifi_config.sta.password);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    
}

void CONNECT_WIFI_Init(void) {
#if 1
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    x_WiFi_2_LCD_Task_Queue = xQueueCreate( 1, sizeof(lcd_wifi_info_t));
    if (x_WiFi_2_LCD_Task_Queue == NULL)
    {
        ESP_LOGE(TAG, "FAILED to create x_WiFi_2_LCD_Task_Queue queue.");
    }

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    // Create a task to check RSSI and handle roaming
    xTaskCreate(check_rssi_and_roam, "check_rssi_and_roam", 1024 * 5, NULL, 5, NULL);
#endif

}

// Return 0: OK
// Return 1: Error
static uint8_t convertStringToIPNumber(char* stringIP, uint8_t* num1, uint8_t* num2, uint8_t* num3, uint8_t* num4) {

    char *token = strtok(stringIP, ".");

    if (token != NULL) {
        *num1 = atoi(token);
        token = strtok(NULL, ".");
        if (token != NULL) {
            *num2 = atoi(token);
            token = strtok(NULL, ".");
            if (token != NULL) {
                *num3 = atoi(token);
                token = strtok(NULL, ".");
                if (token != NULL) {
                    *num4 = atoi(token);
                } else {
                    printf("Invalid IP address format\n");
                    return 1;
                }
            } else {
                printf("Invalid IP address format\n");
                return 1;
            }
        } else {
            printf("Invalid IP address format\n");
            return 1;
        }
    } else {
        printf("Invalid IP address format\n");
        return 1;
    }

    //printf("a = %d, b = %d, c = %d, d = %d\n", a, b, c, d);

    return 0;
}

#endif

