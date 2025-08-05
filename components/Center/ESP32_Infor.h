#ifndef _ESP32_INFOR_H
#define _ESP32_INFOR_H
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/adc.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"



#include "ESP32_Config.h"

/*Định nghĩa định dạng màu tạm thời hoặc chính thức (Chính thức là màu dữ liệu gửi về từ server, 
tạm thời là màu được cộng vào để thêm thông tin do có trùng 4 trường thông tin) */
typedef enum {
    OFFICIAL_WIRE_COLOR = 1,
    TEMPORARY_WIRE_COLOR = 2,
} wire_color_status_t;

/*Định nghĩa các struct quản lý dữ liệu hiển thị */
typedef struct {
    char main_color[6];      // Màu chính, tối đa 5 ký tự + '\0'
    char sub_color[6];       // Màu phụ, tối đa 5 ký tự + '\0'
    wire_color_status_t colorStatus;    //Trạng thái màu chính thức/tạm thời
} wire_color_t;

typedef struct {
    wire_color_t color[5];   // Tối đa 5 cặp màu
    uint8_t color_count;     // Số lượng màu đã nhận (tổng cả chính thức và tạm thời)
} ColorPairs;

//Struct quản lý thông tin hoạt động của wifi
typedef struct {
  char c;   // Test
  uint8_t isConnectedWifi;
  int8_t  rssi;
} lcd_wifi_info_t;
#if 0 // truong thong tin cu
typedef struct {
    char status_card[6];
    char serial[MAX_EPC_LENGTH];
    char location[8];
    ColorPairs colors;
    char length[8];
    char line[8];
    char date[8];
    char lo[8];
    char group[8];
    char SA[8];
    char PartNumber[64];
} wire_infor; 
#endif

typedef struct {
    char status_card[6];//1
    char serial[MAX_EPC_LENGTH];//2
    char line[8];//3
    char PartNumber[64];//4
    char WireType[8];//5
    char WireSize[8];//6
    ColorPairs colors;//7
    char length[8];//8
    char group[8];//9
    char WO[12];//10
    char lot[8];//11
    char location[8];//12
    char factory[4];//13
    char method[8];//14
    char CartNo[24];//15
    char comment[16];//16
} wire_infor;

//Struct quản lý các dây đang được hiển thị trên màn hình
typedef struct {
    wire_infor WireInfo;
    uint8_t isOnScreen;         // Bằng 0: Không hiển thị || Bằng 1: Đang hiển thị
    uint8_t isTheBestRSSI;      // Bằng 0: Thẻ dây bình thường || Bằng 1: Thẻ đang có giá trị RSSI tốt nhất
} Manage_CardInfo_LCD_t;

//Struct quản lý thông tin hoạt động của màn hình
typedef struct {
    uint8_t numberOfCardOnScreen;
} LCD_Infor_TypeDef;

/************************************************/

/*Định nghĩa các struct quản lý dữ liệu đọc thẻ */
typedef struct {
    uint8_t result_cards;
    int8_t rssi;
    uint8_t pc[2];
    uint8_t epc[MAX_EPC_LENGTH];
    char rssi_str[20];
    char pc_str[50];
    char epc_str[50];
    uint8_t epc_len;
} CARD;

//Struct quản lý những thẻ hiện đang được đọc
typedef struct {
  uint8_t epc[12];            //Mã thẻ
  uint8_t epc_len;            //Chiều dài dữ liệu thẻ
  uint8_t isAlive;            //Cờ trạng thái thẻ có đang được đọc không
  uint8_t isHasInfor;         //Cờ trạng thái thẻ đang có thông tin hay không? (Thông tin trả về từ server Socket)
  uint8_t isDuplicateInfor;    //Cờ báo trạng thái bị trùng 4 trường thông tin với thẻ Alive khác
  int8_t current_rssi;          //Giá trị RSSI hiện tại
  uint8_t theBestRSSIValue;     //Nếu được set lên 1, tức đây là thẻ có giá trị RSSI tốt nhất (chỉ xét với thẻ có thông tin tức isHasInfor == 1)
  uint32_t lastAliveTime;     //Thời gian cuối cùng đọc được thẻ
  uint8_t  isSendToServer;    //Cờ trạng thái thẻ đã được gửi lên server hay chưa
} manage_Card_Alive_t;
//Struct quản lý thẻ nào đang ở chế độ IN
typedef struct {
    char epc[16]; // Mã thẻ
    uint8_t isInMode; // Thêm biến này
} UHF_TO_LCD_DATA;

/************************************************/

//Cấu hình thông tin quản lý chế độ làm việc
typedef enum {
    IN_MODE = 0,
    OUT_MODE,
    SLEEP_MODE,
    MODEn,
}   operationMode_t;

typedef struct {
    operationMode_t operationMode;
}   workMode_t;

typedef enum {
    NOT_CONNECT = 0,
    CONNECTED,
} sockettcp_connect_status;

//Struct quản lý thông tin hoạt động của socket
typedef struct  {
    sockettcp_connect_status status;
} sockettcp_info;

/************************************************/
//Quản lý việc chuyển chế độ giữa line mode và sa mode
typedef enum {
    CL01_MODE = 0,
    CL03_MODE,
    CLMODEn,
}  Work_Mode;

typedef enum {
    PN_Mode = 0,
    Lot_Mode,
}  ScreenCL_Mode;

typedef struct {
    Work_Mode mode;
}   center_param_typedef;
/************************************************/

typedef struct {
    operationMode_t operationMode;
    sockettcp_info socketStatus;
    lcd_wifi_info_t wifiStatus;
    uint8_t numberOfCardOnScreen;
    uint8_t batteryCapacity;
    uint8_t screenBrightnessPercentage;
    uint8_t aliveWireCount;
} center_infor_typedef;

/************************************************/
extern QueueHandle_t x_Center_Infor_Queue;
extern QueueHandle_t x_Center_Param_Queue;
extern QueueHandle_t x_WiFi_2_LCD_Task_Queue;      //Gửi dữ liệu lên màn hình LCD
extern QueueHandle_t x_Socket_2_LCD_Task_Queue;     //Gửi dữ liệu socket đến Center sau đó đến LCD
extern QueueHandle_t x_LCD_2_Center_Task_Queue;      //Gửi dữ liệu màn hình tới Center
extern QueueHandle_t x_Server_2_UHF_RFID_Task;                         ////Queue gửi thông tin thẻ trở lại UHF_RFID_task giúp tránh đọc lặp lại
extern QueueHandle_t x_LCD_2_UHFRFID_Task_Queue;      //Gửi dữ liệu màn hình tới UHF RFID Task
extern QueueHandle_t x_UHFRFID_2_Center_Queue;           //Gửi dữ liệu UHF-RFID đến Center task
extern QueueHandle_t x_UHFRFIDModeCL_2_Center_Queue;           //Gửi dữ liệu UHF-RFID đến Center task
extern QueueHandle_t xQueueDeleteWireRegisterLocation;  //Gửi dữ liệu thẻ dây đăng ký location về UHF-RFID task
extern QueueHandle_t xQueueSendAliveWireCount;          //Gửi dữ liệu số lượng thẻ đang alive
extern QueueHandle_t x_NVS_To_Center_Queue;             //Đọc dữ liệu từ NVS strorage khi khởi động
#endif //_ESP32_INFOR_H