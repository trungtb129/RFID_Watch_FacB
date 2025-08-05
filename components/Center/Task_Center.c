#include "Task_Center.h"
#include "Button.h"
#include "esp_adc_cal.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"


#define NEW_ADC_DRIVER 1

#define USE_GPIO_BUTTON 1
// #define USE_ANALOG_BUTTON 1

static const char *TAG = "Center";

//Khai báo các queue hoạt động
QueueHandle_t x_Center_Infor_Queue = NULL;
QueueHandle_t x_Center_Param_Queue = NULL;
QueueHandle_t x_Battery_2_Center_Queue = NULL;
QueueHandle_t x_Button_2_Center_Queue = NULL;
QueueHandle_t x_UHFRFID_2_Center_Queue = NULL;
QueueHandle_t xQueueSendAliveWireCount = NULL; 

//Struct hoạt động của task
center_infor_typedef xCenterInfor;

//Struct parameter của hàm
center_param_typedef xCenterParameter;
//Nguyên mẫu hàm
static void Center_task(void *arg);
static void Battery_task(void *arg);
static void IMU_task(void *arg);
static void Button_task(void *arg);
static void getWifiData(void);
static void getSocketData(void);
static void getScreenData(void);
static void get_AnalogInputData(void);
static operationMode_t get_UHFRFIDData(void);
static void publicInfor(void);
static void publicParam(void);
static void getModeCL(void);
esp_adc_cal_characteristics_t *adc_chars;

#if OLD_ADC_DRIVER
// Cấu hình Analog Input đọc %pin còn lại
void init_InputAnalog(void) {
    ESP_LOGI(TAG, "Configuring ADC1 on GPIO 3...");

    // Cấu hình độ phân giải và attenuation
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

    // Cấu hình hiệu chuẩn
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, 1100, adc_chars);
    ESP_LOGI(TAG, "ADC initialized and calibrated.");
}
#endif //OLD_ADC_DRIVER

#if NEW_ADC_DRIVER
adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,  // Đảm bảo không dùng xung nhịp chính để giảm xung đột
    };
adc_cali_handle_t adc1_cali_chan0_handle = NULL;
bool do_calibration1_chan0;

static int adc_raw[2][10];
static int voltage[2][10];

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static void init_InputAnalog(void)
{
#ifndef USE_ANALOG_BUTTON
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
#endif
    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CENTER_ADC_CHANNEL, &config));

    //-------------ADC1 Calibration Init---------------//
    
    // adc_cali_handle_t adc1_cali_chan1_handle = NULL;
    do_calibration1_chan0 = example_adc_calibration_init(ADC_UNIT_1, CENTER_ADC_CHANNEL, ADC_ATTEN_DB_12, &adc1_cali_chan0_handle);
}

static int center_adc_get_raw (void) {
    if (do_calibration1_chan0) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw[0][0], &voltage[0][0]));
        // ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, CENTER_ADC_CHANNEL, voltage[0][0]);
    }
    else {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CENTER_ADC_CHANNEL, &adc_raw[0][0]));

        // ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, CENTER_ADC_CHANNEL, adc_raw[0][0]);
    }

    // ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw[0][0], &voltage[0][0]));
    // ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, CENTER_ADC_CHANNEL, voltage[0][0]);
    return adc_raw[0][0];
}
#endif //NEW_ADC_DRIVER


//Hàm trong Battery task
uint8_t Battery_get_AnalogInputData(void) {
    uint32_t total = 0;
    int adc_reading = 0;
#if OLD_ADC_DRIVER
    uint8_t sampleCount = 30;
#endif

#if NEW_ADC_DRIVER
    uint8_t sampleCount = 5;
#endif   

    // Lọc trung bình  
    for (int i = 0; i < sampleCount; i++) {
#if OLD_ADC_DRIVER
        adc_reading = adc1_get_raw(ADC_CHANNEL);
        total += adc_reading;
        vTaskDelay(pdMS_TO_TICKS(500));  // Chờ giữa các lần đọc (10 ms)
#endif

#if NEW_ADC_DRIVER
        adc_reading = center_adc_get_raw();
        total += adc_reading;
        vTaskDelay(pdMS_TO_TICKS(1000));  // Chờ giữa các lần đọc (10 ms)
#endif

    }
    

    // Tính giá trị trung bình
    adc_reading = total / sampleCount;

    // // Chuyển đổi giá trị thô sang điện áp
    // uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

    // Ngưỡng ADC thô tương ứng với 1,5V và 2,5V
    const uint32_t adc_min = 1990;  // Giá trị ADC tương ứng 1,6V
    const uint32_t adc_max = 2450; // Giá trị ADC tương ứng 2,5V

    // Tính phần trăm dựa trên điện áp đầu vào tối đa 2650 - ứng với 2,5V là giá trị tối đa sau cầu phân áp OLD: 4096
    // uint8_t percentage = (uint8_t)(((float)adc_reading / 2650) * 100.0);
    uint8_t percentage = (uint8_t)(((float)(adc_reading - adc_min) / (adc_max - adc_min)) * 100.0);

    // if(percentage < 0) percentage = 0; Bỏ bình luận nếu kiểu dữ liệu có thể âm
    if(percentage > 100) percentage = 100;

    // In kết quả

    // printf("Raw: %d, Percentage: %d\n", adc_reading, percentage);

    return percentage;
}

//Khởi tạo giao tiếp I2C cho IMU
static esp_err_t i2c_master_init(void) {
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &i2c_config));
    return i2c_driver_install(I2C_MASTER_NUM, i2c_config.mode, 0, 0, 0);
}

static esp_err_t qmi8658c_write_byte(uint8_t reg_addr, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (QMI8658C_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t qmi8658c_read_bytes(uint8_t reg_addr, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (QMI8658C_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (QMI8658C_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static void qmi8658c_init(void) {
    // Reset cảm biến
    ESP_ERROR_CHECK(qmi8658c_write_byte(0x02, 0x80)); // Register 0x02 - Soft Reset
    vTaskDelay(pdMS_TO_TICKS(10));

    // Cấu hình cảm biến (tùy thuộc vào yêu cầu)
    ESP_ERROR_CHECK(qmi8658c_write_byte(0x02, 0x00)); // Wake up sensor
    ESP_ERROR_CHECK(qmi8658c_write_byte(0x03, 0x4C)); // Config accelerometer
    ESP_ERROR_CHECK(qmi8658c_write_byte(0x04, 0x20)); // Config gyroscope
}

static void qmi8658c_read_data(void) {
    uint8_t data[12]; // Gia tốc (6 bytes) + Con quay (6 bytes)
    ESP_ERROR_CHECK(qmi8658c_read_bytes(0x35, data, 12));

    int16_t accel_x = (data[1] << 8) | data[0];
    int16_t accel_y = (data[3] << 8) | data[2];
    int16_t accel_z = (data[5] << 8) | data[4];

    int16_t gyro_x = (data[7] << 8) | data[6];
    int16_t gyro_y = (data[9] << 8) | data[8];
    int16_t gyro_z = (data[11] << 8) | data[10];

    ESP_LOGI(TAG, "Accel: X=%d, Y=%d, Z=%d", accel_x, accel_y, accel_z);
    ESP_LOGI(TAG, "Gyro: X=%d, Y=%d, Z=%d", gyro_x, gyro_y, gyro_z);
}

void i2c_scanner() {
    printf("Scanning...\n");
    for (int addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        if (ret == ESP_OK) {
            printf("I2C device found at address 0x%02x\n", addr);
        }
    }
    printf("Scan complete.\n");
}

#ifdef USE_ANALOG_BUTTON
// adc_oneshot_unit_handle_t adc1_handle_ch8;
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,  // Đảm bảo không dùng xung nhịp chính để giảm xung đột
    };
adc_cali_handle_t adc1_cali_chan8_handle = NULL;
bool do_calibration1_chan8;

static int adc_raw_button[2][10];
static int voltage_button[2][10];

static int center_adc_get_raw_button(void) {
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_8, &adc_raw_button[0][0]));
    if (do_calibration1_chan8) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan8_handle, adc_raw_button[0][0], &voltage_button[0][0]));
        // ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, CENTER_ADC_CHANNEL, voltage_button[0][0]);
    }
    else {
        // ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, CENTER_ADC_CHANNEL, adc_raw_button[0][0]);
    }

    // ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw_button[0][0], &voltage_button[0][0]));
    // ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC_CHANNEL_8, adc_raw_button[0][0]);
    return adc_raw_button[0][0];
}
#endif

//Cấu hình GPIO cho nút nhấn, hàm này thay đổi tùy thuộc phần cứng.
void init_InputGPIOs(void) {
#ifdef USE_GPIO_BUTTON
  // Cấu hình GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO), // Chỉ định chân GPIO 21
        .mode = GPIO_MODE_INPUT,             // Cấu hình làm đầu vào
        .pull_up_en = GPIO_PULLUP_ENABLE,    // Kích hoạt pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // Không kích hoạt pull-down
        .intr_type = GPIO_INTR_DISABLE       // Không sử dụng ngắt
    };
    gpio_config(&io_conf);
#endif
#ifdef USE_ANALOG_BUTTON
    
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config2, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_8, &config));

    //-------------ADC1 Calibration Init---------------//
    
    // adc_cali_handle_t adc1_cali_chan1_handle = NULL;
    do_calibration1_chan8 = example_adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_8, ADC_ATTEN_DB_12, &adc1_cali_chan8_handle);

#endif
}

// Hàm trả về giá trị thời gian hoạt động, thay đổi tùy thuộc phần cứng.
uint32_t get_Button_Millis(void) {
  return pdTICKS_TO_MS(xTaskGetTickCount());
  
}

uint8_t get_Button1_Data(){
#ifdef USE_GPIO_BUTTON
  // Hàm trả về trạng thái GPIO nối với nút nhấn, lưu ý cấu hình sao cho khi nhấn trả về 1, khi nhả nút trả về 0.
  return !gpio_get_level(BUTTON_GPIO);
#endif
#ifdef USE_ANALOG_BUTTON
    int button_adc_reading = 0;
    button_adc_reading = center_adc_get_raw_button();
    if(button_adc_reading > 1900 && button_adc_reading < 2180) {
        return 0;
    }
    else {
        return 1;
    }
#endif
}

operationMode_t oldWorkMode = IN_MODE;
operationMode_t buttonWorkMode = IN_MODE;
//Cờ isJustHoldButton giúp việc giữ nút chỉ có tác dụng chuyển từ Sleep mode về work mode, không làm thay đổi work mode (work mode gồm IN_MODE và OUT_MODE)
uint8_t isJustHoldButton = 0;
void pressShortButton(void){
	// printf("Ban vua nhan nha nhanh\n");
}

void pressButton(void){
	// printf("Ban vua nhan nut\n");
}

void releaseButton(void){
	// printf("Ban vua nha nut\n");
    if(isJustHoldButton == 1) {
        isJustHoldButton = 0;
        return;
    }
    if(buttonWorkMode != SLEEP_MODE) {
        if(buttonWorkMode == IN_MODE) {
            buttonWorkMode = OUT_MODE;
            oldWorkMode = OUT_MODE;
            ESP_LOGW(TAG, "Doi che do lam viec thanh Out Mode");
        }
        else if(buttonWorkMode == OUT_MODE) {
            buttonWorkMode = IN_MODE;
            oldWorkMode = IN_MODE;
            ESP_LOGW(TAG, "Doi che do lam viec thanh In Mode");
        }
    }
    isJustHoldButton = 0;
}

void holdButton(void){			
	// printf("Ban vua nhan giu nut\n");
    if(buttonWorkMode != SLEEP_MODE) {
        buttonWorkMode = SLEEP_MODE;
    }
    else {
        buttonWorkMode = oldWorkMode;
    }
    isJustHoldButton = 1;
}

void doubleClickButton(void){
	// printf("Ban vua nhap dup nut bam\n");
}

Button_Typdedef button1;

void Center_Config(void) {
    x_Center_Infor_Queue = xQueueCreate( 1, sizeof(center_infor_typedef));
    x_Center_Param_Queue = xQueueCreate( 1, sizeof(center_param_typedef));
    x_Battery_2_Center_Queue = xQueueCreate( 1, sizeof(uint8_t));
    x_Button_2_Center_Queue = xQueueCreate( 1, sizeof(operationMode_t));
    x_UHFRFID_2_Center_Queue = xQueueCreate(10, sizeof(uint8_t));
    x_UHFRFIDModeCL_2_Center_Queue = xQueueCreate(10, sizeof(uint8_t));
    xQueueSendAliveWireCount = xQueueCreate(1, sizeof(uint8_t));


    //Khởi tạo giá trị hoạt động cho nút nhấn
    // init_InputGPIOs();
    // init_ButtonPara(&button1, 20, 5000, 600);
    // init_ButtonCallback(&button1, get_Button1_Data, pressShortButton, pressButton, releaseButton, holdButton, doubleClickButton);

    // ESP_ERROR_CHECK(i2c_master_init());
    // // i2c_scanner();
    // qmi8658c_init();

    //Khởi tại Analog đọc %pin
    init_InputAnalog();

    xTaskCreate(Battery_task, "Battery_task", 1024*4, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(Button_task, "Button_task", 1024*4, NULL, configMAX_PRIORITIES - 3, NULL);
    // xTaskCreate(&IMU_task, "IMU_task", 1024*4, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(Center_task, "Center_task", 1024*5, NULL, configMAX_PRIORITIES - 2, NULL);
}

static void IMU_task(void *arg)  {
    while(1){
        qmi8658c_read_data();
        // if(x_Battery_2_Center_Queue != NULL) {
        //     xQueueOverwrite(x_Battery_2_Center_Queue, &batteryValue);
        // }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void Button_task(void *arg)  {
    while(1){

        // buttonHandler(&button1);

        buttonWorkMode = get_UHFRFIDData();

        if(x_Button_2_Center_Queue != NULL && buttonWorkMode != MODEn) {
            xQueueOverwrite(x_Button_2_Center_Queue, &buttonWorkMode);
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}



uint8_t old_batteryValue = 0;
static void Battery_task(void *arg)  {
    old_batteryValue = Battery_get_AnalogInputData();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    uint8_t batteryValue = 0;
    while(1){
        uint8_t new_batteryValue = Battery_get_AnalogInputData();

        //Nếu giá trị mới và cũ lệch quá lớn thì ngay lật tức cập nhật giá trị mới
        if((new_batteryValue > old_batteryValue && new_batteryValue - old_batteryValue >= 20) || 
        (old_batteryValue > new_batteryValue && old_batteryValue - new_batteryValue >= 20)) {
            batteryValue = new_batteryValue;
        }
        else {
            batteryValue = old_batteryValue * 0.5 + new_batteryValue * 0.5;
        }
        
        old_batteryValue = new_batteryValue;

        if(x_Battery_2_Center_Queue != NULL) {
            xQueueOverwrite(x_Battery_2_Center_Queue, &batteryValue);
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

static void Center_task(void *arg)  {

    while(x_NVS_To_Center_Queue == NULL) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    while(xQueuePeek(x_NVS_To_Center_Queue, &xCenterParameter, 10 / portTICK_PERIOD_MS) != pdPASS) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }


    //Gửi dữ liệu vào queue trước khi bắt đầu tránh lỗi
    uint8_t tempValue = 100;
    xQueueOverwrite(x_Battery_2_Center_Queue, &tempValue);

    while(1) {
        getWifiData();
        getSocketData();
        getScreenData();
        getModeCL();
        if(xQueueSendAliveWireCount != NULL) {
            uint8_t checkAliveCardCount = 0;
            if(xQueuePeek(xQueueSendAliveWireCount, &checkAliveCardCount, 10 / portTICK_PERIOD_MS) == pdPASS) {
                xCenterInfor.aliveWireCount = checkAliveCardCount;
            }
        }

        //Nhận dữ liệu chế độ từ nút nhấn
        if(x_Button_2_Center_Queue != NULL) {
            operationMode_t centerWorkMode;
            if(xQueuePeek(x_Button_2_Center_Queue, &centerWorkMode, 10 / portTICK_PERIOD_MS) == pdPASS) {
                xCenterInfor.operationMode = centerWorkMode;
            }
        }

        get_AnalogInputData();
        // Nếu % pin lớn hơn 30 thì giữ độ sáng màn hình
        if(xCenterInfor.batteryCapacity > 30) {
            xCenterInfor.screenBrightnessPercentage = 60;
        }
        // Nếu % pin thấp hơn 30 thì giảm độ sáng màn hình
        else {
            xCenterInfor.screenBrightnessPercentage = 10;
        }

        publicInfor();
        publicParam();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

}

static void getWifiData(void) { 
    if(x_WiFi_2_LCD_Task_Queue != NULL) {
        xQueuePeek(x_WiFi_2_LCD_Task_Queue, &xCenterInfor.wifiStatus, pdMS_TO_TICKS(7));
    }
}

static void getSocketData(void) { 
    if(x_Socket_2_LCD_Task_Queue != NULL) {
        xQueuePeek(x_Socket_2_LCD_Task_Queue, &xCenterInfor.socketStatus, pdMS_TO_TICKS(7));
    }
}

LCD_Infor_TypeDef xLCDInfor;
static void getScreenData(void) {
    if(x_LCD_2_Center_Task_Queue != NULL) {
        xQueuePeek(x_LCD_2_Center_Task_Queue, &xLCDInfor, pdMS_TO_TICKS(7));
    }
    xCenterInfor.numberOfCardOnScreen = xLCDInfor.numberOfCardOnScreen;
}
center_param_typedef xUHF_RFID_MODE_CL;
static void getModeCL(void) {
    if(x_UHFRFIDModeCL_2_Center_Queue != NULL)
    {
        if(xQueueReceive(x_UHFRFIDModeCL_2_Center_Queue, &xUHF_RFID_MODE_CL, pdMS_TO_TICKS(10)) == pdPASS)
            xCenterParameter.mode = xUHF_RFID_MODE_CL.mode;
    }
}
static void publicInfor(void) {
    if(x_Center_Infor_Queue != NULL) {
        xQueueOverwrite(x_Center_Infor_Queue, &xCenterInfor);
    }
}

static void publicParam(void) {
    if(x_Center_Param_Queue != NULL) {
        xQueueOverwrite(x_Center_Param_Queue, &xCenterParameter);
    }
}

//Hàm trong Center task
static void get_AnalogInputData(void) {
    if(x_Battery_2_Center_Queue != NULL) {
        xQueuePeek(x_Battery_2_Center_Queue, &xCenterInfor.batteryCapacity, pdMS_TO_TICKS(7));
    }
}

//Hàm lấy dữ liệu từ UHF-RFID Task
static operationMode_t get_UHFRFIDData(void) {
    if(x_UHFRFID_2_Center_Queue != NULL) {
        // Bằng 0: Không thực hiện || Bằng 1: IN-Mode || Bằng 2: OUT-Mode || Bằng 3: SLEEP-Mode
        uint8_t changeModeBy_UHFRFID_Tag = 0;
        if(xQueueReceive(x_UHFRFID_2_Center_Queue, &changeModeBy_UHFRFID_Tag, pdMS_TO_TICKS(10)) == pdPASS) {
            if(changeModeBy_UHFRFID_Tag == 1) {
                return IN_MODE;
            }
            else if(changeModeBy_UHFRFID_Tag == 2) {
                return OUT_MODE;
            }
            else if(changeModeBy_UHFRFID_Tag == 3) {
                return SLEEP_MODE;
            }
        }
    }
    //Nếu lỗi trả về Mode không hợp lệ
    return MODEn;
}


