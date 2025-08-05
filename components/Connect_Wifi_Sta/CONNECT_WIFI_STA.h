#ifndef __CONNECT_WIFI_STA_H__
#define __CONNECT_WIFI_STA_H__
#include <stdint.h>
#include "string.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "sdkconfig.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>            // struct addrinfo
#include <arpa/inet.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "ESP32_Infor.h"


#define TEST_UHF_RFID_WATCH

#ifdef TEST_UHF_RFID_WATCH



void CONNECT_WIFI_Init(void);
#endif

//Reconnect to a wifi 
#define RSSI_THRESHOLD -79
#define MAXIMUM_SCAN_APP 20

#define EXAMPLE_ESP_MAXIMUM_RETRY 5

#ifndef TEST_UHF_RFID_WATCH
/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

//Dynamic IP - 0 || Static IP - 1
#ifdef CONFIG_DYNAMIC_IP_DHCP 
  #define CONNECT_TO_WIFI_IP_MODE 0
#elif CONFIG_STATIC_IP_DHCP 
  #define CONNECT_TO_WIFI_IP_MODE 1
#endif


void CONNECT_WIFI_Init(void);
#endif
#endif