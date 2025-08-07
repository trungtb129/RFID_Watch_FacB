/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

 #include <stdio.h>
 #include <stdint.h>
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "freertos/semphr.h"
 #include "freertos/queue.h"
 #include "esp_timer.h"
 #include "esp_lcd_panel_io.h"
 #include "esp_lcd_panel_vendor.h"
 #include "esp_lcd_panel_ops.h"
 #include "driver/gpio.h"
 #include "driver/spi_master.h"
 #include "driver/i2c_master.h"
 #include "driver/ledc.h"
 #include "esp_err.h"
 #include "esp_log.h"
 #include "lvgl.h"
 
 #include "ui.h"
 
 #include "ESP32_Infor.h"
 #include "UNIT_UHF_RFID.h"
 #include "TCP_CLIENT_V4.h"
 #include "CONNECT_WIFI_STA.h"
 #include "Task_Center.h"
 #include "Task_NVS.h"
 
 
 #if CONFIG_EXAMPLE_LCD_CONTROLLER_ILI9341
 #include "esp_lcd_ili9341.h"
 #elif CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01
 #include "esp_lcd_gc9a01.h"
 #endif
 
 #if CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_STMPE610
 #include "esp_lcd_touch_stmpe610.h"
 #endif
 
 static const char *TAG = "example";
 
 // Using SPI2 in the example
 #define LCD_HOST  SPI2_HOST
 
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 //////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 #define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)
 #define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  1
 #define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
 #define EXAMPLE_PIN_NUM_SCLK           36
 #define EXAMPLE_PIN_NUM_MOSI           35
 #define EXAMPLE_PIN_NUM_MISO           -1
 #define EXAMPLE_PIN_NUM_LCD_DC         37  // TFT_DC nối với MTCK (GPIO39)
 #define EXAMPLE_PIN_NUM_LCD_RST        40  // TFT_RST nối với MTDO (GPIO40)
 #define EXAMPLE_PIN_NUM_LCD_CS         7
 #define EXAMPLE_PIN_NUM_BK_LIGHT       CONFIG_PIN_NUM_BK_LIGHT
 #define EXAMPLE_PIN_NUM_TOUCH_CS       -1
 
 #define I2C_MASTER_NUM             I2C_NUM_0
 #define I2C_MASTER_FREQ_HZ         100000
 #define I2C_MASTER_TX_BUF_DISABLE  0
 #define I2C_MASTER_RX_BUF_DISABLE  0
 #define CST816D_I2C_ADDR           0x15  // Địa chỉ I²C của CST816D
 
 #define EXAMPLE_PIN_NUM_TOUCH_SCL      5
 #define EXAMPLE_PIN_NUM_TOUCH_SDA      4
 #define EXAMPLE_PIN_NUM_TOUCH_RST      1
 #define EXAMPLE_PIN_NUM_TOUCH_INT      0
 
 // The pixel number in horizontal and vertical
 #if CONFIG_EXAMPLE_LCD_CONTROLLER_ILI9341
 #define EXAMPLE_LCD_H_RES              240
 #define EXAMPLE_LCD_V_RES              280
 #elif CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01
 #define EXAMPLE_LCD_H_RES              240
 #define EXAMPLE_LCD_V_RES              240
 #endif
 // Bit number used to represent command and parameter
 #define EXAMPLE_LCD_CMD_BITS           8
 #define EXAMPLE_LCD_PARAM_BITS         8
 
 #define EXAMPLE_LVGL_TICK_PERIOD_MS    2
 #define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 70
 #define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 30
 #define EXAMPLE_LVGL_TASK_STACK_SIZE   (10 * 1024)
 #define EXAMPLE_LVGL_TASK_PRIORITY     (configMAX_PRIORITIES - 1)
 
 
 CARD lcd_received_card;
 wire_infor DayDien;
 manage_Card_Alive_t LCD_aliveCardArr[SIZEOF_CHECK_ALIVE_CARD_ARR];            //Mảng lưu trữ các thẻ đang đọc được
 lcd_wifi_info_t  wifi_info = {};
 static uint8_t screen_BrightnessPercentage = 60;                                //Biến lưu trữ % độ sáng màn hình
 
 //Quản lý thông tin hoạt động của màn hình
 LCD_Infor_TypeDef LCDInfor;
 center_param_typedef xLCD_CenterParam;
 sockettcp_info lcd_socketStatus;
 QueueHandle_t x_LCD_2_Center_Task_Queue;      //Gửi dữ liệu màn hình tới Center
 QueueHandle_t x_LCD_2_UHFRFID_Task_Queue;      //Gửi dữ liệu màn hình tới UHF RFID Task
 
 center_infor_typedef lcd_percentBat; //Hiển thị phần trăm pin
 center_infor_typedef lcd_wireCount;  //Hiển thị số thẻ đang được đọc trên màn hình
 extern QueueHandle_t x_Card_2_LCD_Task_Queue;      //Gửi dữ liệu serial lên màn hình LCD
 static SemaphoreHandle_t lvgl_mux = NULL;
 
 #if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
 esp_lcd_touch_handle_t tp = NULL;
 #endif
 
 extern void example_lvgl_demo_ui(lv_disp_t *disp);
 
 static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
 {
     lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
     lv_disp_flush_ready(disp_driver);
     return false;
 }
 
 static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
 {
     esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
     int offsetx1 = area->x1;
     int offsetx2 = area->x2;
     int offsety1 = area->y1;
     int offsety2 = area->y2;
     // copy a buffer's content to a specific area of the display
     esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
 }
 
 /* Rotate display and touch, when rotated screen in LVGL. Called when driver parameters are updated. */
 static void example_lvgl_port_update_callback(lv_disp_drv_t *drv)
 {
     esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
 
     switch (drv->rotated) {
     case LV_DISP_ROT_NONE:
         // Rotate LCD display
         esp_lcd_panel_swap_xy(panel_handle, false);
         esp_lcd_panel_mirror(panel_handle, true, false);
 #if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
         // Rotate LCD touch
         esp_lcd_touch_set_mirror_y(tp, false);
         esp_lcd_touch_set_mirror_x(tp, false);
 #endif
         break;
     case LV_DISP_ROT_90:
         // Rotate LCD display
         esp_lcd_panel_swap_xy(panel_handle, true);
         esp_lcd_panel_mirror(panel_handle, true, true);
 #if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
         // Rotate LCD touch
         esp_lcd_touch_set_mirror_y(tp, false);
         esp_lcd_touch_set_mirror_x(tp, false);
 #endif
         break;
     case LV_DISP_ROT_180:
         // Rotate LCD display
         esp_lcd_panel_swap_xy(panel_handle, false);
         esp_lcd_panel_mirror(panel_handle, false, true);
 #if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
         // Rotate LCD touch
         esp_lcd_touch_set_mirror_y(tp, false);
         esp_lcd_touch_set_mirror_x(tp, false);
 #endif
         break;
     case LV_DISP_ROT_270:
         // Rotate LCD display
         esp_lcd_panel_swap_xy(panel_handle, true);
         esp_lcd_panel_mirror(panel_handle, false, false);
 #if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
         // Rotate LCD touch
         esp_lcd_touch_set_mirror_y(tp, false);
         esp_lcd_touch_set_mirror_x(tp, false);
 #endif
         break;
     }
 }
 
 #if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
 static void example_lvgl_touch_cb(lv_indev_drv_t * drv, lv_indev_data_t * data)
 {
     uint16_t touchpad_x[1] = {0};
     uint16_t touchpad_y[1] = {0};
     uint8_t touchpad_cnt = 0;
 
     /* Read touch controller data */
     esp_lcd_touch_read_data(drv->user_data);
 
     /* Get coordinates */
     bool touchpad_pressed = esp_lcd_touch_get_coordinates(drv->user_data, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);
 
     if (touchpad_pressed && touchpad_cnt > 0) {
         data->point.x = touchpad_x[0];
         data->point.y = touchpad_y[0];
         data->state = LV_INDEV_STATE_PRESSED;
     } else {
         data->state = LV_INDEV_STATE_RELEASED;
     }
 }
 #endif
 
 static void example_increase_lvgl_tick(void *arg)
 {
     /* Tell LVGL how many milliseconds has elapsed */
     lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
 }
 
 bool example_lvgl_lock(int timeout_ms)
 {
     // Convert timeout in milliseconds to FreeRTOS ticks
     // If `timeout_ms` is set to -1, the program will block until the condition is met
     const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
     return xSemaphoreTakeRecursive(lvgl_mux, timeout_ticks) == pdTRUE;
 }
 
 void example_lvgl_unlock(void)
 {
     xSemaphoreGiveRecursive(lvgl_mux);
 }
 
 bool _check_dup_display(wire_infor wires[],  uint8_t wire_index, wire_infor* wire_data)
 {
     for (uint8_t i = 0; i < 6; i++)
     {
         if (strcmp(wires[i].serial, wire_data->serial) == 0 && wire_data->serial[0] != 0x00)
         {
             // ESP_LOGW(TAG, "Dup check - display_wires: %s", wires[i].serial);
             // ESP_LOGW(TAG, "Dup check - receive: %s", wire_data->serial);
             return true;
         }
     }
     memcpy(&wires[wire_index], wire_data, sizeof(wire_infor));
     // ESP_LOGW(TAG, "new dis_wire at %"PRIu8" with serial=%s, compare data=%s", wire_index, wires[wire_index].serial, wire_data->serial);
 
     // (*p_wire_sount)++;
     
     return false;
 }
 
 void deleteTemporaryWireColor(wire_infor* wireTarget) {
     //Tạm thời lưu số lượng màu hiện tại
     uint8_t tempCount = wireTarget->colors.color_count;
     //Kiểm tra theo số lượng màu của dây source cho destination
     for(uint8_t i = 0; i < tempCount; i++) {
         //Nếu phát hiện là màu tạm thời
         if(wireTarget->colors.color[i].colorStatus == TEMPORARY_WIRE_COLOR) {
             //Tiến hành xóa màu tạm thời
             memset(&wireTarget->colors.color[i], 0, sizeof(wire_color_t));
             //Giảm số lượng
             wireTarget->colors.color_count --;
         }
     }
 }
 
 void addColorToWireInfor(wire_infor* destination, wire_infor* source) {
     if(destination->colors.color_count >= MAX_WIRE_COLOR_ON_ONELINE) {
         //Nếu dây đích đã có số lượng màu lớn hơn tối đa -> Bỏ qua không cộng
         return;
     }
 
     //Kiểm tra và gán đủ số lượng màu của dây source cho destination
     for(uint8_t i = 0; i < source->colors.color_count; i++) {
         //Kiểm tra màu của dây source có trùng với màu của dây đích hay không
         for(uint8_t j = 0; j < destination->colors.color_count; j++) {
             if(strcmp((char*)&destination->colors.color[j].main_color, (char*)&source->colors.color[i].main_color) == 0
             && strcmp((char*)&destination->colors.color[j].sub_color, (char*)&source->colors.color[i].sub_color) == 0
             && destination->colors.color[j].main_color[0] != 0x00 && destination->colors.color[j].sub_color[0] != 0x00) {
                 //Nếu trùng rồi thì thôi không hiển thị thêm nữa
                 continue;
             }
         }
         //Còn nếu không trùng thì ra sẽ ghi màu dây vào dây đích (đánh dấu là tạm thời)
         memcpy(&destination->colors.color[destination->colors.color_count], &source->colors.color[i], sizeof(wire_color_t));
         destination->colors.color[destination->colors.color_count].colorStatus = TEMPORARY_WIRE_COLOR;
         //Tăng biến đếm số lượng thông tin màu dây
         destination->colors.color_count ++;
         
         if(destination->colors.color_count >= MAX_WIRE_COLOR_ON_ONELINE) {
             //Nếu dây đích đã có số lượng màu lớn hơn tối đa -> Bỏ qua không cộng
             return;
         }
     }
 
 }
 
 wire_infor DataWireReceive;
 wire_infor DataWireReceiveArr[SIZEOF_ARR_CARD_INFOR_FROM_SERVER];     //Mảng chứa thông tin các dây nhận được từ server 
 uint8_t DataWireReceiveArr_Index = 0;
 wire_infor display_wires[6] = {};
 uint8_t wireCount = 0;
 
 Manage_CardInfo_LCD_t LCD_cardInfo[6];
 uint8_t isNeedToggleScreen = 0;
 uint32_t checkTimeToggleScreen = 0;
 uint8_t BlinkOK = 0;
 uint32_t BlinkOKTime = 0;
 
 uint8_t isNeedHighLightWires = 0;       //Cờ trạng thái có cần mở chức năng HighLight không
 
 center_infor_typedef xLCD_CenterInfor;
 
 static void example_lvgl_port_task(void *arg)
 {
     ESP_LOGI(TAG, "Starting LVGL task");
     uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
     memset(display_wires, '\0', sizeof(display_wires));
 
     uint8_t isGetCenterInforOK = 0;

    // TickType_t last_tick = 0;
    // TickType_t current_tick = 0;
    // TickType_t period_tick = 0;

    // TickType_t start_tick, end_tick, duration;
 
     while (1) 
     {
        //Nhận thông tin từ Center
            if(x_Center_Param_Queue != NULL) {
                //// Peek thành công: xUHF_RFID_CenterInfor chứa thông tin mới từ Center
                if(xQueueReceive(x_Center_Param_Queue, &xLCD_CenterParam, 10 / portTICK_PERIOD_MS) == pdTRUE) { 
                }
            }
            // ESP_LOGI(TAG, "Mode: %d", xLCD_CenterParam.mode);
            Check_ModeCL_Screen(xLCD_CenterParam.mode);
        /**************************************************************************************** */
#if 1 //FacB mode
        if(xLCD_CenterParam.mode == SA_MODE || xLCD_CenterParam.mode == GROUP_MODE || xLCD_CenterParam.mode == SA_GROUP_MODE) 
        {
            #if 1 ///CL03 Mode
            
            // current_tick = xTaskGetTickCount();

            // if (last_tick != 0) {
            //     period_tick = current_tick - last_tick;
            //     printf("Task cycle time: %lu ticks\n", (unsigned long)period_tick);
            // }

            // last_tick = current_tick;

            //Nhận thông tin từ Task Center 
            if(x_Center_Infor_Queue != NULL) {
                if(xQueuePeek(x_Center_Infor_Queue, &xLCD_CenterInfor, pdMS_TO_TICKS(10)) == pdTRUE) { 
                    isGetCenterInforOK = 1;
                }
                else {
                    isGetCenterInforOK = 0;
                }
            }
    
    
    
            //Cập nhật độ sáng màn hình nếu có sự thay đổi
            if(screen_BrightnessPercentage != xLCD_CenterInfor.screenBrightnessPercentage) {
                // gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
                ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 8192 * xLCD_CenterInfor.screenBrightnessPercentage / 100.0));
                screen_BrightnessPercentage = xLCD_CenterInfor.screenBrightnessPercentage;
                // Update duty to apply the new value
                ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
            }
    
                    
            // Lock the mutex due to the LVGL APIs are not thread-safe
            if (example_lvgl_lock(-1)) {
                task_delay_ms = lv_timer_handler();
            
                if(isGetCenterInforOK == 1) {
                    //Cập nhật thông tin kết nối wifi
                    wifi_info.rssi = xLCD_CenterInfor.wifiStatus.rssi;
                    wifi_info.isConnectedWifi = xLCD_CenterInfor.wifiStatus.isConnectedWifi;
                    lcd_set_wifi_color(wifi_info.isConnectedWifi, wifi_info.rssi);
                    lcd_set_wifi_rssi(wifi_info.rssi);
                    //Cập nhật thông tin kết nối Socket server
                    lcd_socketStatus = xLCD_CenterInfor.socketStatus;
                    lcd_set_socket_status(lcd_socketStatus.status);
                    // Cập nhật thông tin phần trăm pin
                    lcd_set_bat_percent(xLCD_CenterInfor.batteryCapacity);
                    // Cập nhật trạng thái chế độ
                    // lcd_set_mode(xLCD_CenterInfor.operationMode);
                    //Cập nhật số thẻ trên màn hình
                    lcd_aliveWire(xLCD_CenterInfor.aliveWireCount);

                }
    
                
                
                //Nhận mảng chứa các thẻ đang alive
                if(x_AliveCard_2_LCD_Task_Queue != NULL) {
                    xQueuePeek(x_AliveCard_2_LCD_Task_Queue, LCD_aliveCardArr, pdMS_TO_TICKS(10));
                }
    
    
                static uint8_t toggleScreenOneTime;


                if (x_Card_2_LCD_Task_Queue != NULL) {
                    while (xQueueReceive(x_Card_2_LCD_Task_Queue, &DataWireReceive, pdMS_TO_TICKS(10)) == pdTRUE) {
                        if (x_Server_2_UHF_RFID_Task != NULL) {
                            CARD tempTCPCard;
                            memset(&tempTCPCard, 0, sizeof(CARD));
                            memcpy(tempTCPCard.epc, DataWireReceive.serial, MAX_EPC_LENGTH);
                            tempTCPCard.result_cards = 1;

                            // Gửi dữ liệu trở lại Task UHF-RFID
                            xQueueSend(x_Server_2_UHF_RFID_Task, &tempTCPCard, pdMS_TO_TICKS(10));
                        }

                        ESP_LOGW(TAG, "Hiển thị thông tin dây nhận được: %s, Color count: %d",
                                DataWireReceive.serial, DataWireReceive.colors.color_count);

                        if (_check_dup_display(display_wires, wireCount, &DataWireReceive) == true) {
                            ESP_LOGW(TAG, "Dup response");
                            continue;  // Bỏ qua hiển thị nếu trùng
                        }

                        memcpy(&DataWireReceiveArr[DataWireReceiveArr_Index++], &DataWireReceive, sizeof(wire_infor));
                        DataWireReceiveArr_Index %= SIZEOF_ARR_CARD_INFOR_FROM_SERVER;

                        // Cờ cho phép chuyển màn hình sau khi nhận dữ liệu mới
                        isNeedToggleScreen = 1;
                        toggleScreenOneTime = 0;

                        // Tăng chỉ số đếm dây hiển thị
                        wireCount = (wireCount + 1) % 6;
                    }
                }
    
                //     end_tick = xTaskGetTickCount();
                //     duration = end_tick - start_tick;

                // printf("Execution time: %lu ticks (%lu ms)\n",
                //     (unsigned long)duration,
                //     (unsigned long)(duration * portTICK_PERIOD_MS));

                if(isNeedToggleScreen == 1) {
                    
                    if(toggleScreenOneTime == 0) {
                        start_blink_effect(ui_Blink1, ui_Blink2);
                        ESP_LOGW(TAG, "\nStart Toggle\n");
                        toggleScreenOneTime = 1;
                        checkTimeToggleScreen = xTaskGetTickCount();
                    }
                    //Xuất hiện trên màn hình 200ms
                    if(toggleScreenOneTime == 1 && (xTaskGetTickCount() - checkTimeToggleScreen >= 100/ portTICK_PERIOD_MS)) {
                        end_blink_effect(ui_Blink1, ui_Blink2);
                        isNeedToggleScreen = 0;
                        ESP_LOGW(TAG, "\nEnd Toggle\n");
                    } 
                    // if (toggleScreenOneTime == 1) {
                    //     vTaskDelay(50 / portTICK_PERIOD_MS); // Giảm thời gian đợi xuống 100ms
                    //     end_blink_effect(ui_Blink1, ui_Blink2);
                    //     isNeedToggleScreen = 0;
                    //     ESP_LOGW(TAG, "\nEnd Toggle\n");
                    // }              
                }
    
                char lcd_location[50] = {0};
                // if(xQueueLocation != NULL) {       
                //     if(xQueueReceive(xQueueLocation, lcd_location, pdMS_TO_TICKS(10)) == pdTRUE) {
                //         lv_label_set_text(ui_WireCheckLoca, "");
                //         // Gọi hàm start_blink_effect sau khi nhận dữ liệu mới từ TCP
                //         BlinkOK = 1;
                //         BlinkOKTime = 0;
                //     }
                // }

                #if 0//Blink
                if(BlinkOK == 1) {
                    
                    if(BlinkOKTime == 0) {
                        lv_label_set_text(ui_WireCheckLoca, lcd_location);
                        start_blink_effect_OK();
                        ESP_LOGW(TAG, "\nStart Toggle location\n");
                        BlinkOKTime = 1;
                        checkTimeToggleScreen = xTaskGetTickCount();
                    }
                    //Xuất hiện trên màn hình 200ms
                    if(BlinkOKTime == 1 && (xTaskGetTickCount() - checkTimeToggleScreen >= 600/ portTICK_PERIOD_MS)) {
                        lv_label_set_text(ui_WireCheckLoca, "");
                        end_blink_effect_OK();
                        BlinkOK = 0;
                        ESP_LOGW(TAG, "\nEnd Toggle location\n");
                    }               
                }            
                #endif
    
                // //Hàm hiển thị mảng Alive - Debug
                // for(uint8_t lcd_count = 0; lcd_count < SIZEOF_CHECK_ALIVE_CARD_ARR; lcd_count++) {
                //     ESP_LOGW(TAG, "\nCheck alive card: %s Flag: %d\n", (char*)LCD_aliveCardArr[lcd_count].epc, LCD_aliveCardArr[lcd_count].isAlive);
                // }
    
                //Xóa những màu tạm thời chuẩn bị cho chu kì hiển thị mới
                for(uint8_t lcd_count = 0; lcd_count < 6; lcd_count++) {
                    //Clear cờ phát hiện thẻ RSSI tốt nhất ở vòng lặp trước
                    LCD_cardInfo[lcd_count].isTheBestRSSI = 0;
                    if(LCD_cardInfo[lcd_count].isOnScreen == 1) {
                        deleteTemporaryWireColor(&LCD_cardInfo[lcd_count].WireInfo);
                    }    
                }
    
                //Kiểm tra mảng những thẻ còn sống
                for(uint8_t lcd_count = 0; lcd_count < SIZEOF_CHECK_ALIVE_CARD_ARR; lcd_count++) {
                    //Nếu thẻ đó đang được đọc và đã có thông tin từ server
                    if(LCD_aliveCardArr[lcd_count].isAlive == 1 && LCD_aliveCardArr[lcd_count].isHasInfor == 1 && LCD_aliveCardArr[lcd_count].isDuplicateInfor == 0 && LCD_aliveCardArr[lcd_count].epc[0] != 0x00) {
                        uint8_t checkCardOnScreen = 0;
                        //Kiểm tra xem thẻ sống có đang được hiển thị không, quét 6 thẻ đang trong mảng hiển thị
                        for(uint8_t lcd_count1 = 0; lcd_count1 < 6; lcd_count1++) {
                            if(LCD_cardInfo[lcd_count1].isOnScreen == 1 && strcmp((char*)LCD_aliveCardArr[lcd_count].epc, LCD_cardInfo[lcd_count1].WireInfo.serial) == 0
                            && LCD_cardInfo[lcd_count1].WireInfo.serial[0] != 0x00) {
                                checkCardOnScreen = 1;
                                //Nếu là thẻ đang chiếm cờ RSSI tốt nhất thì cập nhật vào struct quản lý thẻ trên màn hình
                                if(LCD_aliveCardArr[lcd_count].theBestRSSIValue == 1) {
                                    LCD_cardInfo[lcd_count1].isTheBestRSSI = 1;
                                }
                                //Nếu đã có rồi thì bỏ qua, không xử lý
                                break;
                            }
                        }
                        //Nếu chưa được hiển thị lên
                        if(checkCardOnScreen == 0) {
                            uint8_t isOnScreenOK = 0;
                            //Kiểm tra thông tin của các thẻ đã hiển trị trên màn hình xem có thẻ nào trùng 4 trường thông tin không
                            uint8_t isDuplicateInfor = 0;
                            //Quét 6 vị trí hiển thị trên màn hình
                            for(uint8_t lcd_count1 = 0; lcd_count1 < 6; lcd_count1++) {
                                //Nếu tìm được vị trí trống, chưa hiển thị
                                if(LCD_cardInfo[lcd_count1].isOnScreen == 0) {
                                    //Lọc tìm thông tin trong mảng những thông tin nhận từ server
                                    for(uint8_t lcd_count2 = 0; lcd_count2 < SIZEOF_ARR_CARD_INFOR_FROM_SERVER; lcd_count2++) {
                                        // ESP_LOGW(TAG, "So sanh 2 the, the dang song so %d: %s, hasInfor: %d, isAlive: %d.", lcd_count,  (char*)LCD_aliveCardArr[lcd_count].epc, LCD_aliveCardArr[lcd_count].isHasInfor, LCD_aliveCardArr[lcd_count].isAlive);
                                        // ESP_LOGW(TAG, "Mang chua thong tin the so %d: %s.",lcd_count2,  DataWireReceiveArr[lcd_count2].serial);
                                        // ESP_LOGW(TAG, "Mang chua the tren man hinh: %s, isOnscreen: %d.", LCD_cardInfo[lcd_count1].WireInfo.serial, LCD_cardInfo[lcd_count1].isOnScreen);
                                        // printf("\n---------\n");
                                        //Tìm phần tử nào có mã EPC trùng với thẻ đang alive
                                        if(strcmp((char*)LCD_aliveCardArr[lcd_count].epc, DataWireReceiveArr[lcd_count2].serial) == 0 && DataWireReceiveArr[lcd_count2].serial[0] != 0x00){
                                            // Lọc những thẻ dây trùng 4 trường thông tin như sau
                                            /*
                                            1. Part number
                                            2. Lô
                                            3. WO
                                            4. Group
                                            */                        
                                            //Kiểm tra thông tin của các thẻ đã hiển trị trên màn hình xem có thẻ nào trùng 4 trường thông tin không
                                            #if 0 // kiem tra 4 truong thong tin
                                            for(uint8_t lcd_count3 = 0; lcd_count3 < 6; lcd_count3++) {
                                                if(LCD_cardInfo[lcd_count3].isOnScreen == 1 &&
                                                strcmp(DataWireReceiveArr[lcd_count2].PartNumber, LCD_cardInfo[lcd_count3].WireInfo.PartNumber) == 0 &&
                                                strcmp(DataWireReceiveArr[lcd_count2].lot, LCD_cardInfo[lcd_count3].WireInfo.lot) == 0 &&
                                                strcmp(DataWireReceiveArr[lcd_count2].WO, LCD_cardInfo[lcd_count3].WireInfo.WO) == 0 &&
                                                strcmp(DataWireReceiveArr[lcd_count2].group, LCD_cardInfo[lcd_count3].WireInfo.group) == 0) 
                                                {
                                                    isDuplicateInfor = 1;
                                                    ESP_LOGW(TAG, "Tag %s was duplicated 4 fields information!", DataWireReceiveArr[lcd_count2].serial);
                                                    //Cộng màu cho dây đang hiển thị trên màn hình
                                                    addColorToWireInfor(&LCD_cardInfo[lcd_count3].WireInfo, &DataWireReceiveArr[lcd_count2]);
                                                    // if(x_LCD_2_UHFRFID_Task_Queue != NULL) {
                                                    //     CARD tempLCDCard;
                                                    //     strcpy((char*)tempLCDCard.epc, LCD_cardInfo[lcd_count3].WireInfo.serial);
                                                    //     tempLCDCard.epc_len = strlen(LCD_cardInfo[lcd_count3].WireInfo.serial);
                                                    //     xQueueSend(x_LCD_2_UHFRFID_Task_Queue, &tempLCDCard, pdMS_TO_TICKS(10));
                                                    // }
                                                    //Nếu là thẻ đang chiếm cờ RSSI tốt nhất thì cập nhật vào struct quản lý thẻ trên màn hình
                                                    if(LCD_aliveCardArr[lcd_count].theBestRSSIValue == 1) {
                                                        LCD_cardInfo[lcd_count3].isTheBestRSSI = 1;
                                                    }
                                                    break;  //Break khỏi vòng for lcd_count3
                                                }
                                            }
                                            #endif
                                            if(isDuplicateInfor == 1) { //Break khỏi vòng for lcd_count2
                                                break;
                                            }
                                            
                                            //Gán giá trị và hiển thị lên màn hình
                                            // memset(&LCD_cardInfo[lcd_count1].WireInfo, 0, sizeof(wire_infor));
                                            memcpy(&LCD_cardInfo[lcd_count1].WireInfo, &DataWireReceiveArr[lcd_count2], sizeof(wire_infor));
                                            LCD_cardInfo[lcd_count1].isOnScreen = 1;
                                            //Nếu là thẻ đang chiếm cờ RSSI tốt nhất thì cập nhật vào struct quản lý thẻ trên màn hình
                                            if(LCD_aliveCardArr[lcd_count].theBestRSSIValue == 1) {
                                                LCD_cardInfo[lcd_count1].isTheBestRSSI = 1;
                                            }
                                            
                                            //Nếu tìm được vị trí hiển thị thì thoát vòng kiểm tra và hiển thị
                                            isOnScreenOK = 1;
                                            break;
                                        }
                                    }
                                    
                                }
                                //Nếu đã hiển thị OK thì không tìm vị trí tiếp theo nữa
                                if(isOnScreenOK == 1 || isDuplicateInfor == 1) { //Break khỏi vòng for lcd_count1
                                    break;
                                }
                            } 
                            if(isOnScreenOK == 0 && isDuplicateInfor == 0) { //Trùng thông tin thì không cần báo lỗi
                                ESP_LOGE(TAG, "Khong tim thay thong tin hoac so the da toi da, khong the hien thi them");
                                // //Hàm hiển thị mảng Alive - Debug
                                // for(uint8_t lcd_count = 0; lcd_count < 6; lcd_count++) {
                                //     ESP_LOGW(TAG, "\nCheck lcd card: %s Flag: %d\n", (char*)LCD_cardInfo[lcd_count].WireInfo.serial, LCD_cardInfo[lcd_count].isOnScreen);
                                // }
                            }
                        }
                    }
                }
    
                //Kiểm tra các thẻ trên màn hình
                for(uint8_t lcd_count = 0; lcd_count < 6; lcd_count++) {
                    //Nếu gặp thẻ đang hiển thị thì kiểm ta
                    if(LCD_cardInfo[lcd_count].isOnScreen == 1) {
                        uint8_t checkCardAlive = 0; //Biến kiểm tra xem thẻ hiện trên màn hình còn sống không
                        for(uint8_t lcd_count1 = 0; lcd_count1 < SIZEOF_CHECK_ALIVE_CARD_ARR; lcd_count1++) {
                            //Nếu trùng thì giữ cho thẻ sôngs, loại bỏ trường hợp cả 2 mảng đều trống vẫn trả về giống nhau
                            if(strcmp(LCD_cardInfo[lcd_count].WireInfo.serial, (char*)LCD_aliveCardArr[lcd_count1].epc) == 0 && LCD_aliveCardArr[lcd_count1].epc[0] != 0x00 && LCD_aliveCardArr[lcd_count1].isAlive == 1) {
                                checkCardAlive = 1;
                            }
                        }
                        //Nếu không trùng thẻ nào
                        if(checkCardAlive == 0) {
                            LCD_cardInfo[lcd_count].isOnScreen = 0;
                            // memset(LCD_cardInfo[lcd_count].WireInfo, 0, sizeof(LCD_cardInfo[lcd_count].WireInfo)); 
                            ESP_LOGE(TAG, "\nThe tren man hinh khong con ALive, tien hanh xoa the %s\n", LCD_cardInfo[lcd_count].WireInfo.serial);
                        }
                    }
                }
    
                for(uint8_t lcd_count = 0; lcd_count < 6; lcd_count++) {
                        if(LCD_cardInfo[lcd_count].isOnScreen == 1) {
                            //Kiểm tra các thẻ dây đang có trên màn hình
                            for(uint8_t lcd_count1 = 0; lcd_count1 < 6; lcd_count1++) {
                                if(LCD_cardInfo[lcd_count1].isOnScreen == 1 && lcd_count1 != lcd_count) {
                                    //Nếu trùng màu nhau
                                    if(memcmp(&LCD_cardInfo[lcd_count].WireInfo.colors, &LCD_cardInfo[lcd_count1].WireInfo.colors, sizeof(ColorPairs)) == 0) {
                                        isNeedHighLightWires = 1;   //Cần highlight màu dây
                                    }
                                }
                            }
                        }
                    }
                // //Hàm hiển thị mảng Alive - Debug
                // for(uint8_t lcd_count = 0; lcd_count < 6; lcd_count++) {
                //     ESP_LOGW(TAG, "\nCheck lcd card: %s Flag: %d\n", (char*)LCD_cardInfo[lcd_count].WireInfo.serial, LCD_cardInfo[lcd_count].isOnScreen);
                // }
    
                /************************Thêm code hiển thị dây RSSI cao nhất ở đây************************************/
    
                //Biến lưu chỉ số hiển thị trên màn hình cần HighLight, khởi tạo bằng giá trị sai (không thể có)
                uint8_t HighLightIndex = 10;
                //Reset biến đếm số thẻ trên màn hình để bắt đầu đếm
                LCDInfor.numberOfCardOnScreen = 0;
                for(uint8_t lcd_count = 0; lcd_count < 6; lcd_count++) {
                    if(LCD_cardInfo[lcd_count].isOnScreen == 1) {
                        // Old: lcd_set_daydien_text(&LCD_cardInfo[lcd_count].WireInfo, lcd_count); // Gọi hàm để gửi dữ liệu sang queue hiển thị
                        lcd_set_daydien_text(&LCD_cardInfo[lcd_count].WireInfo, LCDInfor.numberOfCardOnScreen, xLCD_CenterParam.mode); // Gọi hàm hiển thị thông tin thẻ dây
                        if(LCD_cardInfo[lcd_count].isTheBestRSSI == 1) {
                            HighLightIndex = LCDInfor.numberOfCardOnScreen;
                        }
                        LCDInfor.numberOfCardOnScreen ++;
                        // ESP_LOGI(TAG, "\nThẻ đã lên màn hình\n");
                    }
                    else if(LCD_cardInfo[lcd_count].isOnScreen == 0) {
                        lcd_reset_daydien_text(lcd_count); // Gọi hàm xoá thông tin thẻ dây
                        // ESP_LOGW(TAG, "\nCheck Delete OnScreen card: %s Index: %d\n", (char*)LCD_cardInfo[lcd_count].WireInfo.serial, lcd_count);
                    }
                }
                
                set_numberCardOnScreenCL(LCDInfor.numberOfCardOnScreen);
    
                #if 0//Bật highlight
                if(isNeedHighLightWires == 1) {
                    //Nếu tìm được thẻ cần HightLight
                    if(HighLightIndex != 10) {
                        printf("Highlighting LCD index: %d\n", HighLightIndex);
                        highlight_item(HighLightIndex);  // Gọi hàm highlight với index tương ứng
                    }
                    //Nếu không tìm được thẻ cần HightLight
                    else {
                        HideHighlight_item();
                    }
                }
                //Nếu không cần HighLight
                else {
                    HideHighlight_item();
                }
                #endif
                #if 0
                bool found = false;  // Cờ kiểm tra có thẻ nào được highlight không
    
                for (int lcd_count1 = 0; lcd_count1 < 6; lcd_count1++) {
                    if (LCD_cardInfo[lcd_count1].isTheBestRSSI == 1) {
                        highlight_item(lcd_count1);  // Highlight thẻ có RSSI tốt nhất
                        found = true;  // Đánh dấu đã tìm thấy thẻ
                        break;  // Chỉ highlight 1 thẻ
                    }
                }
    
                // Nếu không có thẻ nào có isTheBestRSSI == 1, xóa tất cả highlight
                if (!found) {
                    highlight_item(-1);
                }
                #endif
    
                if(x_LCD_2_Center_Task_Queue != NULL) {
                    xQueueOverwrite(x_LCD_2_Center_Task_Queue, &LCDInfor);
                }
                // printf("\nLVGL Task is working!\n");
    
                // Release the mutex
                example_lvgl_unlock();
                // vTaskDelay(pdMS_TO_TICKS(50));
            }
            if (task_delay_ms > EXAMPLE_LVGL_TASK_MAX_DELAY_MS) {
                task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
            } else if (task_delay_ms < EXAMPLE_LVGL_TASK_MIN_DELAY_MS) {
                task_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS;
            }

            vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
        
            #endif //CL03 mode
        }
    }
#endif //CL03 Mode    
    }


 void app_main(void)
 {
 #if 1
     static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
     static lv_disp_drv_t disp_drv;      // contains callback functions
 
 
     ESP_LOGI(TAG, "Turn off LCD backlight");
     // gpio_config_t bk_gpio_config = {
     //     .mode = GPIO_MODE_OUTPUT,
     //     .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT
     // };
     // ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
 
     // Prepare and then apply the LEDC PWM timer configuration (For BackLight Pinout)
     ledc_timer_config_t ledc_timer = {
         .speed_mode       = LEDC_MODE,
         .timer_num        = LEDC_TIMER,
         .duty_resolution  = LEDC_DUTY_RES,
         .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
         .clk_cfg          = LEDC_AUTO_CLK
     };
     ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
 
     // Prepare and then apply the LEDC PWM channel configuration
     ledc_channel_config_t ledc_channel = {
         .speed_mode     = LEDC_MODE,
         .channel        = LEDC_CHANNEL,
         .timer_sel      = LEDC_TIMER,
         .intr_type      = LEDC_INTR_DISABLE,
         .gpio_num       = LEDC_OUTPUT_IO,
         .duty           = 0, // Set duty to 0%
         .hpoint         = 0
     };
     ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
     // Turn offf Backlight Screen
     ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
     // Update duty to apply the new value
     ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
 
 
     ESP_LOGI(TAG, "Initialize SPI bus");
     spi_bus_config_t buscfg = {
         .sclk_io_num = EXAMPLE_PIN_NUM_SCLK,
         .mosi_io_num = EXAMPLE_PIN_NUM_MOSI,
         .miso_io_num = EXAMPLE_PIN_NUM_MISO,
         .quadwp_io_num = -1,
         .quadhd_io_num = -1,
         .max_transfer_sz = EXAMPLE_LCD_H_RES * 80 * sizeof(uint16_t),
     };
     ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));
 
     ESP_LOGI(TAG, "Install panel IO");
     esp_lcd_panel_io_handle_t io_handle = NULL;
     esp_lcd_panel_io_spi_config_t io_config = {
         .dc_gpio_num = EXAMPLE_PIN_NUM_LCD_DC,
         .cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS,
         .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
         .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
         .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
         .spi_mode = 0,
         .trans_queue_depth = 10,
         .on_color_trans_done = example_notify_lvgl_flush_ready,
         .user_ctx = &disp_drv,
     };
     // Attach the LCD to the SPI bus
     ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));
 
     esp_lcd_panel_handle_t panel_handle = NULL;
     esp_lcd_panel_dev_config_t panel_config = {
         .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
         .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
         .bits_per_pixel = 16,
     };
 #if CONFIG_EXAMPLE_LCD_CONTROLLER_ILI9341
     ESP_LOGI(TAG, "Install ILI9341 panel driver");
     ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));
 #elif CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01
     ESP_LOGI(TAG, "Install GC9A01 panel driver");
     ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle));
 #endif
 
     ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
     ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
 #if CONFIG_EXAMPLE_LCD_CONTROLLER_GC9A01
     ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
 #endif
     // ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
     ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));
 
     // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
     ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
 
 #if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
     esp_lcd_panel_io_handle_t tp_io_handle = NULL;
     esp_lcd_panel_io_spi_config_t tp_io_config = ESP_LCD_TOUCH_IO_SPI_STMPE610_CONFIG(EXAMPLE_PIN_NUM_TOUCH_CS);
     // Attach the TOUCH to the SPI bus
     ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &tp_io_config, &tp_io_handle));
 
     esp_lcd_touch_config_t tp_cfg = {
         .x_max = EXAMPLE_LCD_H_RES,
         .y_max = EXAMPLE_LCD_V_RES,
         .rst_gpio_num = -1,
         .int_gpio_num = -1,
         .flags = {
             .swap_xy = 0,
             .mirror_x = 0,
             .mirror_y = 0,
         },
     };
 
 #if CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_STMPE610
     ESP_LOGI(TAG, "Initialize touch controller STMPE610");
     ESP_ERROR_CHECK(esp_lcd_touch_new_spi_stmpe610(tp_io_handle, &tp_cfg, &tp));
 #endif // CONFIG_EXAMPLE_LCD_TOUCH_CONTROLLER_STMPE610
 #endif // CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
 
     ESP_LOGI(TAG, "Turn on LCD backlight");
     // gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, 1);
 
     ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 8192 * screen_BrightnessPercentage / 100.0));
     // Update duty to apply the new value
     ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
 
     ESP_LOGI(TAG, "Initialize LVGL library");
     lv_init();
     // alloc draw buffers used by LVGL
     // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
     lv_color_t *buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
     assert(buf1);
     lv_color_t *buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
     assert(buf2);
     // initialize LVGL draw buffers
     lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 20);
 
     ESP_LOGI(TAG, "Register display driver to LVGL");
     lv_disp_drv_init(&disp_drv);
     disp_drv.hor_res = EXAMPLE_LCD_H_RES;
     disp_drv.ver_res = EXAMPLE_LCD_V_RES;
     disp_drv.flush_cb = example_lvgl_flush_cb;
     disp_drv.drv_update_cb = example_lvgl_port_update_callback;
     disp_drv.draw_buf = &disp_buf;
     disp_drv.user_data = panel_handle;
     lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
 
     ESP_LOGI(TAG, "Install LVGL tick timer");
     // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
     const esp_timer_create_args_t lvgl_tick_timer_args = {
         .callback = &example_increase_lvgl_tick,
         .name = "lvgl_tick"
     };
     esp_timer_handle_t lvgl_tick_timer = NULL;
     ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
     ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));
 
 #if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
     static lv_indev_drv_t indev_drv;    // Input device driver (Touch)
     lv_indev_drv_init(&indev_drv);
     indev_drv.type = LV_INDEV_TYPE_POINTER;
     indev_drv.disp = disp;
     indev_drv.read_cb = example_lvgl_touch_cb;
     indev_drv.user_data = tp;
 
     lv_indev_drv_register(&indev_drv);
 #endif
 
     lvgl_mux = xSemaphoreCreateRecursiveMutex();
     assert(lvgl_mux);
     
     // esp_lcd_panel_swap_xy(panel_handle, false);
     // esp_lcd_panel_mirror(panel_handle, false, true);
 
     // esp_lcd_panel_swap_xy(panel_handle, true);
     esp_lcd_panel_mirror(panel_handle, false, true);
 
     ESP_LOGI(TAG, "Display LVGL Meter Widget");
     // Lock the mutex due to the LVGL APIs are not thread-safe
     if (example_lvgl_lock(-1)) {
         // example_lvgl_demo_ui(disp);
         ui_init();
         // Release the mutex
         example_lvgl_unlock();
     }
     ESP_LOGI(TAG, "Create LVGL task");
     x_LCD_2_Center_Task_Queue = xQueueCreate( 1, sizeof(LCD_Infor_TypeDef));
     x_LCD_2_UHFRFID_Task_Queue = xQueueCreate( 15, sizeof(CARD));

     NVS_Config();

     xTaskCreate(example_lvgl_port_task, "LVGL", EXAMPLE_LVGL_TASK_STACK_SIZE, NULL, EXAMPLE_LVGL_TASK_PRIORITY, NULL);
 #endif
 
     //vTaskDelay(pdMS_TO_TICKS(500));  // Delay for 500ms
 
     esp_log_level_set("gdma", ESP_LOG_DEBUG);      // Bật log chi tiết cho GDMA
     
     Center_Config();
     UHF_RFID_UART_Init();
     CONNECT_WIFI_Init();
 
     TCP_Init();
     // char DataFromServer[]="[OK]45E00815,LA99,BR,601,TRFC,15,1,2;[OK]45E03088,LA99,SB,3359,TRFC,15,1,304;[OK]45E00891,LA99,G -1,3969,TRFC,15,1,198;[OK]45E00892,LA99,BR,3969,TRFC,15,1,198;[OK]45E00893,LA99,G,3969,TRFC,15,1,198;[OK]45E03086,LA99,L,2374,TRFC,15,1,269;[OK]45E03087,LA99,GR,3359,TRFC,15,1,304;[OK]45E03373,LA99,LG,2000,TRFC,15,1,17;[OK]45E00894,LA99,L,3969,TRFC,15,1,198;[OK]45I1F292,LA01,,1507,589D,20,1,7";
 
     // TachData(DataFromServer);
 
 }
 