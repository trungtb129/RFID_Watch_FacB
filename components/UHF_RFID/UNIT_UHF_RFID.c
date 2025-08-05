/*
Logic xử lý cũ:
1. Lấy dữ liệu từ đầu đọc thẻ lưu vào mảng recv_cards
2. So sánh dữ liệu với mảng cards và CheckDupcards, nếu thẻ đã có trong 2 mảng này sẽ loại đi bởi đã được đọc trước đó
Note: mảng cards lưu các thẻ đã đọc và server đã phản hồi oke,mảng CheckDupcards lưu những thẻ mới đọc xong, làm mới mỗi 2,5s (phụ thuộc macro CHECK_DUPLICATE_TIME)
3. Kiểm tra giá trị RSSI của thẻ (thể hiện chất lượng tín hiệu) nếu đạt ngưỡng thì thực hiện lưu và gửi dữ liệu đến task TCP_CLIENT
4. Nếu thẻ OK sẽ được gửi trở lại bằng queue x_Server_2_UHF_RFID_Task để thực hiện lưu vào mảng cards.
*/
#include "UNIT_UHF_RFID.h"



// #define MY_PRINT(fmt, ...) printf("%s[%d]:" fmt, __func__, __LINE__, ##__VA_ARGS__)
#define MY_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
// #define MY_PRINT(fmt, ...) do { (void)0; } while (0)
#define ARR_PRINT(begin_tag, end_tag, data_ptr, len) do    \
{                                           \
    MY_PRINT("%s (%d bytes): ", begin_tag, len);                \
    for (uint8_t data_index = 0; data_index < len; data_index++)   \
    {                                                              \
        printf("%02X ", data_ptr[data_index]);                   \
    }                                                              \
    printf("%s", end_tag);         \
} while (0);

#define RX_BUF_SIZE 255
#define TX_BUF_SIZE 128

#define CHECK_DUPLICATE_TIME (PERIOD_SEND_TO_SOCKET_TASK*2)
#define SIZEOF_CHECK_DUPLICATE_ARR 10

#define SAVE_CARDS_NUMBER 20
#define TAG "UNIT_UHF_RFID"


extern QueueHandle_t x_Server_2_UHF_RFID_Task;      //Nhận dữ liệu thẻ từ server để lưu vào mảng cards

uint8_t _debug = 0;
uint8_t buffer[RX_BUF_SIZE] = {0};
uint8_t Txbuffer[TX_BUF_SIZE] = {0};
static uint32_t checkDuplicateCardTime = 0;
static uint32_t checkTheBestRSSITime = 0; //Biến hỗ trợ delay cho việc tìm RSSI tốt nhất
uint8_t CheckDupPtr = 0;

uint32_t checkTimeDebug = 0;
uint32_t checkTimeDebug1 = 0;
uint32_t checkTimeDebug2 = 0;

static uint32_t checkPeriodSendToSocketTime = 0;    //Biến hỗ trợ chu kì gửi thẻ đến Task Socket

CARD recv_cards[SAVE_CARDS_NUMBER];             // Mảng lưu trữ các thẻ mới đọc để gửi tới server
CARD CheckDupcards[SIZEOF_CHECK_DUPLICATE_ARR];                         // Mảng lưu trữ thẻ hỗ trợ lọc thẻ trùng, tránh việc gửi thẻ giống nhau liên tục qua Task Socket
CARD cards[SAVE_CARDS_NUMBER];                  // Mảng lưu trữ các thẻ đã đọc (đúng xe hàng)

manage_Card_Alive_t aliveCardArr[SIZEOF_CHECK_ALIVE_CARD_ARR];            //Mảng lưu trữ các thẻ đang đọc được
uint8_t aliveCardArrIndex = 0;                  //Biến lưu con trỏ mảng đang đọc

uint16_t MulPoll_count = 0;                     //Biến lưu trữ chỉ số + 1 của thẻ mới nhất trong mảng cards
uint16_t MulPoll_recv_count = 0;                //Biến lưu trữ chỉ số + 1 của thẻ mới nhất trong mảng recv_cards
//--------------------------------------------------------------------------------------------------
uint8_t write_buffer[]  = {0xab, 0xcd, 0xef, 0xdd};
uint8_t read_buffer[4] = {0};
//--------------------------------------------------------------------------------------------------




// Hardware version 硬件版本
const uint8_t HARDWARE_VERSION_CMD[] = {0xBB, 0x00, 0x03, 0x00,
                                        0x01, 0x00, 0x04, 0x7E};
// Single polling instruction 单次轮询指令
const uint8_t POLLING_ONCE_CMD[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};
// Multiple polling instructions 多次轮询指令
const uint8_t POLLING_MULTIPLE_CMD[] = {0xBB, 0x00, 0x27, 0x00, 0x03,
                                        0x22, 0x27, 0x10, 0x83, 0x7E};
// Set the SELECT mode 设置Select模式
const uint8_t SET_SELECT_MODE_CMD[] = {0xBB, 0x00, 0x12, 0x00,
                                       0x01, 0x01, 0x14, 0x7E};
// Set the SELECT parameter instruction 设置Select参数指令
const uint8_t SET_SELECT_PARAMETER_CMD[] = {
    0xBB, 0x00, 0x0C, 0x00, 0x13, 0x01, 0x00, 0x00, 0x00,
    0x20, 0x60, 0x00, 0x30, 0x75, 0x1F, 0xEB, 0x70, 0x5C,
    0x59, 0x04, 0xE3, 0xD5, 0x0D, 0x70, 0xAD, 0x7E};
// SELECT OK RESPONSE 选中成功响应帧
const uint8_t SET_SELECT_OK[]            = {0xBB, 0x01, 0x0C, 0x00,
                                 0x01, 0x00, 0x0E, 0x7E};
const uint8_t GET_SELECT_PARAMETER_CMD[] = {0xBB, 0x00, 0x0B, 0x00,
                                            0x00, 0x0B, 0x7E};
const uint8_t READ_STORAGE_CMD[]         = {0xBB, 0x00, 0x39, 0x00, 0x09, 0x00,
                                    0x00, 0xFF, 0xFF, 0x03, 0x00, 0x00,
                                    0x00, 0x02, 0x45, 0x7E};
const uint8_t READ_STORAGE_ERROR[]       = {0xBB, 0x01, 0xFF, 0x00,
                                      0x01, 0x09, 0x0A, 0x7E};
const uint8_t WRITE_STORAGE_CMD[]   = {0xBB, 0x00, 0x49, 0x00, 0x0D, 0x00, 0x00,
                                     0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x02,
                                     0x12, 0x34, 0x56, 0x78, 0x6D, 0x7E};
const uint8_t WRITE_STORAGE_ERROR[] = {0xBB, 0x01, 0xFF, 0x00,
                                       0x01, 0x10, 0x0A, 0x7E};
// Set the transmitting power 设置发射功率
const uint8_t SET_TX_POWER[] = {0xBB, 0x00, 0xB6, 0x00, 0x02,
                                0x07, 0xD0, 0x8F, 0x7E};
const uint8_t GET_TX_POWER[] = {0xBB, 0x00, 0xB7, 0x00, 0x00,
0xB7, 0x7E}; 
const uint8_t STOP_MULTIPLE_POLLING_CMD[] = {0xBB, 0x00, 0x28, 0x00, 0x00, 0x28, 0x7E};

const uint8_t SELECT_DENSE_READER_MODE_CMD[] = {0xBB, 0x00, 0xF5, 0x00, 0x01, 0x00, 0xF6, 0x7E};

const uint8_t SELECT_HIGH_SENSITIVITY_MODE_CMD[] = {0xBB, 0x00, 0xF5, 0x00, 0x01, 0x01, 0xF7, 0x7E};



//--------------------------------------------------------------------------------------------------
uint32_t lastPollingCheckTime = 0;

//Prototypes
static UHF_RFID_status filterInputModeFunc(CARD checkCard);
uint8_t selectHighSensitivityMode(void);
uint8_t selectDenseReaderMode(void);

QueueHandle_t x_polling_UHF_RFID_2_Server_Queue;
QueueHandle_t x_reading_UHF_RFID_2_Server_Queue;

QueueHandle_t x_AliveCard_2_LCD_Task_Queue; //Gửi mảng chứa các thẻ đang alive tới màn hình
QueueHandle_t x_Card_2_LCD_Task_Queue;      //Gửi dữ liệu lên màn hình LCD

//Struct nhận thông tin từ Center
center_infor_typedef xUHF_RFID_CenterInfor;
center_param_typedef xUHF_RFID_CenterParam;
//Struct gửi thông tin đến Center
QueueHandle_t x_UHFRFIDModeCL_2_Center_Queue;
uint16_t UHF_RFID_Tx_Power = 25;
uint16_t UHF_RFID_readerMode = 1;
uint16_t old_UHF_RFID_Tx_Power = 15;
uint16_t old_UHF_RFID_readerMode = 1;
int16_t UHF_RFID_readerThreshold = -60;
uint16_t UHF_RFID_numberCardsPolling = 20;



/*!
 *  @attention: epc_len NEED for compareEPC() function
 *  
 *  @retval 0: bằng nhau
 */
static uint8_t compareEPC (CARD *new_card, CARD *cmp_card) {
    uint8_t ret = 1;
    if (new_card->epc_len == cmp_card->epc_len)
    {
        ret = 0;
        for (uint8_t i = 0; i < cmp_card->epc_len; i++)
        {
            if(new_card->epc[i] != cmp_card->epc[i]) {
                ret = 1;
                break;
            }
        }
    }
    return ret;
}

char* hex2str(uint8_t num) {
    static char result[3]; // Mảng char để chứa kết quả, bao gồm 2 ký tự hex và ký tự kết thúc chuỗi
    if (num > 0xf) {
        sprintf(result, "%X", num); // Chuyển đổi số thành chuỗi hex
    } else {
        sprintf(result, "0%X", num); // Chuyển đổi số thành chuỗi hex và thêm "0" ở trước nếu cần
    }
    return result;
}


uint8_t IsNeed2ResetCheckDupcards = 0;
uint8_t sentNoPollData = 0, sentNoReadData = 0;
uint8_t saveAliveCardIndex = 0;         //Biến lưu chỉ số để ghi đè nếu số thẻ đang đọc được lớn hơn 6
uint32_t UHFRFIDDelayTime = DELAY_WHEN_WORKING; //Biến lưu thời gian delay task UHF-RFID, mục đích thay đổi Delay giảm số lần đọc khi rảnh rỗi
uint8_t UHFRFIDDelayTimeTrigger = 0;            //Biến hỗ trợ giảm delay time khi có sự kiện đọc thẻ mới ở IN và đọc lại thẻ ở OUT
uint32_t UHFRFIDDelayCheckTime = 0;             //Biến đếm thời gian hỗ trợ Timeout cho trạng thái Working
static void UHF_RFID_task(void *arg)
{
    static const char *UHF_RFID_TASK_TAG = "UHF_RFID_TASK";
    esp_log_level_set(UHF_RFID_TASK_TAG, ESP_LOG_INFO);
    //uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);

    if (setTxPower(UHF_RFID_Tx_Power) == 1){
        MY_PRINT("Set TxPower: %ddB successfully!\n", UHF_RFID_Tx_Power);
        old_UHF_RFID_Tx_Power = UHF_RFID_Tx_Power;
        vTaskDelay(500 / portTICK_PERIOD_MS);
        MY_PRINT("Check TxPower value: %ddB again!\n", getTxPower());
    }
    else {
        MY_PRINT("Failed to set TxPower!\n");
    }

    if(1) {       
        MY_PRINT("Set High Sensitivity Mode! Result:%d\n", selectHighSensitivityMode());
    }
    else {
        MY_PRINT("Set Dense Reader Mode! Result:%d\n", selectDenseReaderMode());
    }
    

    while (1) 
    {
            //Nhận thông tin từ Center
        if(x_Center_Param_Queue != NULL) {
            //// Peek thành công: xUHF_RFID_CenterInfor chứa thông tin mới từ Center
            if(xQueuePeek(x_Center_Param_Queue, &xUHF_RFID_CenterParam, 10 / portTICK_PERIOD_MS) == pdPASS) { 
            }
        }        
        if(xUHF_RFID_CenterParam.mode == CL01_MODE) 
        {
            #if 01 //CR01_MODE

            //Reset cờ trạng thái mỗi lượt
            UHFRFIDDelayTimeTrigger = 0;
            if(x_Center_Infor_Queue != NULL) {
                //// Peek thành công: xUHF_RFID_CenterInfor chứa thông tin mới từ Center
                if(xQueuePeek(x_Center_Infor_Queue, &xUHF_RFID_CenterInfor, 10 / portTICK_PERIOD_MS) == pdTRUE) { 
                }
            }
            //// Biến static lưu trạng thái chế độ làm việc trước đó
            static workMode_t oldWorkMode = {
                    .operationMode = IN_MODE, //Mặc định khởi động là IN MODE
            };

            //Kiểm tra sự thay đổi chế độ làm việc
            if(oldWorkMode.operationMode != xUHF_RFID_CenterInfor.operationMode) {
                //Nếu chuyển từ OUT Mode sang IN Mode
                if(oldWorkMode.operationMode == OUT_MODE && xUHF_RFID_CenterInfor.operationMode == IN_MODE) {
                    //Làm mới mảng quản lý các thẻ đang Alive
                    memset(aliveCardArr, 0, sizeof(aliveCardArr)); 
                    //Clear queue dữ liệu thẻ
                    if(x_polling_UHF_RFID_2_Server_Queue != NULL) {
                        xQueueReset(x_polling_UHF_RFID_2_Server_Queue);
                    }
                    if(x_Server_2_UHF_RFID_Task != NULL) {
                        xQueueReset(x_Server_2_UHF_RFID_Task);  
                    }                   
                } 
                //Nếu chuyển từ Work Mode sang Sleep Mode
                if(oldWorkMode.operationMode != SLEEP_MODE && xUHF_RFID_CenterInfor.operationMode == SLEEP_MODE) {
                    if(x_polling_UHF_RFID_2_Server_Queue != NULL) {
                        xQueueReset(x_polling_UHF_RFID_2_Server_Queue);
                    }
                    if(x_Server_2_UHF_RFID_Task != NULL) {
                        xQueueReset(x_Server_2_UHF_RFID_Task);  
                    }
                    //Dừng polling
                    stopMulPolling();
                    ESP_LOGI(TAG, "Stop polling & reset work queue!");
                }
                oldWorkMode.operationMode = xUHF_RFID_CenterInfor.operationMode;
            }

            if(xUHF_RFID_CenterInfor.operationMode == SLEEP_MODE) {
                
                vTaskDelay(pdMS_TO_TICKS(5000)); // Delay 5 giây
                continue;
            }
            // 1. Nhận yêu cầu xóa thẻ khỏi mảng Alive từ hàng đợi xQueueDeleteWireRegisterLocation
            if(xQueueDeleteWireRegisterLocation != NULL) {
                CARD Location_server_2_UHFRFID;
                if(xQueueReceive(xQueueDeleteWireRegisterLocation, &Location_server_2_UHFRFID, 0 / portTICK_PERIOD_MS) == pdPASS) {
                    for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                        // Nếu thẻ trùng với 1 phần tử trong mảng Alive và EPC không rỗng
                        if(strcmp((char*)Location_server_2_UHFRFID.epc, (char*)aliveCardArr[checkCount].epc) == 0 && aliveCardArr[checkCount].epc[0] != 0x00) {
                            //Tiến hành xóa thẻ đó trong mảng Alive
                            memset(&aliveCardArr[checkCount], 0, sizeof(manage_Card_Alive_t)); 
                        }
                    }
                }
            }
            // 2. Nhận dữ liệu thẻ từ Task Server và đánh dấu thẻ đã có thông tin
            CARD server_2_UHFRFID;
            if (x_Server_2_UHF_RFID_Task != NULL)
            {
                //Queue nhận thẻ từ Task Socket, kiểm tra và gán cờ cho thẻ nào đã có thông tin
                /* code */
                while(xQueueReceive(x_Server_2_UHF_RFID_Task, &server_2_UHFRFID, 0 / portTICK_PERIOD_MS) == pdTRUE) {
                    for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                        //Nếu thẻ đọc được trùng với thẻ trong mảng Alive
                        if(strcmp((char*)server_2_UHFRFID.epc, (char*)aliveCardArr[checkCount].epc) == 0 && aliveCardArr[checkCount].epc[0] != 0x00) {
                            //Cập nhật rằng thẻ đó đã nhận được thông tin
                            aliveCardArr[checkCount].isHasInfor = 1;
                        }
                    }

                    
                    //////////////////////////////////////////
                }
            }
            // 3. Nhận dữ liệu từ LCD và đánh dấu thẻ bị trùng thông tin
            if(x_LCD_2_UHFRFID_Task_Queue != NULL) {
                CARD lcd_2_UHFRFID;
                while(xQueueReceive(x_LCD_2_UHFRFID_Task_Queue, &lcd_2_UHFRFID, 0 / portTICK_PERIOD_MS) == pdTRUE) {
                    for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                        //Nếu thẻ đọc được trùng với thẻ trong mảng Alive
                        if(aliveCardArr[checkCount].isAlive == 1 && strcmp((char*)lcd_2_UHFRFID.epc, (char*)aliveCardArr[checkCount].epc) == 0 && aliveCardArr[checkCount].epc[0] != 0x00) {
                            aliveCardArr[checkCount].isDuplicateInfor = 1;
                        }
                    }
                }
            }  
            


            if(old_UHF_RFID_Tx_Power != UHF_RFID_Tx_Power) {
                if (setTxPower(UHF_RFID_Tx_Power) == 1){
                    MY_PRINT("Set TxPower: %ddB successfully!\n", UHF_RFID_Tx_Power);
                    old_UHF_RFID_Tx_Power = UHF_RFID_Tx_Power;
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    MY_PRINT("Check TxPower value: %ddB again!\n", getTxPower());
                }
                else {
                    MY_PRINT("Failed to set TxPower!\n");
                }
            }


            uint16_t old_MulPoll_recv_count = MulPoll_recv_count;

            //Polling Multiple
            // MY_PRINT("Polling multiple:\n");
            checkTimeDebug1 = xTaskGetTickCount();
            
            uint8_t result = pollingMultiple(UHF_RFID_numberCardsPolling);

            // Tính thời gian chu kỳ kiểm tra polling và chuyển đổi sang ms
            uint32_t debugCheckTime1 = pdTICKS_TO_MS(xTaskGetTickCount() - checkTimeDebug1);

            // //Hiển thị kết quả
            // printf("Check time debug1: %ld ms!\n", debugCheckTime1);

            // // Lấy giá trị tick count hiện tại một lần
            // uint32_t currentTickCount = xTaskGetTickCount();

            // // Tính thời gian chu kỳ kiểm tra polling và chuyển đổi sang ms
            // uint32_t pollingCheckTime = pdTICKS_TO_MS(currentTickCount - lastPollingCheckTime);

            // // Hiển thị kết quả
            // printf("Check polling cycle-time: %ld ms!\n", pollingCheckTime);

            // // Cập nhật thời gian kiểm tra lần cuối
            // lastPollingCheckTime = currentTickCount;

            /////////////////////////////////////////
            // MY_PRINT("scan result: %d\r\n", result);                       // SAVE_CARDS_NUMBER
            
            // Lưu lại số lượng thẻ đã nhận sau lần polling
            uint16_t new_MulPoll_recv_count = MulPoll_recv_count;

            if (result > 0) { // 
                //Nếu ở chế độ OUT có đọc được thẻ mới thì sẽ bật cờ lên 1 để vào trạng thái Working
                if(xUHF_RFID_CenterInfor.operationMode == OUT_MODE) {
                    UHFRFIDDelayTimeTrigger = 1;
                }
                sentNoPollData = 0;
                // Trường hợp bộ đếm bị vòng (overflow) → cộng thêm số lượng thẻ để tính đúng
                if(old_MulPoll_recv_count > new_MulPoll_recv_count) {
                    new_MulPoll_recv_count += SAVE_CARDS_NUMBER;
                }

                // Lặp qua các thẻ mới được đọc
                for(uint8_t i = old_MulPoll_recv_count; i < new_MulPoll_recv_count; i++) {
                    uint8_t index = i % SAVE_CARDS_NUMBER;
                    recv_cards[i % SAVE_CARDS_NUMBER].result_cards = result;


                    //Kiểm tra thẻ chuyển chế độ
                    //Kiểm tra thẻ chuyển chế độ
                    if(x_UHFRFIDModeCL_2_Center_Queue != NULL) {
                        if(strncmp((char*)recv_cards[index].epc, "CL01", 4) == 0) {
                            if(x_UHFRFIDModeCL_2_Center_Queue != NULL) {
                                // Bằng 0: Không thực hiện || Bằng 1: IN-Mode || Bằng 2: OUT-Mode || Bằng 3: SLEEP-Mode || Bằng 4: CL01-Mode ||bằng 5: CL03-Mode
                                xUHF_RFID_CenterParam.mode = CL01_MODE;
                                // ESP_LOGI(TAG, "Check reset: %d", __LINE__);
                                xQueueSend(x_UHFRFIDModeCL_2_Center_Queue, &xUHF_RFID_CenterParam.mode, pdMS_TO_TICKS(20));
                            }   
                            continue;
                        }
                        else if(strncmp((char*)recv_cards[index].epc, "CL02", 4) == 0) {
                            if(x_UHFRFIDModeCL_2_Center_Queue != NULL) {
                                // Bằng 0: Không thực hiện || Bằng 1: IN-Mode || Bằng 2: OUT-Mode || Bằng 3: SLEEP-Mode || Bằng 4: CL01-Mode ||bằng 5: CL03-Mode
                                xUHF_RFID_CenterParam.mode = CL03_MODE;
                                // ESP_LOGI(TAG, "Check reset: %d", __LINE__);
                                xQueueSend(x_UHFRFIDModeCL_2_Center_Queue, &xUHF_RFID_CenterParam.mode, pdMS_TO_TICKS(20));
                            }   
                            continue;
                        }
                    }

                    if(x_UHFRFID_2_Center_Queue != NULL) { 
                        if(strncmp((char*)recv_cards[index].epc, "IN", 3) == 0) {
                            //Kiểm tra nếu thẻ OUT có chất lượng RSSI thấp hơn ngưỡng thì bỏ qua
                            if(recv_cards[index].rssi < RSSI_THRESHOLD_IN_CHECKPOINT) {
                                continue;
                            }
                            if(x_UHFRFID_2_Center_Queue != NULL) {
                                // Bằng 0: Không thực hiện || Bằng 1: IN-Mode || Bằng 2: OUT-Mode || Bằng 3: SLEEP-Mode
                                uint8_t UHFRFIDSendToCenterData = 1;
                                xQueueSend(x_UHFRFID_2_Center_Queue, &UHFRFIDSendToCenterData, pdMS_TO_TICKS(20));
                            }
                            continue;
                        }
                        // TH2: Thẻ yêu cầu chuyển sang OUT_MODE
                        else if(strncmp((char*)recv_cards[index].epc, "OUT", 4) == 0) {
                            //Kiểm tra nếu thẻ OUT có chất lượng RSSI thấp hơn ngưỡng thì bỏ qua
                            if(recv_cards[index].rssi < RSSI_THRESHOLD_OUT_CHECKPOINT) {
                                continue;
                            }
                            if(x_UHFRFID_2_Center_Queue != NULL) {
                                // Bằng 0: Không thực hiện || Bằng 1: IN-Mode || Bằng 2: OUT-Mode || Bằng 3: SLEEP-Mode
                                uint8_t UHFRFIDSendToCenterData = 2;
                                xQueueSend(x_UHFRFID_2_Center_Queue, &UHFRFIDSendToCenterData, pdMS_TO_TICKS(20));
                            }   
                            continue;
                        }
                        else if(strncmp((char*)recv_cards[index].epc, "SLEEP", 6) == 0) {
                            if(x_UHFRFID_2_Center_Queue != NULL) {
                                // Bằng 0: Không thực hiện || Bằng 1: IN-Mode || Bằng 2: OUT-Mode || Bằng 3: SLEEP-Mode
                                uint8_t UHFRFIDSendToCenterData = 3;
                                xQueueSend(x_UHFRFID_2_Center_Queue, &UHFRFIDSendToCenterData, pdMS_TO_TICKS(20));
                            }   
                            continue;
                        }
                    }
                    
                    // ===== Xử lý theo từng chế độ hoạt động của hệ thống =====
                    if(xUHF_RFID_CenterInfor.operationMode == IN_MODE) {
                        //Nếu thẻ không qua được khỏi bộ lọc thì bỏ qua thẻ
                        if(filterInputModeFunc(recv_cards[index]) != UHF_RFID_OK) {
                            continue;
                        }
                    }
                    else if (xUHF_RFID_CenterInfor.operationMode == OUT_MODE) {
                        if((int8_t)recv_cards[index].rssi < SET_OUTMODE_READ_THRESHOLD) {
                            continue;
                        }
                    }

                    
                    
                    uint8_t CheckCardBeforeSend = 0; 
                    

                        
                    uint8_t isDuplicateAlive = 0;       //Cờ check thẻ có bị trùng với trong mảng không
                    for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                        //Nếu thẻ đọc được trùng với thẻ trong mảng Alive
                        if(strcmp((char*)recv_cards[index].epc, (char*)aliveCardArr[checkCount].epc) == 0 && aliveCardArr[checkCount].epc[0] != 0x00) {
                            //Cập nhật lại thời gian đọc được gần nhất
                            aliveCardArr[checkCount].lastAliveTime = xTaskGetTickCount();
                            aliveCardArr[checkCount].isAlive = 1;       //Cờ báo trạng thái đang alive
                            aliveCardArr[checkCount].current_rssi = recv_cards[index].rssi;    //Gán dữ liệu RSSI
                            isDuplicateAlive = 1;
                            // //Hàm hiển thị mảng Alive - Debug
                            // for(uint8_t lcd_count = 0; lcd_count < SIZEOF_CHECK_ALIVE_CARD_ARR; lcd_count++) {
                            //     ESP_LOGW(TAG, "\nCheck alive card: %s Flag: %d\n", (char*)aliveCardArr[lcd_count].epc, aliveCardArr[lcd_count].isAlive);
                            // }
                            break;      //Thoát khỏi for bởi đã cập nhật thời gian xong
                        }
                    }
                    //Nếu không trùng trong mảng, tức là thẻ mới, tiến hành lưu vào mảng alive (ghi đè nếu đầy)
                    //Chỉ tiến hành ghi thẻ ở chế độ Input, ở chế độ Output, ta không quan tâm các thẻ mới
                    if(isDuplicateAlive == 0 && xUHF_RFID_CenterInfor.operationMode == IN_MODE) {
                        // Nếu ở chế độ IN có đọc được thẻ mới, thêm vào mảng ALive thì bật cờ lên 1 để vào trạng thái Working
                        UHFRFIDDelayTimeTrigger = 1;
                        uint8_t isSaveAliveCard = 0;
                        for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                            //Tìm kiếm vị trí trống trong mảng check a live
                            if(aliveCardArr[checkCount].isAlive == 0) {
                                isSaveAliveCard = 1;    //Cập nhật cờ theo dõi
                                //Cập nhật lại thời gian đọc được gần nhất
                                aliveCardArr[checkCount].lastAliveTime = xTaskGetTickCount();
                                aliveCardArr[checkCount].isAlive = 1;               //Cờ báo trạng thái đang alive
                                aliveCardArr[checkCount].isHasInfor = 0;    //Reset cờ báo trạng thái có thông tin
                                aliveCardArr[checkCount].isDuplicateInfor = 0;    //Reset cờ báo trạng thái trùng lặp thông tin
                                memset(aliveCardArr[checkCount].epc, 0, sizeof(aliveCardArr[checkCount].epc));
                                strcpy((char*)aliveCardArr[checkCount].epc, (char*) recv_cards[index].epc);
                                aliveCardArr[checkCount].epc_len = recv_cards[index].epc_len;
                                aliveCardArr[checkCount].current_rssi = recv_cards[index].rssi;    //Gán dữ liệu RSSI
                                isDuplicateAlive = 1;
                                ESP_LOGE(TAG, "\nAdd new alive card: %s\n", aliveCardArr[checkCount].epc);
                                break;      //Thoát khỏi for để không tiến hành ghi nữa
                            }
                        }
                        //Nếu vẫn không ghi được do không còn trống, tiến hành ghi đè
                        if(isSaveAliveCard == 0) {
                            aliveCardArr[saveAliveCardIndex].lastAliveTime = xTaskGetTickCount();
                            aliveCardArr[saveAliveCardIndex].isAlive = 1;       //Cờ báo trạng thái đang alive
                            aliveCardArr[saveAliveCardIndex].isHasInfor = 0;    //Reset cờ báo trạng thái có thông tin
                            aliveCardArr[saveAliveCardIndex].isDuplicateInfor = 0;    //Reset cờ báo trạng thái trùng lặp thông tin
                            aliveCardArr[saveAliveCardIndex].current_rssi = recv_cards[index].rssi;    //Gán dữ liệu RSSI
                            memset(aliveCardArr[saveAliveCardIndex].epc, 0, sizeof(aliveCardArr[saveAliveCardIndex].epc));
                            strcpy((char*)aliveCardArr[saveAliveCardIndex].epc, (char*)recv_cards[index].epc);
                            aliveCardArr[saveAliveCardIndex].epc_len = recv_cards[index].epc_len;
                            saveAliveCardIndex++;
                            saveAliveCardIndex = saveAliveCardIndex % SIZEOF_CHECK_ALIVE_CARD_ARR;
                            ESP_LOGW(TAG, "\nAdd alive card: %s\n", aliveCardArr[saveAliveCardIndex].epc);
                        }
                        // //Hàm hiển thị mảng Alive - Debug
                        // for(uint8_t lcd_count = 0; lcd_count < SIZEOF_CHECK_ALIVE_CARD_ARR; lcd_count++) {
                        //     ESP_LOGW(TAG, "\nCheck alive card: %s Flag: %d\n", (char*)aliveCardArr[lcd_count].epc, aliveCardArr[lcd_count].isAlive);
                        // }
                    }    

                    // vTaskDelay(10 / portTICK_PERIOD_MS);
                }
                
            }
            

            /*
            Ở chế độ Input, với chu kì PERIOD_SEND_TO_SOCKET_TASK, ta sẽ gửi những thẻ đang ALive và chưa có thông tin tới Task Socket
            để lấy thông tin từ server.
            */
            if( xUHF_RFID_CenterInfor.socketStatus.status != CONNECTED) {
                xQueueReset(x_polling_UHF_RFID_2_Server_Queue);
            }
            uint8_t checkAliveCardCount = 0;
            static uint16_t isReSendToServer = 0;
            static uint32_t reSendToServerCheckTime = 0;
            if(xTaskGetTickCount() - reSendToServerCheckTime >= 2000 / portTICK_PERIOD_MS) {
                isReSendToServer = 1;
                reSendToServerCheckTime = xTaskGetTickCount();
            }
            for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                if(aliveCardArr[checkCount].isAlive == 1) {
                    checkAliveCardCount++;
                    if(aliveCardArr[checkCount].isHasInfor == 0 && aliveCardArr[checkCount].isSendToServer == 1 && isReSendToServer > 0) {
                        aliveCardArr[checkCount].isSendToServer = 0;
                    }
                }
            }

            // Neu reset xong thi reset flag
            isReSendToServer = 0;

            if(xQueueSendAliveWireCount != NULL) {
                xQueueOverwrite(xQueueSendAliveWireCount, &checkAliveCardCount);
            }
            // ESP_LOGI(TAG, "xUHF_RFID_CenterInfor.operationMode= %d", xUHF_RFID_CenterInfor.operationMode);
            //     ESP_LOGI(TAG, "xUHF_RFID_CenterInfor.socketStatus.status= %d", xUHF_RFID_CenterInfor.socketStatus.status);
                // ESP_LOGI(TAG, "xUHF_RFID_CenterInfor.operationMode= %d", xUHF_RFID_CenterInfor.operationMode);
            if (xUHF_RFID_CenterInfor.operationMode == IN_MODE && x_polling_UHF_RFID_2_Server_Queue != NULL && xUHF_RFID_CenterInfor.socketStatus.status == CONNECTED)
            {
                uint32_t tempPeriodSendSocketTime = xTaskGetTickCount();
                if(tempPeriodSendSocketTime - checkPeriodSendToSocketTime >= PERIOD_SEND_TO_SOCKET_TASK / portTICK_PERIOD_MS) {
                    // MY_PRINT("Chuẩn bị gửi thẻ.\n");
                    CARD tempSendCard;
                    for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                        //Nếu thẻ đang ALive và chưa có thông tin thì gửi qua Task Socket //Code moi
                        // ESP_LOGI(TAG, "aliveCardArr[%d].isAlive= %d", checkCount, aliveCardArr[checkCount].isAlive);
                        // ESP_LOGI(TAG, "aliveCardArr[%d].isHasInfor= %d", checkCount, aliveCardArr[checkCount].isHasInfor);
                        // ESP_LOGI(TAG, "aliveCardArr[%d].isSendToServer= %d", checkCount, aliveCardArr[checkCount].isSendToServer);
                        #if 1// Bật cờ chỉ cho gửi 1 lần lên server
                        if(aliveCardArr[checkCount].isAlive == 1 && aliveCardArr[checkCount].isHasInfor == 0 && aliveCardArr[checkCount].isSendToServer == 0) 
                        { 
                            memset(&tempSendCard, 0, sizeof(CARD));
                            memcpy(tempSendCard.epc, aliveCardArr[checkCount].epc, aliveCardArr[checkCount].epc_len);
                            tempSendCard.epc_len = aliveCardArr[checkCount].epc_len;
                            tempSendCard.result_cards = 1;
    
                            
                            
                            // ESP_LOGI(TAG, "Send card: %s", tempSendCard.epc);
                            if (xQueueSend(x_polling_UHF_RFID_2_Server_Queue, &tempSendCard, 0 / portTICK_PERIOD_MS) == pdTRUE )
                            {   
                                // //Hàm hiển thị mảng Alive - Debug
                                // for(uint8_t lcd_count = 0; lcd_count < SIZEOF_CHECK_ALIVE_CARD_ARR; lcd_count++) {
                                //     ESP_LOGW(TAG, "\nCheck alive card: %s Flag alive: %d Flag hasInfor: %d\n", (char*)aliveCardArr[lcd_count].epc, aliveCardArr[lcd_count].isAlive, aliveCardArr[lcd_count].isHasInfor);
                                // }
                                ESP_LOGI(TAG, "Successed to post the EPC to task socket: %s.\n", (char*)tempSendCard.epc);

                                // Đánh dấu đã gửi thẻ lên server
                                aliveCardArr[checkCount].isSendToServer = 1;
                                
                            }
                            else
                            {
                                // MY_PRINT("Failed to post the EPC to task socket.\n");
                            }
                        }
                        #endif
                    }
                    //Cập nhật thời gian, đợi chu kì tiếp theo
                    checkPeriodSendToSocketTime = tempPeriodSendSocketTime;
                }
                
            }
        
        

            //Ở chế độ Input Mode, ta sẽ không xóa thẻ
            if(xUHF_RFID_CenterInfor.operationMode == IN_MODE) {
                uint32_t tempLastTimeAlive = xTaskGetTickCount();
                for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                    if(aliveCardArr[checkCount].isAlive == 1) {
                        aliveCardArr[checkCount].lastAliveTime = tempLastTimeAlive;
                    }              
                }
            }
            //Ở chế độ hoạt động khác, ta sẽ xóa thẻ nếu quá Timeout
            else if(xUHF_RFID_CenterInfor.operationMode != IN_MODE){
                for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                    //Tìm kiếm vị trí trống trong mảng check a live
                    if(aliveCardArr[checkCount].isAlive == 1 && xTaskGetTickCount() - aliveCardArr[checkCount].lastAliveTime >= CHECK_ALIVE_CARD_TIMEOUT/ portTICK_PERIOD_MS) {
                        aliveCardArr[checkCount].isAlive = 0;       //Cờ báo trạng thái đang alive
                        aliveCardArr[checkCount].current_rssi = 0;  //Reset giá trị RSSI
                        ESP_LOGW(TAG, "\nClear card: %s\n", aliveCardArr[checkCount].epc);
                        // memset(aliveCardArr[checkCount].epc, 0, sizeof(aliveCardArr[checkCount].epc)); 
                    }
                }
            }
            #if 1
            if (xUHF_RFID_CenterInfor.operationMode == IN_MODE) {
                for (uint8_t checkRSSI = 0; checkRSSI < SIZEOF_CHECK_ALIVE_CARD_ARR; checkRSSI++) {
                    aliveCardArr[checkRSSI].theBestRSSIValue = 2;
                }
            }
            else if (xUHF_RFID_CenterInfor.operationMode == OUT_MODE) { 
                if (xTaskGetTickCount() - checkTheBestRSSITime >= (CHECK_THE_BEST_RSSI_DELAY_TIME / portTICK_PERIOD_MS)) {
                    
                    // Tìm thẻ có thông tin và RSSI tốt nhất
                    uint8_t theBestRSSI_Index = SIZEOF_CHECK_ALIVE_CARD_ARR + 1;
                    int8_t theWorstRSSI_Value = -100;
            
                    // Reset tất cả theBestRSSIValue trước khi quét
                    for (uint8_t i = 0; i < SIZEOF_CHECK_ALIVE_CARD_ARR; i++) {
                        aliveCardArr[i].theBestRSSIValue = 0;
                    }
                    #if 1
                    for (uint8_t checkRSSI = 0; checkRSSI < SIZEOF_CHECK_ALIVE_CARD_ARR; checkRSSI++) {
                        if (aliveCardArr[checkRSSI].isAlive == 1 &&
                            aliveCardArr[checkRSSI].isHasInfor == 1 &&
                            aliveCardArr[checkRSSI].current_rssi != 0) {
                            
                            if (aliveCardArr[checkRSSI].current_rssi >= theWorstRSSI_Value) {
                                theWorstRSSI_Value = aliveCardArr[checkRSSI].current_rssi;
                                theBestRSSI_Index = checkRSSI;
                            }
                        }
                    }
                    #endif
            
                    // Nếu tìm thấy thẻ có RSSI tốt nhất, đánh dấu nó
                    if (theBestRSSI_Index != SIZEOF_CHECK_ALIVE_CARD_ARR + 1) {
                        aliveCardArr[theBestRSSI_Index].theBestRSSIValue = 1;
                    }
            
                    checkTheBestRSSITime = xTaskGetTickCount();
                }
            }
            
            #endif

            if(x_AliveCard_2_LCD_Task_Queue != NULL) {
                xQueueOverwrite(x_AliveCard_2_LCD_Task_Queue, aliveCardArr);
            }

            //Nếu có sự kiện Working thì cập nhật biến checkTime
            uint32_t currentTime = xTaskGetTickCount();
            if(UHFRFIDDelayTimeTrigger) {
                UHFRFIDDelayCheckTime = currentTime;
            }
            if(currentTime - UHFRFIDDelayCheckTime >= TIMEOUT_WORKING_TIME / portTICK_PERIOD_MS) {
                UHFRFIDDelayTime = DELAY_WHEN_WORKING;
            }
            else {
                UHFRFIDDelayTime = DELAY_WHEN_IDLE_TIME; 
            }

            vTaskDelay(UHFRFIDDelayTime / portTICK_PERIOD_MS);

            #endif
        }

        /*********************************************************************************** */
        else if(xUHF_RFID_CenterParam.mode == CL03_MODE) 
        {
            #if 01 //CL03_MODE
            //Nhận thông tin từ Center
            if(x_Center_Infor_Queue != NULL) {
                //// Peek thành công: xUHF_RFID_CenterInfor chứa thông tin mới từ Center
                if(xQueuePeek(x_Center_Infor_Queue, &xUHF_RFID_CenterInfor, 10 / portTICK_PERIOD_MS) == pdTRUE) { 
                }
            }


            // 1. Nhận yêu cầu xóa thẻ khỏi mảng Alive từ hàng đợi xQueueDeleteWireRegisterLocation
            if(xQueueDeleteWireRegisterLocation != NULL) {
                CARD Location_server_2_UHFRFID;
                if(xQueueReceive(xQueueDeleteWireRegisterLocation, &Location_server_2_UHFRFID, 0 / portTICK_PERIOD_MS) == pdPASS) {
                    for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                        // Nếu thẻ trùng với 1 phần tử trong mảng Alive và EPC không rỗng
                        if(strcmp((char*)Location_server_2_UHFRFID.epc, (char*)aliveCardArr[checkCount].epc) == 0 && aliveCardArr[checkCount].epc[0] != 0x00) {
                            //Tiến hành xóa thẻ đó trong mảng Alive
                            memset(&aliveCardArr[checkCount], 0, sizeof(manage_Card_Alive_t)); 
                        }
                    }
                }
            }
            // 2. Nhận dữ liệu thẻ từ Task Server và đánh dấu thẻ đã có thông tin
            CARD server_2_UHFRFID;
            if (x_Server_2_UHF_RFID_Task != NULL)
            {
                //Queue nhận thẻ từ Task Socket, kiểm tra và gán cờ cho thẻ nào đã có thông tin
                /* code */
                while(xQueueReceive(x_Server_2_UHF_RFID_Task, &server_2_UHFRFID, 0 / portTICK_PERIOD_MS) == pdTRUE) {
                    for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                        //Nếu thẻ đọc được trùng với thẻ trong mảng Alive
                        if(strcmp((char*)server_2_UHFRFID.epc, (char*)aliveCardArr[checkCount].epc) == 0 && aliveCardArr[checkCount].epc[0] != 0x00) {
                            //Cập nhật rằng thẻ đó đã nhận được thông tin
                            aliveCardArr[checkCount].isHasInfor = 1;
                        }
                    }

                    
                    //////////////////////////////////////////
                }
            }
            // 3. Nhận dữ liệu từ LCD và đánh dấu thẻ bị trùng thông tin
            if(x_LCD_2_UHFRFID_Task_Queue != NULL) {
                CARD lcd_2_UHFRFID;
                while(xQueueReceive(x_LCD_2_UHFRFID_Task_Queue, &lcd_2_UHFRFID, 0 / portTICK_PERIOD_MS) == pdTRUE) {
                    for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                        //Nếu thẻ đọc được trùng với thẻ trong mảng Alive
                        if(aliveCardArr[checkCount].isAlive == 1 && strcmp((char*)lcd_2_UHFRFID.epc, (char*)aliveCardArr[checkCount].epc) == 0 && aliveCardArr[checkCount].epc[0] != 0x00) {
                            aliveCardArr[checkCount].isDuplicateInfor = 1;
                        }
                    }
                }
            }  
            


            if(old_UHF_RFID_Tx_Power != UHF_RFID_Tx_Power) {
                if (setTxPower(UHF_RFID_Tx_Power) == 1){
                    MY_PRINT("Set TxPower: %ddB successfully!\n", UHF_RFID_Tx_Power);
                    old_UHF_RFID_Tx_Power = UHF_RFID_Tx_Power;
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    MY_PRINT("Check TxPower value: %ddB again!\n", getTxPower());
                }
                else {
                    MY_PRINT("Failed to set TxPower!\n");
                }
            }


            uint16_t old_MulPoll_recv_count = MulPoll_recv_count;

            //Polling Multiple
            // MY_PRINT("Polling multiple:\n");
            checkTimeDebug1 = xTaskGetTickCount();
            
            uint8_t result = pollingMultiple(UHF_RFID_numberCardsPolling);

            // Tính thời gian chu kỳ kiểm tra polling và chuyển đổi sang ms
            uint32_t debugCheckTime1 = pdTICKS_TO_MS(xTaskGetTickCount() - checkTimeDebug1);

            // //Hiển thị kết quả
            // printf("Check time debug1: %ld ms!\n", debugCheckTime1);

            // // Lấy giá trị tick count hiện tại một lần
            // uint32_t currentTickCount = xTaskGetTickCount();

            // // Tính thời gian chu kỳ kiểm tra polling và chuyển đổi sang ms
            // uint32_t pollingCheckTime = pdTICKS_TO_MS(currentTickCount - lastPollingCheckTime);

            // // Hiển thị kết quả
            // printf("Check polling cycle-time: %ld ms!\n", pollingCheckTime);

            // // Cập nhật thời gian kiểm tra lần cuối
            // lastPollingCheckTime = currentTickCount;

            /////////////////////////////////////////
            // MY_PRINT("scan result: %d\r\n", result);                       // SAVE_CARDS_NUMBER
            
            // Lưu lại số lượng thẻ đã nhận sau lần polling
            uint16_t new_MulPoll_recv_count = MulPoll_recv_count;

            if (result > 0) { // 
                sentNoPollData = 0;
                // Trường hợp bộ đếm bị vòng (overflow) → cộng thêm số lượng thẻ để tính đúng
                if(old_MulPoll_recv_count > new_MulPoll_recv_count) {
                    new_MulPoll_recv_count += SAVE_CARDS_NUMBER;
                }

                // Lặp qua các thẻ mới được đọc
                for(uint8_t i = old_MulPoll_recv_count; i < new_MulPoll_recv_count; i++) {
                    uint8_t index = i % SAVE_CARDS_NUMBER;
                    recv_cards[i % SAVE_CARDS_NUMBER].result_cards = result;

                    //Kiểm tra thẻ chuyển chế độ
                    if(x_UHFRFIDModeCL_2_Center_Queue != NULL) {
                        if(strncmp((char*)recv_cards[index].epc, "CL01", 4) == 0) {
                            if(x_UHFRFIDModeCL_2_Center_Queue != NULL) {
                                // Bằng 0: Không thực hiện || Bằng 1: IN-Mode || Bằng 2: OUT-Mode || Bằng 3: SLEEP-Mode || Bằng 4: CL01-Mode ||bằng 5: CL03-Mode
                                xUHF_RFID_CenterParam.mode = CL01_MODE;
                                // ESP_LOGI(TAG, "Check reset: %d", __LINE__);
                                xQueueSend(x_UHFRFIDModeCL_2_Center_Queue, &xUHF_RFID_CenterParam.mode, pdMS_TO_TICKS(20));
                            }   
                            continue;
                        }
                        else if(strncmp((char*)recv_cards[index].epc, "CL02", 4) == 0) {
                            if(x_UHFRFIDModeCL_2_Center_Queue != NULL) {
                                // Bằng 0: Không thực hiện || Bằng 1: IN-Mode || Bằng 2: OUT-Mode || Bằng 3: SLEEP-Mode || Bằng 4: CL01-Mode ||bằng 5: CL03-Mode
                                xUHF_RFID_CenterParam.mode = CL03_MODE;;
                                xQueueSend(x_UHFRFIDModeCL_2_Center_Queue, &xUHF_RFID_CenterParam.mode, pdMS_TO_TICKS(20));
                            }   
                            continue;
                        }
                    }
                    
                    uint8_t CheckCardBeforeSend = 0; 
                    

                        
                    uint8_t isDuplicateAlive = 0;       //Cờ check thẻ có bị trùng với trong mảng không
                    for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                        //Nếu thẻ đọc được trùng với thẻ trong mảng Alive
                        if(strcmp((char*)recv_cards[index].epc, (char*)aliveCardArr[checkCount].epc) == 0 && aliveCardArr[checkCount].epc[0] != 0x00) {
                            //Cập nhật lại thời gian đọc được gần nhất
                            aliveCardArr[checkCount].lastAliveTime = xTaskGetTickCount();
                            aliveCardArr[checkCount].isAlive = 1;       //Cờ báo trạng thái đang alive
                            aliveCardArr[checkCount].current_rssi = recv_cards[index].rssi;    //Gán dữ liệu RSSI
                            isDuplicateAlive = 1;
                            // //Hàm hiển thị mảng Alive - Debug
                            // for(uint8_t lcd_count = 0; lcd_count < SIZEOF_CHECK_ALIVE_CARD_ARR; lcd_count++) {
                            //     ESP_LOGW(TAG, "\nCheck alive card: %s Flag: %d\n", (char*)aliveCardArr[lcd_count].epc, aliveCardArr[lcd_count].isAlive);
                            // }
                            break;      //Thoát khỏi for bởi đã cập nhật thời gian xong
                        }
                    }
                    //Nếu không trùng trong mảng, tức là thẻ mới, tiến hành lưu vào mảng alive (ghi đè nếu đầy)
                    //Chỉ tiến hành ghi thẻ ở chế độ Input, ở chế độ Output, ta không quan tâm các thẻ mới
                    if(isDuplicateAlive == 0) {
                        // Nếu ở chế độ IN có đọc được thẻ mới, thêm vào mảng ALive thì bật cờ lên 1 để vào trạng thái Working
                        uint8_t isSaveAliveCard = 0;
                        for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                            //Tìm kiếm vị trí trống trong mảng check a live
                            if(aliveCardArr[checkCount].isAlive == 0) {
                                isSaveAliveCard = 1;    //Cập nhật cờ theo dõi
                                //Cập nhật lại thời gian đọc được gần nhất
                                aliveCardArr[checkCount].lastAliveTime = xTaskGetTickCount();
                                aliveCardArr[checkCount].isAlive = 1;               //Cờ báo trạng thái đang alive
                                aliveCardArr[checkCount].isHasInfor = 0;    //Reset cờ báo trạng thái có thông tin
                                aliveCardArr[checkCount].isDuplicateInfor = 0;    //Reset cờ báo trạng thái trùng lặp thông tin
                                memset(aliveCardArr[checkCount].epc, 0, sizeof(aliveCardArr[checkCount].epc));
                                strcpy((char*)aliveCardArr[checkCount].epc, (char*) recv_cards[index].epc);
                                aliveCardArr[checkCount].epc_len = recv_cards[index].epc_len;
                                aliveCardArr[checkCount].current_rssi = recv_cards[index].rssi;    //Gán dữ liệu RSSI
                                isDuplicateAlive = 1;
                                ESP_LOGE(TAG, "\nAdd new alive card: %s\n", aliveCardArr[checkCount].epc);
                                break;      //Thoát khỏi for để không tiến hành ghi nữa
                            }
                        }
                        //Nếu vẫn không ghi được do không còn trống, tiến hành ghi đè
                        if(isSaveAliveCard == 0) {
                            aliveCardArr[saveAliveCardIndex].lastAliveTime = xTaskGetTickCount();
                            aliveCardArr[saveAliveCardIndex].isAlive = 1;       //Cờ báo trạng thái đang alive
                            aliveCardArr[saveAliveCardIndex].isHasInfor = 0;    //Reset cờ báo trạng thái có thông tin
                            aliveCardArr[saveAliveCardIndex].isDuplicateInfor = 0;    //Reset cờ báo trạng thái trùng lặp thông tin
                            aliveCardArr[saveAliveCardIndex].current_rssi = recv_cards[index].rssi;    //Gán dữ liệu RSSI
                            memset(aliveCardArr[saveAliveCardIndex].epc, 0, sizeof(aliveCardArr[saveAliveCardIndex].epc));
                            strcpy((char*)aliveCardArr[saveAliveCardIndex].epc, (char*)recv_cards[index].epc);
                            aliveCardArr[saveAliveCardIndex].epc_len = recv_cards[index].epc_len;
                            saveAliveCardIndex++;
                            saveAliveCardIndex = saveAliveCardIndex % SIZEOF_CHECK_ALIVE_CARD_ARR;
                            ESP_LOGW(TAG, "\nAdd alive card: %s\n", aliveCardArr[saveAliveCardIndex].epc);
                        }
                        // //Hàm hiển thị mảng Alive - Debug
                        // for(uint8_t lcd_count = 0; lcd_count < SIZEOF_CHECK_ALIVE_CARD_ARR; lcd_count++) {
                        //     ESP_LOGW(TAG, "\nCheck alive card: %s Flag: %d\n", (char*)aliveCardArr[lcd_count].epc, aliveCardArr[lcd_count].isAlive);
                        // }
                    }    

                    // vTaskDelay(10 / portTICK_PERIOD_MS);
                }
                
            }
            

            /*
            Ở chế độ Input, với chu kì PERIOD_SEND_TO_SOCKET_TASK, ta sẽ gửi những thẻ đang ALive và chưa có thông tin tới Task Socket
            để lấy thông tin từ server.
            */
            if( xUHF_RFID_CenterInfor.socketStatus.status != CONNECTED) {
                xQueueReset(x_polling_UHF_RFID_2_Server_Queue);
            }
            uint8_t checkAliveCardCount = 0;
            static uint16_t isReSendToServer = 0;
            static uint32_t reSendToServerCheckTime = 0;
            if(xTaskGetTickCount() - reSendToServerCheckTime >= 2000 / portTICK_PERIOD_MS) {
                isReSendToServer = 1;
                reSendToServerCheckTime = xTaskGetTickCount();
            }
            for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                if(aliveCardArr[checkCount].isAlive == 1) {
                    checkAliveCardCount++;
                    if(aliveCardArr[checkCount].isHasInfor == 0 && aliveCardArr[checkCount].isSendToServer == 1 && isReSendToServer > 0) {
                        aliveCardArr[checkCount].isSendToServer = 0;
                    }
                }
            }

            // Neu reset xong thi reset flag
            isReSendToServer = 0;

            if(xQueueSendAliveWireCount != NULL) {
                xQueueOverwrite(xQueueSendAliveWireCount, &checkAliveCardCount);
            }

            if (x_polling_UHF_RFID_2_Server_Queue != NULL && xUHF_RFID_CenterInfor.socketStatus.status == CONNECTED)
            {
                uint32_t tempPeriodSendSocketTime = xTaskGetTickCount();
                if(tempPeriodSendSocketTime - checkPeriodSendToSocketTime >= PERIOD_SEND_TO_SOCKET_TASK / portTICK_PERIOD_MS) {
                    // MY_PRINT("Chuẩn bị gửi thẻ.\n");
                    CARD tempSendCard;
                    for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                        //Nếu thẻ đang ALive và chưa có thông tin thì gửi qua Task Socket //Code moi

                        #if 1// Bật cờ chỉ cho gửi 1 lần lên server
                        if(aliveCardArr[checkCount].isAlive == 1 && aliveCardArr[checkCount].isHasInfor == 0 && aliveCardArr[checkCount].isSendToServer == 0) 
                        { 
                            memset(&tempSendCard, 0, sizeof(CARD));
                            memcpy(tempSendCard.epc, aliveCardArr[checkCount].epc, aliveCardArr[checkCount].epc_len);
                            tempSendCard.epc_len = aliveCardArr[checkCount].epc_len;
                            tempSendCard.result_cards = 1;
    
                            
                            
                            // ESP_LOGI(TAG, "Send card: %s", tempSendCard.epc);
                            if (xQueueSend(x_polling_UHF_RFID_2_Server_Queue, &tempSendCard, 0 / portTICK_PERIOD_MS) == pdTRUE )
                            {   
                                // //Hàm hiển thị mảng Alive - Debug
                                // for(uint8_t lcd_count = 0; lcd_count < SIZEOF_CHECK_ALIVE_CARD_ARR; lcd_count++) {
                                //     ESP_LOGW(TAG, "\nCheck alive card: %s Flag alive: %d Flag hasInfor: %d\n", (char*)aliveCardArr[lcd_count].epc, aliveCardArr[lcd_count].isAlive, aliveCardArr[lcd_count].isHasInfor);
                                // }
                                ESP_LOGI(TAG, "Successed to post the EPC to task socket: %s.\n", (char*)tempSendCard.epc);

                                // Đánh dấu đã gửi thẻ lên server
                                aliveCardArr[checkCount].isSendToServer = 1;
                                
                            }
                            else
                            {
                                // MY_PRINT("Failed to post the EPC to task socket.\n");
                            }
                        }
                        #endif
                    }
                    //Cập nhật thời gian, đợi chu kì tiếp theo
                    checkPeriodSendToSocketTime = tempPeriodSendSocketTime;
                }
                
            }
        
        
            //Ở chế độ hoạt động khác, ta sẽ xóa thẻ nếu quá Timeout
            if(1){
                for(uint8_t checkCount = 0; checkCount < SIZEOF_CHECK_ALIVE_CARD_ARR; checkCount++) {
                    //Tìm kiếm vị trí trống trong mảng check a live
                    if(aliveCardArr[checkCount].isAlive == 1 && xTaskGetTickCount() - aliveCardArr[checkCount].lastAliveTime >= CHECK_ALIVE_CARD_TIMEOUT/ portTICK_PERIOD_MS) {
                        aliveCardArr[checkCount].isAlive = 0;       //Cờ báo trạng thái đang alive
                        aliveCardArr[checkCount].current_rssi = 0;  //Reset giá trị RSSI
                        ESP_LOGW(TAG, "\nClear card: %s\n", aliveCardArr[checkCount].epc);
                        // memset(aliveCardArr[checkCount].epc, 0, sizeof(aliveCardArr[checkCount].epc)); 
                    }
                }
            }
            #if 1
            if (1) { 
                if (xTaskGetTickCount() - checkTheBestRSSITime >= (CHECK_THE_BEST_RSSI_DELAY_TIME / portTICK_PERIOD_MS)) {
                    
                    // Tìm thẻ có thông tin và RSSI tốt nhất
                    uint8_t theBestRSSI_Index = SIZEOF_CHECK_ALIVE_CARD_ARR + 1;
                    int8_t theWorstRSSI_Value = -100;
            
                    // Reset tất cả theBestRSSIValue trước khi quét
                    for (uint8_t i = 0; i < SIZEOF_CHECK_ALIVE_CARD_ARR; i++) {
                        aliveCardArr[i].theBestRSSIValue = 0;
                    }
                    #if 1
                    for (uint8_t checkRSSI = 0; checkRSSI < SIZEOF_CHECK_ALIVE_CARD_ARR; checkRSSI++) {
                        if (aliveCardArr[checkRSSI].isAlive == 1 &&
                            aliveCardArr[checkRSSI].isHasInfor == 1 &&
                            aliveCardArr[checkRSSI].current_rssi != 0) {
                            
                            if (aliveCardArr[checkRSSI].current_rssi >= theWorstRSSI_Value) {
                                theWorstRSSI_Value = aliveCardArr[checkRSSI].current_rssi;
                                theBestRSSI_Index = checkRSSI;
                            }
                        }
                    }
                    #endif
            
                    // Nếu tìm thấy thẻ có RSSI tốt nhất, đánh dấu nó
                    if (theBestRSSI_Index != SIZEOF_CHECK_ALIVE_CARD_ARR + 1) {
                        aliveCardArr[theBestRSSI_Index].theBestRSSIValue = 1;
                    }
            
                    checkTheBestRSSITime = xTaskGetTickCount();
                }
            }
            
            #endif

            if(x_AliveCard_2_LCD_Task_Queue != NULL) {
                xQueueOverwrite(x_AliveCard_2_LCD_Task_Queue, aliveCardArr);
            }

            //Nếu có sự kiện Working thì cập nhật biến checkTime
            uint32_t currentTime = xTaskGetTickCount();
            if(currentTime - UHFRFIDDelayCheckTime >= TIMEOUT_WORKING_TIME / portTICK_PERIOD_MS) {
                UHFRFIDDelayTime = DELAY_WHEN_WORKING;
            }
            else {
                UHFRFIDDelayTime = DELAY_WHEN_IDLE_TIME; 
            }

            vTaskDelay(UHFRFIDDelayTime / portTICK_PERIOD_MS);

            #endif
        }
    }

}





//Mảng quản lý các thẻ phục vụ hàm lọc Input
checkCardInputMode_typedef checkCardInputArr[SIZEOF_CHECK_CARD_INPUT_ARR];
uint8_t checkCardInput_Index = 0;   //Chỉ số hiện tại của mảng checkCardInputArr AKA số lượng thẻ đang chứa trong mảng checkCardInputArr

/**
 * @brief Lọc thẻ đầu vào dựa trên cường độ tín hiệu (RSSI) và số lần đọc lặp lại.
 *
 * Hàm này dùng để quyết định xem một thẻ RFID đọc được có hợp lệ để xử lý tiếp hay không
 * bằng cách đánh giá các tiêu chí:
 * 
 * 1. Nếu RSSI của thẻ vượt ngưỡng cao (`SET_READ_THRESHOLD_2`) → chấp nhận ngay.
 * 2. Nếu không, kiểm tra lịch sử đọc thẻ trong mảng `checkCardInputArr`:
 *    - Nếu thẻ đã từng đọc: tăng số lần đếm (`repeatCount`) và cập nhật RSSI lớn nhất.
 *      - Nếu số lần đọc >= `SET_REPEAT_READ_CARD_THRESHOLD` và RSSI lớn nhất vượt `SET_READ_THRESHOLD_1` → chấp nhận.       
 *    - Nếu là thẻ mới: lưu vào mảng, bắt đầu theo dõi.
 * 3. Đồng thời, loại bỏ các thẻ đã hết thời gian theo dõi (`SET_READ_TIMEOUT`).
 *
 * @param checkCard Cấu trúc thông tin thẻ RFID vừa được đọc (bao gồm RSSI, EPC, v.v.).
 * 
 * @return UHF_RFID_OK nếu thẻ hợp lệ đủ điều kiện lọc.
 *         UHF_RFID_NG nếu thẻ chưa đủ điều kiện, tiếp tục chờ đọc thêm.
 */
static UHF_RFID_status filterInputModeFunc(CARD checkCard) {
    // Nếu RSSI đủ mạnh vượt ngưỡng 2 → chấp nhận luôn
    if(checkCard.rssi > SET_READ_THRESHOLD_2) {
        // ESP_LOGI(TAG, "The %s vuot nguon 2 => OK", checkCard.epc_str);
        return UHF_RFID_OK;
    }

    //Kiểm tra xem các thẻ trong mảng lọc Input đã quá Timeout hay chưa?
       // 1. Xóa các thẻ đã hết thời gian theo dõi (timeout)
    for(uint8_t i = 0; i < checkCardInput_Index; i++) {
        if(checkCardInputArr[i].repeatCount > 0 && xTaskGetTickCount() - checkCardInputArr[i].firstTimeReadThisCard >= SET_READ_TIMEOUT / portTICK_PERIOD_MS) {
            memset(&checkCardInputArr[i].card, 0, sizeof(CARD));
            checkCardInputArr[i].repeatCount = 0;
            checkCardInput_Index -- ;
            checkCardInputArr[i].maxOfRSSIReceived = -127;
        }
    }

    // 2. Kiểm tra xem thẻ có trong mảng chưa
    int16_t isAppearInArr = -1;
    for(uint8_t i = 0; i < checkCardInput_Index; i++) {
        //Nếu là thẻ đã được đọc và đến từ trước, tiến hành cập nhật số lần đếm cho thẻ
        if(checkCardInputArr[i].repeatCount > 0 && memcmp(checkCardInputArr[i].card.epc, checkCard.epc, checkCard.epc_len) == 0) {
             // Thẻ đã có → cập nhật thông tin
            checkCardInputArr[i].repeatCount ++;
            // ESP_LOGI(TAG, "The %s doc duoc %d lan", checkCard.epc_str, checkCardInputArr[i].repeatCount );
            isAppearInArr = i;

            //Kiểm tra giá trị RSSI hiện tại có phải lớn nhất không? Nếu lớn nhất tiến hành cập nhật giá trị
            if(checkCard.rssi > checkCardInputArr[i].maxOfRSSIReceived) {
                checkCardInputArr[i].maxOfRSSIReceived = checkCard.rssi;
            }
            //Nếu thẻ có số lần đọc lại đủ điều kiện và ngưỡng RSSI lớn nhất đạt qua ngưỡng 1 thì thông báo thẻ OK
            if(checkCardInputArr[i].repeatCount >= SET_REPEAT_READ_CARD_THRESHOLD && checkCardInputArr[i].maxOfRSSIReceived > SET_READ_THRESHOLD_1){
                // ESP_LOGI(TAG, "The %s vuot nguon 1 & dat du so lan doc lai => OK", checkCard.epc_str);
                return UHF_RFID_OK;
            }
            break;
        }
    }

    //Nếu là thẻ mới được đọc, tiến hành lưu vào mảng
    if(isAppearInArr == -1 && checkCardInput_Index <= SIZEOF_CHECK_CARD_INPUT_ARR) {
        for(uint8_t i = 0; i < SIZEOF_CHECK_CARD_INPUT_ARR; i++) {
            if(checkCardInputArr[i].repeatCount == 0) { //Nếu gặp vị trí trống thì điền vào
                memset(&checkCardInputArr[i].card , 0, sizeof(CARD));
                memcpy(&checkCardInputArr[i].card , &checkCard, sizeof(CARD));
                checkCardInputArr[i].repeatCount = 1;  //Tăng biến đếm
                checkCardInputArr[i].firstTimeReadThisCard = xTaskGetTickCount();
                checkCardInputArr[i].maxOfRSSIReceived = checkCard.rssi;
                //Tăng biến đếm số thẻ trong mảng
                checkCardInput_Index ++;
            }
        }
        
    }

    if(checkCardInput_Index > SIZEOF_CHECK_CARD_INPUT_ARR)
        ESP_LOGE(TAG, "So luong the vuot qua kich thuoc mang (array) quan ly");

    return UHF_RFID_NG;
}


//Tầng BSP
void UHF_RFID_UART_Init(void) {
    x_reading_UHF_RFID_2_Server_Queue = xQueueCreate( 10, sizeof(read_buffer) );
    x_polling_UHF_RFID_2_Server_Queue = xQueueCreate( 120, sizeof(CARD));
    x_Card_2_LCD_Task_Queue = xQueueCreate(10, sizeof(wire_infor));
    x_AliveCard_2_LCD_Task_Queue = xQueueCreate(1, SIZEOF_CHECK_ALIVE_CARD_ARR * sizeof(manage_Card_Alive_t));

    const uart_config_t uart_config = {
        .baud_rate = UHF_RFID_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    }; 
    // We won't use a buffer for sending data.
    uart_driver_install(UHF_RFID_UART, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UHF_RFID_UART, &uart_config);
    uart_set_pin(UHF_RFID_UART, TXD_UART_PIN, RXD_UART_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // UHF_RFID_Tx_Power = CONFIG_UHF_RFID_TX_POWER;
    UHF_RFID_Tx_Power = SET_UHF_RFID_TX_POWER;
    UHF_RFID_readerThreshold = -60;
    UHF_RFID_numberCardsPolling = 8;

    //Clear mảng lọc chế độ Input
    memset(checkCardInputArr, 0, sizeof(checkCardInputMode_typedef) * SIZEOF_CHECK_CARD_INPUT_ARR);
    memset(recv_cards, 0, sizeof(recv_cards));

    xTaskCreate(&UHF_RFID_task, "uhf_rfid_task", 1024*14, NULL, configMAX_PRIORITIES - 1, NULL);
}




//chờ và kiểm tra xem có gói tin hợp lệ đến từ UART hay không
uint8_t waitMsg(unsigned long timerout) {
    //uint8_t count           = 0;
    cleanBuffer();
    //Đọc dữ liệu UART từ thiết bị RFID vào buffer, Đọc tối đa RX_BUF_SIZE byte trong thời gian tối đa 40ms.
    uart_read_bytes(UHF_RFID_UART, buffer, RX_BUF_SIZE, 40 / portTICK_PERIOD_MS);   
                          
    for(int i = 0; i < RX_BUF_SIZE - 25; i++) {
        //Buffer hợp lệ: Byte đầu là BB và byte thứ 3 ko đc là FF
        if(buffer[i] == 0xBB &&  buffer[i + 2] != 0xFF) { //buffer[count] == 0x7E &&
            
            return true;           
        }
    }
 
    return false;
}

/*! @brief Hàm xử lý thô dữ liệu nhận về khi poll các thẻ Tag.
    Giá trị trả về là số thẻ nhận được, nếu không nhận được thẻ nào, trả về 0
    */
uint8_t waitMsgWhenPolling(uint16_t *startIndex, uint16_t *receivedBytes) {
    cleanBuffer();
    //Số byte đọc được
    *receivedBytes = uart_read_bytes(UHF_RFID_UART, buffer, RX_BUF_SIZE, 20 / portTICK_PERIOD_MS); 
    // printf("LOG received buffer data:  ");
    // for(int i = 0; i < *receivedBytes; i++) {
    //     printf("%X  ", buffer[i]);
    // }
    // printf("\n");
    uint8_t tagCount = 0;   //Biến đếm các thẻ nhận được
    uint16_t loopArrCount = (*receivedBytes >= (RX_BUF_SIZE - 24)) ? (RX_BUF_SIZE - 24) : *receivedBytes; //Nếu *receivedBytes quá lớn, chỉ duyệt đến RX_BUF_SIZE - 24.
    for(int i = 0; i < loopArrCount; i++) {
        if(buffer[i] == 0xBB && buffer[i + 1] == 0x02) { 
            // Lấy vị trí đầu tiên
            if(tagCount == 0) {
                *startIndex = i;
            }
            //Ghép byte cao và byte thấp để lấy độ dài dữ liệu
            uint16_t dataLength = ((uint16_t)buffer[i + 3] << 8) | buffer[i + 4];
            //Di chuyển đến đầu frame tiếp theo
            i = i + dataLength + 6; //Do vòng for tự cộng 1, nên +6 để bỏ qua toàn bộ frame
        
            tagCount++;  //tăng biến đếm thẻ RFID        
        }
        //Nếu nhận được frame báo lỗi (không đọc được) chứng tỏ không còn thẻ nào khác
        // if(buffer[i] == 0xBB && buffer[i + 1] == 0x01 && buffer[i + 2] == 0xFF) {
        //     return tagCount;
        // }
    }
    // printf("wait_MsgWhenPolling: Check tag count:%d\n", tagCount);
    return tagCount;
}


/*! @brief Send command.*/
void sendCMD(uint8_t *data, size_t size) {
    // const int len = strlen(data);

    uart_write_bytes(UHF_RFID_UART, data, size);
}

/*! @brief Clear the buffer.*/
//Reset mảng buffer[] về 0, dùng để chứa dữ liệu UART vừa nhận được từ RFID module.
void cleanBuffer() {
    for (int i = 0; i < RX_BUF_SIZE; i++) {
        buffer[i] = 0;
    }
}

/*! @brief Clear the card's data buffer.*/
//Xóa toàn bộ thông tin cũ của các thẻ RFID đã lưu trong mảng cards[] trước khi bắt đầu đọc lô mới.
void cleanCardsBuffer() {
    for (int i = 0; i < SAVE_CARDS_NUMBER; i++) {
        cards[i].rssi = 0;
        for(int j = 0; j < 2; j++) {
            cards[i].pc[j] = 0;
        }
        for(int j = 0; j < MAX_EPC_LENGTH; j++) {
            cards[i].epc[j] = 0;
        }
        memset(cards[i].rssi_str, 0, 50);
        memset(cards[i].pc_str, 0, 50);
        memset(cards[i].epc_str, 0, 50);
    }
}

/*! @brief Clear the card's before send to server data buffer.*/
//đảm bảo vùng nhớ chứa dữ liệu gửi đi là mới hoàn toàn, tránh lỗi gửi dữ liệu cũ.
void cleanBeforeSendCardsBuffer() {
    memset(recv_cards, 0x00, sizeof(recv_cards));
}


/*! @brief Save the card information.(lưu thông tin thẻ vào 1 struct card)
    @param card: The card storage datas.
    @param number: The location of the data in the buffer.    
*/
uint8_t saveCardInfo(CARD *card, uint8_t number) {

    char rssi[3] = {0};
    char pc[5] = {0};
    char epc[MAX_EPC_LENGTH] = {0};
    uint8_t X = 1; //Old = 24
    uint8_t countEpc = 0;


    //Đọc chiều dài phần dữ liệu Data Length từ frame (chuẩn giao tiếp đầu đọc UHF).
    uint16_t dataLength = ((uint16_t)buffer[number + 3] << 8) | buffer[number + 4];
    //Tại đây, number là vị trí bắt đầu frame, number + dataLength + 6 là vị trí cuối frame

    //Đọc giá trị RSSI từ frame, rồi chuyển nó sang chuỗi để hiển thị 
    for (uint8_t j = 0; j < 2; j++) {
        rssi[j] = hex2str(buffer[5 + X*number])[j];
    }
    rssi[2] = '\0';

    //Lưu giá trị pc
    // for (uint8_t j = 0; j < 2; j++) {
    //     pc[j] = hex2str(buffer[6 + X*number])[j];
    // }
    // for (uint8_t j = 2; j < 4; j++) {
    //     pc[j] = hex2str(buffer[7 + X*number])[j-2];
    // }
    // pc[4] = '\0';

    //Kiểm tra độ dài của EPC (dataLength = PC(2) + EPC(n) + RSSI(1) + CRC(2) → EPC = dataLength - 5.)
    if(dataLength - 5 >= MAX_EPC_LENGTH - 1) {
        MY_PRINT("Failed to save card infor because of EPC Length.\n");
        return false;
    }

    //Lưu giá trị chuỗi epc
    countEpc = 0;
    for (uint8_t j = number + 8; j < number + 8 + dataLength - 5 ; j++) {
        epc[countEpc] = (buffer[j]);
        countEpc++;  
    }
    epc[countEpc] = '\0';

    int len_CountEpc = strlen(epc);
    if(len_CountEpc < 0) {
        return false;
    }
    else {
        countEpc = len_CountEpc;
    }

    // MY_PRINT("Epc string: %s\n", epc);

    //Lọc giá trị trùng lặp những thẻ đã lọc bởi server
    // if (!filterCardInfo(epc)) {
    //     // MY_PRINT("Failed to save card info1.\n");
    //     return false;
    // }

    //Lọc giá trị trùng lặp trước khi gửi
    if (!filterBeforeSendCardInfo(epc)) {
        // MY_PRINT("Failed to save card info2.\n");
        return false;
    }
    //Lưu giá trị các thành phần vào mảng card (struct CARD)
    card->rssi  = (int8_t)buffer[5 + X*number];
    // int8_t CheckRSSI = card->rssi;
    // MY_PRINT("\nCheck RSSI: %d vs Threshold: %d\n", CheckRSSI, (int8_t)UHF_RFID_readerThreshold);
    // MY_PRINT("\nCheck RSSI: %d \n", card->rssi);
    //Nếu sóng nhận được yếu hơn ngưỡng quy định thì bỏ qua thẻ này.
    if(card->rssi < UHF_RFID_readerThreshold) {
        // MY_PRINT("Failed to save card info3.\n");
        return false;
    }            

    //Lưu giá trị chuỗi epc vào mảng Cards
    strcpy(card->epc_str, epc);


    

    //Lưu giá trị chiều dài Epc
    card->epc_len = countEpc;

    for (uint8_t j = 0; j < MAX_EPC_LENGTH; j++) {
        card->epc[j] = epc[j];
    }

    // MY_PRINT("\n\nSave Card Info: EPC value  \n");
    // for(uint8_t a = 0; a <= MAX_EPC_LENGTH ; a++) {
    //     MY_PRINT("%X       ", epc[a]);
    // }
    // MY_PRINT("\n");



    // card->rssi  = (int8_t)buffer[5 + X*number];
    card->pc[0] = buffer[6 + X*number];
    card->pc[1] = buffer[7 + X*number];

    strcpy(card->rssi_str, rssi);
    strcpy(card->pc_str, pc);
    
    if (_debug) {
        MY_PRINT("pc: %s\n", pc);
        MY_PRINT("rssi: %s\n", rssi);
        MY_PRINT("epc: %s\n", epc);
        for (uint8_t i = 0; i < 24; i++) {
            MY_PRINT("%s", hex2str(buffer[i + X*number]));
        }
        MY_PRINT("\n");
    }
    //MY_PRINT("Successfully save card info.\n");
    // MY_PRINT("\n \n Save Card Info: EPC string: %s \n \n",card->epc_str);
    return true;
}


/**
 * @brief Lọc thẻ RFID vừa nhận.
 * 
 * Hàm kiểm tra chuỗi EPC đã tồn tại trong danh sách thẻ đã lưu hay chưa.
 * Nếu thẻ đã tồn tại (trùng), trả về false. Nếu là thẻ mới, trả về true.
 *
 * @param epc Con trỏ tới chuỗi EPC của thẻ RFID vừa nhận.
 * @return true  Nếu là thẻ mới (không trùng).
 * @return false Nếu là thẻ trùng (đã lưu trước đó).
 */

uint8_t filterCardInfo(char* epc) {
    for (int i = 0; i < SAVE_CARDS_NUMBER; i++) {

        if (strcmp(epc, cards[i].epc_str) == 0) {


            return false;
        }
    }
    return true;
}


/*! @brief Filter the received message before send to server.
     When CardInfo like a card in buffer card, return false. Else return true.
*/

/*!
 * @brief Kiểm tra xem EPC có tồn tại trong danh sách các thẻ đã nhận (`recv_cards`) hay không.
 *
 * Hàm sẽ duyệt qua mảng `recv_cards` và so sánh EPC đầu vào với từng thẻ đã lưu,
 * dựa trên chiều dài `epc_len` tương ứng của mỗi thẻ. Nếu tìm thấy thẻ trùng khớp,
 * tức là đã được xử lý trước đó, hàm sẽ trả về `false`. Ngược lại, nếu EPC là mới
 * (chưa tồn tại trong danh sách), hàm trả về `true`.
 *
 * @param epc Con trỏ đến chuỗi EPC cần kiểm tra.
 * @return `true` nếu EPC chưa có trong danh sách `recv_cards`, `false` nếu đã tồn tại.
 */
uint8_t filterBeforeSendCardInfo(char* epc) {
    // MY_PRINT("EPC First: %s\n", epc);
    for (int i = 0; i < SAVE_CARDS_NUMBER; i++) {
 
        if (strncmp(epc, (char*)recv_cards[i].epc, recv_cards[i].epc_len) == 0 && recv_cards[i].epc_len > 0) {


            return false;
        }
    }
    return true;
}


/*!
 * @brief Gửi lệnh yêu cầu phiên bản phần cứng và trích xuất chuỗi phiên bản từ buffer phản hồi.
 *
 * Hàm gửi lệnh `HARDWARE_VERSION_CMD` tới thiết bị và chờ phản hồi trong 500ms.
 * Nếu nhận được phản hồi hợp lệ, nó sẽ đọc chuỗi phiên bản từ `buffer` bắt đầu từ vị trí thứ 6.
 * Chuỗi được kết thúc khi gặp ký tự 0x7E (được hiểu là ký tự kết thúc frame) và trả về chuỗi kết quả.
 *
 * Nếu không nhận được phản hồi hợp lệ, hàm sẽ in thông báo lỗi cùng dữ liệu trong `buffer`
 * (30 byte đầu), và trả về chuỗi "ERROR".
 *
 * @note Biến `infoGetVerion` được khai báo là `static` để có thể trả về con trỏ đến vùng nhớ
 * cục bộ sau khi hàm kết thúc.
 *
 * @return Con trỏ tới chuỗi chứa phiên bản phần cứng nếu thành công, hoặc chuỗi `"ERROR"` nếu thất bại.
 */
char* getVersion(void) {
    sendCMD((uint8_t *)HARDWARE_VERSION_CMD, sizeof(HARDWARE_VERSION_CMD));

    if (waitMsg(500)) {
        static char infoGetVerion[1024];

        for (uint8_t i = 0; i < 50; i++) {
            infoGetVerion[i] = (char)buffer[6 + i];
            if (buffer[8 + i] == 0x7e) {
                infoGetVerion[i + 1] = '\0';
                break;
            }
        }
        return infoGetVerion;
    } else {
        MY_PRINT("\n Failed to read version, check buffer data:   ");
        for(uint8_t i = 0; i <30; i++) {
            MY_PRINT("    %X", buffer[i]);
        }
        MY_PRINT("\n");
        return "ERROR";
    }
}


/*!
 * @brief Gửi lệnh "polling multiple" để quét nhiều thẻ RFID cùng lúc và lưu thông tin vào buffer thẻ nhận.
 *
 * Hàm này gửi một lệnh polling tới module RFID để quét nhiều thẻ (tags) trong vùng phủ sóng.
 * Sau khi gửi lệnh, hàm chờ phản hồi, phân tích dữ liệu nhận được để xác định số lượng thẻ
 * và kiểm tra các frame dữ liệu hợp lệ theo giao thức. Mỗi frame hợp lệ sẽ được xử lý và
 * thông tin thẻ sẽ được lưu vào mảng `recv_cards`.
 *
 * @param polling_count Số lần polling (chu kỳ lặp lệnh đọc) được yêu cầu.
 * 
 * @return Số lượng thẻ đọc được thành công trong phiên polling này.
 */

uint8_t pollingMultiple(uint16_t polling_count) {
    //cleanCardsBuffer();
    cleanBeforeSendCardsBuffer();
    memcpy(Txbuffer, POLLING_MULTIPLE_CMD, sizeof(POLLING_MULTIPLE_CMD));
    Txbuffer[6] = (polling_count >> 8) & 0xff;
    Txbuffer[7] = (polling_count)&0xff;

    uint8_t check = 0;
    for (uint8_t i = 1; i < 8; i++) {
        check += Txbuffer[i];
    }

    Txbuffer[8] = check & 0xff;
    if (_debug) {
        printf("Send cmd: \n");
        for (uint8_t i = 0; i < sizeof(POLLING_MULTIPLE_CMD); i++) {
            printf("%s", hex2str(Txbuffer[i]));
        }
        printf("\n");
    }
    sendCMD(Txbuffer, sizeof(POLLING_MULTIPLE_CMD));

    // MulPoll_count = 0;  
    uint16_t result_count = 0;

    uint16_t receivedBytesCount = 0;
    uint16_t startFrameIndex = 0;

    //printf("Wait for polling mulltiple!\n");
    uint8_t numOfTagCOunt = waitMsgWhenPolling(&startFrameIndex, &receivedBytesCount);

    // ESP_LOGW(TAG, "Nhan duoc %d the", numOfTagCOunt);
    //Nếu không nhận được thẻ nào thì return hàm ngay lật tức
    if (numOfTagCOunt == 0) {
        return 0;               //Trả về không đọc được thẻ nào
    }

    // uint8_t temmpCount = 0;
    for(int i = startFrameIndex; i < RX_BUF_SIZE - 24; i++) {  //Trừ đi 24 (Tùy chọn)
        // ESP_LOGW(TAG, "%d", i);  
        if(buffer[i] == 0xBB && buffer[i + 1] == 0x02 && buffer[i + 2] == 0x22) {// && (buffer[i+23] == 0x7E || buffer[i+19 == 0x7E])) {
            uint16_t dataLength = ((uint16_t)buffer[i + 3] << 8) | buffer[i + 4];
            if(buffer[i + dataLength + 6] == 0x7E) {
                //Tại đây, i là vị trí bắt đầu frame, i + dataLength + 6 là vị trí cuối frame
                //Tính checksum
                uint8_t check = 0;
                uint8_t check_checksum = 0;

                for (uint8_t k = 1; k < dataLength + 5; k++) {
                    check += buffer[i+k];
                }
                check_checksum = check & 0xff;
                // printf("\nUHF-RFID Checksum = %X\n\n", check_checksum);
                // for(int a = i; a <= i + dataLength + 6; a++) {
                //     printf("    %X", buffer[a]);
                // }
                // printf("\n");
                if(buffer[i+dataLength + 5] == check_checksum) {
                    if (saveCardInfo(&recv_cards[MulPoll_recv_count], i) == 1) {
                        // printf("Save the card had the index: %d EPC: %s\n", MulPoll_recv_count,  recv_cards[MulPoll_recv_count].epc_str);
                        // printf("recv_cards: %s\n", recv_cards[MulPoll_recv_count].epc_str);
                        // printf("\n");                        
                        MulPoll_recv_count = (MulPoll_recv_count + 1) % SAVE_CARDS_NUMBER;  
                        result_count++;
                        // vTaskDelay(10 / portTICK_PERIOD_MS);
                    }
                }
                //Di chuyển đền vị trí frame tiếp theo
                i = i + dataLength + 5; //Chỉ cộng 6 do vòng for tự cộng 1
                // vTaskDelay(10 / portTICK_PERIOD_MS);
            }           
        }
        //Nếu đã lấy đủ số lượng thẻ Tag hoặc quét đủ số bytes nhận được từ phiên này
        if(numOfTagCOunt == result_count || i > (receivedBytesCount - 13)) {  //Trừ đi chiều dài frame tối thiểu khi EPC là 1 byte
            break;
        }
        // temmpCount ++;  
    }
    // printf("Check count debug: %d!\n", temmpCount);
    // printf("Check bytes debug: %d!\n", receivedBytesCount);

    //printf("Polling Mul: count = %d\n", MulPoll_count);
    return result_count;
}


// MAX 1500 => 15dB
// MAX 2600 => 26dB
/**
 * @brief   Thiết lập công suất phát cho thiết bị.
 * @param   db  Giá trị công suất phát đầu vào (đơn vị: dBm), ví dụ: 26 nghĩa là 26 dBm.
 * @return  true nếu thiết lập thành công, false nếu thất bại.
 */
uint8_t setTxPower(uint16_t db) {
    db = db * 100;
    memcpy(Txbuffer, SET_TX_POWER, sizeof(SET_TX_POWER));
    Txbuffer[5] = (db >> 8) & 0xff;
    Txbuffer[6] = db & 0xff;

    uint8_t check = 0;

    for (uint8_t i = 1; i < 7; i++) {
        check += Txbuffer[i];
    }
    Txbuffer[7] = check & 0xff;
    sendCMD(Txbuffer, sizeof(SET_TX_POWER));
    if (waitMsg(500)) {
        if (buffer[2] != 0xB6) {
            return false;
        }
        return true;
    } else {
        return false;
    }
}

/**
 * @brief   Gửi lệnh dừng chế độ đọc nhiều thẻ (multiple polling).
 * @return  true nếu dừng thành công, false nếu thất bại hoặc không nhận được phản hồi.
 */
uint8_t stopMulPolling(void) {
    memcpy(Txbuffer, STOP_MULTIPLE_POLLING_CMD, sizeof(STOP_MULTIPLE_POLLING_CMD));
    sendCMD(Txbuffer, sizeof(STOP_MULTIPLE_POLLING_CMD));
    if (waitMsg(500)) {
        if (buffer[2] != 0x28) {
            return false;
        }
        return true;
    } else {
        return false;
    }
}

/**
 * @brief   Kích hoạt chế độ Dense Reader Mode (DRM) cho thiết bị đọc RFID.
 *
 * --- Dense Reader Mode (DRM) ---
 * Mục tiêu              : Giảm nhiễu trong môi trường có nhiều đầu đọc RFID hoạt động đồng thời.
 * Khoảng cách đọc       : Thường ngắn hơn chế độ thông thường.
 * Tốc độ đọc            : Chậm hơn.
 * Khả năng chống nhiễu  : Cao hơn.
 * Ứng dụng              : Môi trường nhà máy, kho bãi, hoặc nơi nhiều đầu đọc RFID hoạt động gần nhau.
 *
 * @return  true nếu cấu hình chế độ DRM thành công, false nếu thất bại hoặc không có phản hồi.
 */
uint8_t selectDenseReaderMode(void) {
    memcpy(Txbuffer, SELECT_DENSE_READER_MODE_CMD, sizeof(SELECT_DENSE_READER_MODE_CMD));
    sendCMD(Txbuffer, sizeof(SELECT_DENSE_READER_MODE_CMD));
    if (waitMsg(500)) {
        if (buffer[2] == 0xFF) {
            return false;
        }
        return true;
    } else {
        return false;
    }
}

/**
 * @brief   Kích hoạt chế độ High Sensitivity Mode (HSM) cho thiết bị đọc RFID.
 *
 * --- High Sensitivity Mode (HSM) ---
 * Mục tiêu              : Tăng khoảng cách đọc và độ nhạy tín hiệu của đầu đọc RFID.
 * Khoảng cách đọc       : Lớn hơn so với chế độ thông thường.
 * Tốc độ đọc            : Nhanh hơn.
 * Khả năng chống nhiễu  : Thấp hơn (vì độ nhạy cao dễ bị nhiễu).
 * Ứng dụng              : Các tình huống cần đọc thẻ từ xa, hoặc nơi tín hiệu yếu.
 *
 * @return  true nếu thiết lập thành công, false nếu thất bại hoặc không nhận được phản hồi.
 */
uint8_t selectHighSensitivityMode(void) {
    memcpy(Txbuffer, SELECT_HIGH_SENSITIVITY_MODE_CMD, sizeof(SELECT_HIGH_SENSITIVITY_MODE_CMD));
    sendCMD(Txbuffer, sizeof(SELECT_HIGH_SENSITIVITY_MODE_CMD));
    if (waitMsg(500)) {
        if (buffer[2] == 0xFF) {
            return false;
        }
        return true;
    } else {
        return false;
    }
}

/**
 * @brief   Đọc giá trị công suất phát (Tx Power) hiện tại của thiết bị.
 *
 * Lệnh GET_TX_POWER gửi yêu cầu đến thiết bị, và kết quả trả về là 2 byte công suất ở vị trí xác định trong buffer.
 * Giá trị trả về có đơn vị là 0.01 dBm (ví dụ: 2650 tương ứng với 26.50 dBm).
 *
 * @return  Giá trị công suất phát (0.01 dBm). Trả về 0 nếu không nhận được phản hồi hợp lệ.
 */
uint16_t getTxPower(void) {
    memcpy(Txbuffer, GET_TX_POWER, sizeof(GET_TX_POWER));
    sendCMD(Txbuffer, sizeof(GET_TX_POWER));
    if (waitMsg(500)) {
        for(uint16_t i = 0; i < RX_BUF_SIZE - 25; i++ ) {
            if(buffer[i] == 0xBB && buffer[i+1] == 0x01 && buffer[i+2]== 0xB7 && buffer[i+8] == 0x7E) {
                uint16_t txValue = ((uint16_t)buffer[i+5] << 8) | buffer[i+6];
                return txValue;
            }
        }
        return false;
    } else {
        return false;
    }    
}


// Hàm tính CRC-16/GENIBUS
uint16_t crc16_genibus(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF; // Giá trị khởi đầu của CRC

    for (size_t i = 0; i < length; i++) {
        uint8_t table_index = (crc >> 8) ^ data[i];
        crc = (crc << 8) ^ crc16_genibus_table[table_index];
    }

    return crc ^ 0xFFFF; // XOR với 0xFFFF
}

