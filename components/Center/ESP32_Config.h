#ifndef _ESP32_CONFIG_H
#define _ESP32_CONFIG_H

#define USING_PARAMETER_FROM_NVS 0      //Có sử dụng thông số lấy từ bộ nhớ NVS không - Chưa code

#define TEST_UHF_RFID_WATCH

/*Định nghĩa các struct quản lý kết nối wifi & socket server */
/*
Sumi wifi
*/
#if 0 //RFID FAC_A
#define ESP_WIFI_SSID "SHWS-RFID_Test"
#define ESP_WIFI_PASS "Shws2025"

// #define ESP_WIFI_SSID "Wifi6"
// #define ESP_WIFI_PASS "18112000"

#define HOST_IP_ADDR "10.0.12.2"
// #define HOST_IP_ADDR "10.0.12.51"

#define PORT 8888

#define UHF_ESP_USING_STATIC_IP 1
// #define UHF_ESP_STATIC_IP "10.0.12.30"
// #define UHF_ESP_STATIC_IP "10.0.12.31" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.32"//ok
// #define UHF_ESP_STATIC_IP "10.0.12.33"//ok
// #define UHF_ESP_STATIC_IP "10.0.12.34"//ok
// #define UHF_ESP_STATIC_IP "10.0.12.35"//ok
// #define UHF_ESP_STATIC_IP "10.0.12.36" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.37" //
// #define UHF_ESP_STATIC_IP "10.0.12.38" //
// #define UHF_ESP_STATIC_IP "10.0.12.39" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.40" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.41"//
// #define UHF_ESP_STATIC_IP "10.0.12.42   " //
#define UHF_ESP_STATIC_IP "10.0.12.43" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.44" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.45"//ok
// #define UHF_ESP_STATIC_IP "10.0.12.46" //
// #define UHF_ESP_STATIC_IP "10.0.12.47" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.48" // ok
// #define UHF_ESP_STATIC_IP "10.0.12.49"//ok
// #define UHF_ESP_STATIC_IP "10.0.12.50"
// #define UHF_ESP_STATIC_IP "10.0.12.51"



#define UHF_ESP_STATIC_SUBNET_MASK "255.255.255.0"
#define UHF_ESP_STATIC_GW "10.0.12.1"
#endif //RFID FAC_A

#if 1//RFID FAC_B
#define ESP_WIFI_SSID "SHWS-RFID_Test"
#define ESP_WIFI_PASS "RFID_FB_25@"

// #define ESP_WIFI_SSID "Wifi6"
// #define ESP_WIFI_PASS "18112000"

#define HOST_IP_ADDR "10.0.12.2"
// #define HOST_IP_ADDR "10.0.12.51"

#define PORT 10001

#define UHF_ESP_USING_STATIC_IP 1
// #define UHF_ESP_STATIC_IP "10.0.12.30"
// #define UHF_ESP_STATIC_IP "10.0.12.31" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.32"//ok
// #define UHF_ESP_STATIC_IP "10.0.12.33"//ok
// #define UHF_ESP_STATIC_IP "10.0.12.34"//ok
// #define UHF_ESP_STATIC_IP "10.0.12.35"//ok
// #define UHF_ESP_STATIC_IP "10.0.12.36" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.37" //
// #define UHF_ESP_STATIC_IP "10.0.12.38" //
// #define UHF_ESP_STATIC_IP "10.0.12.39" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.40" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.41"//
// #define UHF_ESP_STATIC_IP "10.0.12.42   " //
#define UHF_ESP_STATIC_IP "10.0.12.43" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.44" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.45"//ok
// #define UHF_ESP_STATIC_IP "10.0.12.46" //
// #define UHF_ESP_STATIC_IP "10.0.12.47" //ok
// #define UHF_ESP_STATIC_IP "10.0.12.48" // ok
// #define UHF_ESP_STATIC_IP "10.0.12.49"//ok
// #define UHF_ESP_STATIC_IP "10.0.12.50"
// #define UHF_ESP_STATIC_IP "10.0.12.51"



#define UHF_ESP_STATIC_SUBNET_MASK "255.255.255.0"
#define UHF_ESP_STATIC_GW "10.0.12.1"
#endif//RFID FAC_B

#if 0
#define ESP_WIFI_SSID "CTY ROBOTICS AUBOT"
#define ESP_WIFI_PASS "123456789"

// #define HOST_IP_ADDR "10.1.131.35"
// #define HOST_IP_ADDR "10.1.3.1"
#define HOST_IP_ADDR "10.1.131.35"
#define PORT 8889

#define UHF_ESP_USING_STATIC_IP 0
#define UHF_ESP_STATIC_IP "10.1.12.31"
#define UHF_ESP_STATIC_SUBNET_MASK "255.255.0.0"
#define UHF_ESP_STATIC_GW "10.1.0.1"

#endif

 #if 00 //Wifi6
#define PORT 8888
#define HOST_IP_ADDR "192.168.2.2"

#define ESP_WIFI_SSID "Wifi6"
#define ESP_WIFI_PASS "18112000"

#define UHF_ESP_STATIC_IP "192.168.2.11"
#define UHF_ESP_STATIC_SUBNET_MASK "255.255.255.0"
#define UHF_ESP_STATIC_GW "192.168.2.1"
#endif

//Aubot Wifi


//Wifi
#if 0
#define ESP_WIFI_SSID "Nha 03 Tuu Liet"
#define ESP_WIFI_PASS "12344321"

// #define HOST_IP_ADDR "10.1.1.3"
#define HOST_IP_ADDR "192.168.1.155"
#define PORT 8889

#define UHF_ESP_USING_STATIC_IP 1
#define UHF_ESP_STATIC_IP "192.168.1.156"
#define UHF_ESP_STATIC_SUBNET_MASK "255.255.255.0"
#define UHF_ESP_STATIC_GW "192.168.1.1"
#endif

#define BUTTON_GPIO 44

//Giao tiếp đọc dung lượng pin
#define ADC_CHANNEL ADC1_CHANNEL_2 // GPIO 3 thuộc ADC1_CHANNEL_2
#define ADC_ATTEN   ADC_ATTEN_DB_12 // Độ suy giảm tín hiệu
#define ADC_WIDTH   ADC_WIDTH_BIT_12 // Độ phân giải 12-bit
#define MAX_VOLTAGE 3300             // Điện áp đầu vào tối đa tính bằng mV (3.3V)

#define CENTER_ADC_CHANNEL ADC_CHANNEL_2

//Giao tiếp IMU
#define I2C_MASTER_SCL_IO 41
#define I2C_MASTER_SDA_IO 42
#define I2C_MASTER_FREQ_HZ 100000  // Tần số I2C
#define I2C_MASTER_NUM I2C_NUM_0   // Kênh I2C

// Định nghĩa địa chỉ I2C của QMI8658C
#define QMI8658C_I2C_ADDR 0x6B

// Chiều dài tối đa của mã EPC là MAX_EPC_LENGTH - 1 do còn 1 byte kết thúc chuỗi
#define MAX_EPC_LENGTH 20

// Công suất cài đặt cho đầu đọc, với M5Stack sẽ có giá trị từ 15 - 26
#define SET_UHF_RFID_TX_POWER 24

// Ngưỡng lọc RSSI gồm 2 ngưỡng riêng biệt
// #define SET_READ_THRESHOLD_1  -62
// #define SET_READ_THRESHOLD_2  -58
#define SET_READ_THRESHOLD_1  -60 // Tốt hơn ngưỡng này và có số lần đọc nhất định thì cho là OK
#define SET_READ_THRESHOLD_2  -58 // Tốt hơn ngưỡng này thì cho hiển thị
// Chiều dài frame dữ liệu nhận về từ server cho mỗi bản tin
#define NORMAL_SIZE_OF_RESPONSE_DATA_FROM_SERVER 89

// Ngưỡng RSSI lọc cho chế độ Out
#define SET_OUTMODE_READ_THRESHOLD -85

// Chu kì gửi dữ liệu tới server (ms)
#define PERIOD_SEND_TO_SOCKET_TASK 0

// Kích thước mảng chứa thông tin thẻ nhận về từ server
// #define SIZEOF_ARR_CARD_INFOR_FROM_SERVER 12
#define SIZEOF_ARR_CARD_INFOR_FROM_SERVER 15

// Kích thước mảng chứa các thẻ đang ở trạng thái ALive
// #define SIZEOF_CHECK_ALIVE_CARD_ARR 10
#define SIZEOF_CHECK_ALIVE_CARD_ARR 12


// Thời gian timeout - tức nếu vượt quá thời gian này (ms) mà không đọc lại sẽ xoá trạng thái ALive (Chỉ ở chế độ Out)
#define CHECK_ALIVE_CARD_TIMEOUT 1000

// Thời gian đọc lặp lại cho thẻ, nếu quá thời gian này (ms) thì số lần đếm đọc lại được reset về 0
#define SET_READ_TIMEOUT 5000

// Ngưỡng đọc lặp lại đạt yêu cầu qua bộ lọc
// #define SET_REPEAT_READ_CARD_THRESHOLD 6
#define SET_REPEAT_READ_CARD_THRESHOLD 3 // Số lần đọc lớn hơn ngưỡng 1 nhỏ hơn ngưỡng 2


// Kích thước mảng phục vụ cho bộ lọc Input (Bộ lọc sử dụng 2 ngưỡng RSSI và số lần đọc lặp lại)
#define SIZEOF_CHECK_CARD_INPUT_ARR 30

// // Độ sáng màn hình giá trị từ 0 - 100
// #define SCREEN_BRIGHTNESS_PERCENTAGE 60

#define MAX_WIRE_COLOR_ON_ONELINE 3

#define CONFIG_PIN_NUM_BK_LIGHT 45

#define RSSI_THRESHOLD_IN_CHECKPOINT -82    
#define RSSI_THRESHOLD_OUT_CHECKPOINT -82

#define CHECK_THE_BEST_RSSI_DELAY_TIME 100

#define DELAY_WHEN_IDLE_TIME    50
#define DELAY_WHEN_WORKING      50
#define TIMEOUT_WORKING_TIME    3000

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (CONFIG_PIN_NUM_BK_LIGHT) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

/************************************************/

#endif  //_ESP32_CONFIG_H