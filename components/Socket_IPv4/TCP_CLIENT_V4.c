/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "TCP_CLIENT_V4.h"

static const char *TAG = "TCP-IP Client";
#define TIME_TO_SEND    20   // ms
#define TIME_TO_SEND_LOCATION_TAG 5000 // ms
#define TIME_TO_WAIT_RESPONE 2500
#define MAX_EPC_BATCH 5  // Số lượng mã EPC tối đa trong một cụm gửi

// static const char *payload = "Message from ESP32 ";

#ifdef TEST_UHF_RFID_WATCH
extern QueueHandle_t x_polling_UHF_RFID_2_Server_Queue;
extern QueueHandle_t x_Card_2_LCD_Task_Queue;      //Gửi dữ liệu serial lên màn hình LCD

QueueHandle_t x_GPIO_UHF_RFID_2_Server_Queue = NULL;                   //Tạm thời không dùng
QueueHandle_t x_Server_2_UHF_RFID_Task = NULL;                         ////Queue gửi thông tin thẻ trở lại UHF_RFID_task giúp tránh đọc lặp lại
QueueHandle_t x_Socket_2_LCD_Task_Queue = NULL;                        //Queue gửi trạng thái đã/chưa kết nối tới Socket server
QueueHandle_t xQueueLocation = NULL; // Khai báo handle cho hàng đợi
QueueHandle_t xQueueDeleteWireRegisterLocation = NULL; // Khai báo handle cho hàng đợi

// char rx_buffer[128];
char host_ip[18];
uint16_t portValue;
int addr_family = 0;
int ip_protocol = 0;
struct sockaddr_in dest_addr;

struct timeval timeout;

int sock;
uint8_t isConnectedToServer = 0;
sockettcp_info socketStatus;

CARD socket_cards;
uint8_t socket_read_buffer[SOCKET_READ_CARD_BUFFER_LENGTH] = {0};
wire_infor wires[MAX_ENTRIES];

socket_EPC_CARD checkUnDuplicateCardToServer[SOCKET_CHECK_DUPLICATE_SIZE];
uint8_t checkUnDuplicateCardToServer_Count = 0;

center_param_typedef xTCP_CenterParam;
/*********************************************************************** */


/*********************************************************************** */

static int buf_cat(char* buf, int max_len, char * str) {
    if(strlen(buf) + strlen(str) < max_len) {
        strcat(buf, str);
    }
    return strlen(buf);
}

// int MaxChamPhay;
int findSemicolonPosition(const char *str, const char ch) {

	int len = strlen(str);
	for(int i = 0; i < len; i++)
	{
	    if(str[i] == ch)
	    {
	        return i;
	    }
	}
	return -1; // Na:?u khC4ng tC,m tha:%y kC= ta;1 ';', tra:# va; -1
}

//Hàm xử lý chuỗi cho dữ liệu hiển thị màu dây
static void splitString(const char *input, ColorPairs *result) {
    result->color_count = 0; // Đặt lại color_count

    // Tạo bản sao của chuỗi đầu vào để không thay đổi chuỗi gốc
    char buffer[30];
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *saveptr1;
    char *pair_token = strtok_r(buffer, "+", &saveptr1);
    
    while (pair_token != NULL && result->color_count < 5) { // Tối đa 5 cặp màu
        char *saveptr2;
        char *main = strtok_r(pair_token, "-", &saveptr2);
        char *sub = strtok_r(NULL, "-", &saveptr2);

        if (main != NULL) {
            strncpy(result->color[result->color_count].main_color, main, 5);
            result->color[result->color_count].main_color[5] = '\0';

            if (sub != NULL) {
                strncpy(result->color[result->color_count].sub_color, sub, 5);
                result->color[result->color_count].sub_color[5] = '\0';
            } else {
                /*
                // Nếu có màu chính nhưng không có màu phụ, đặt sub là "None"
                strncpy(result->color[result->color_count].sub_color, "None", 5);
                result->color[result->color_count].sub_color[5] = '\0';
                */
                // Nếu có màu chính nhưng không có màu phụ, đặt sub theo màu chính
                strncpy(result->color[result->color_count].sub_color, result->color[result->color_count].main_color, 5);
                result->color[result->color_count].sub_color[5] = '\0';
            }
            result->color_count++;
            //Dữ liệu màu từ server gửi về được coi là chính thức
            result->color->colorStatus = OFFICIAL_WIRE_COLOR;
        }

        // Lấy cặp tiếp theo
        pair_token = strtok_r(NULL, "+", &saveptr1);
    }
}
#if 0 //Ham cu
void CopyBuffToStruct (wire_infor *wires, char Buffer[MAX_FIELDS][MAX_LENGTH]) {
        char temp_color[30] = {0};

        memset(wires,'\0',sizeof(wire_infor));

        strcpy(wires->status_card, Buffer[0]);
        strcpy(wires->serial, Buffer[1]);
        strcpy(wires->location, Buffer[2]);

        strcpy(temp_color, Buffer[3]);
        splitString(temp_color, &wires->colors);

        strcpy(wires->length, Buffer[4]);
        strcpy(wires->line, Buffer[5]);
        strcpy(wires->date, Buffer[6]);
        strcpy(wires->lo, Buffer[7]);
        strcpy(wires->group, Buffer[8]);

        strcpy(wires->SA, Buffer[9]);
        strcpy(wires->PartNumber, Buffer[10]);

        // printf("\nCHECK LOG1: %s\n", Buffer[8]);
        // printf("\nCHECK LOG1: %s\n", wires->group);

}
#endif

#if 1 //Ham moi
void CopyBuffToStruct (wire_infor *wires, char Buffer[MAX_FIELDS][MAX_LENGTH]) {
        char temp_color[30] = {0};

        memset(wires,'\0',sizeof(wire_infor));

        //Cấu trúc phản hồi cũ trước ngày 28.03.2025
        // strcpy(wires->status_card, Buffer[0]);
        // strcpy(wires->serial, Buffer[1]);
        // strcpy(wires->line, Buffer[2]);
        // strcpy(wires->PartNumber, Buffer[3]);
        // strcpy(wires->WireType, Buffer[4]);
        // strcpy(wires->WireSize, Buffer[5]);
        // strcpy(temp_color, Buffer[6]);
        // splitString(temp_color, &wires->colors);
        // strcpy(wires->length, Buffer[7]);
        // strcpy(wires->group, Buffer[8]);
        // strcpy(wires->WO, Buffer[9]);
        // strcpy(wires->Lot, Buffer[10]);
        // strcpy(wires->location, Buffer[11]);
        // strcpy(wires->factory, Buffer[12]);
        // strcpy(wires->method, Buffer[13]);
        // strcpy(wires->CartNo, Buffer[14]);
        // strcpy(wires->comment, Buffer[15]);

        // //Cấu trúc phản hồi mới từ ngày 28.03.2025
        // strcpy(wires->status_card, Buffer[0]);
        // strcpy(wires->serial, Buffer[1]);
        // strcpy(wires->line, Buffer[2]);
        // strcpy(wires->PartNumber, Buffer[3]);
        // strcpy(wires->WireType, Buffer[4]);
        // strcpy(wires->WireSize, Buffer[5]);
        // strcpy(temp_color, Buffer[6]);
        // splitString(temp_color, &wires->colors);
        // strcpy(wires->length, Buffer[7]);
        // strcpy(wires->group, Buffer[8]);
        // strcpy(wires->WO, Buffer[9]);
        // strcpy(wires->lot, Buffer[10]);
        // strcpy(wires->location, Buffer[11]);
        // strcpy(wires->factory, Buffer[12]);
        // strcpy(wires->method, Buffer[13]);
        // strcpy(wires->CartNo, Buffer[14]);
        // strcpy(wires->comment, Buffer[15]);

        //Cấu trúc phản hồi bên nhà máy B
        strcpy(wires->status_card, Buffer[0]); //0
        strcpy(wires->serial, Buffer[1]);//1
        strcpy(temp_color, Buffer[2]);//2
        splitString(temp_color, &wires->colors);
        strcpy(wires->group, Buffer[3]);//3
        strcpy(wires->Sa, Buffer[4]);//4
        strcpy(wires->wire_type, Buffer[5]);//5
        strcpy(wires->comment, Buffer[6]);//5

        // printf("\nCHECK LOG1: %s\n", Buffer[8]);
        // printf("\nCHECK LOG1: %s\n", wires->group);

}
#endif
char Buffer[MAX_ENTRIES][MAX_FIELDS][MAX_LENGTH] = {}; 
int TachData(char DataFromServer[]) {
	// char DataFromServer[] = "[OK]45E00815,LA99,BR,601,TRFC,15,1,2;[OK]45E03088,LA99,SB,3359,TRFC,15,1,304;[OK]45E00891,LA99,G -1,3969,TRFC,15,1,198;[OK]45E00892,LA99,BR,3969,TRFC,15,1,198;[OK]45E00893,LA99,G,3969,TRFC,15,1,198;[OK]45E03086,LA99,L,2374,TRFC,15,1,269;[OK]45E03087,LA99,GR,3359,TRFC,15,1,304;[OK]45E03373,LA99,LG,2000,TRFC,15,1,17;[OK]45E00894,LA99,L,3969,TRFC,15,1,198;[OK]45I1F292,LA01,,1507,589D,20,1,7;";
    // [OK],4B41E012,LA03,W -B,1605,TX33,05,1,1,212,486D-TXF3      Z 0008, ,08/01/2025,14:27:37;
    int MaxChamPhay=0;
    memset(Buffer, 0, sizeof(Buffer));
	int i = 0;
	
	char str[200]= {0};
	int prev_index = 0;
	while(1) {
		int offset_index =findSemicolonPosition(&DataFromServer[prev_index], ';');
		
		if (offset_index != -1) {
			printf("Offset index: %d\n", offset_index);
			memset(str,0,sizeof(str));
			memcpy(str,&DataFromServer[prev_index], offset_index);
			prev_index += offset_index + 1;

			char *field;
			
            char *statusEnd = strchr(str, ']');
            if (statusEnd != NULL) {
                statusEnd++;
            } else {
                // Xử lý lỗi
                ESP_LOGE(TAG, "Handle received frame failed!");
            }

			
			strncpy(Buffer[i][0], str, 4);
			
			int j = 1;
			field = strtok(statusEnd, ",");
			while (field != NULL && j < MAX_FIELDS) {
                int len_field = strlen(field);
				if (len_field < MAX_LENGTH && len_field > 0) {
                    strncpy(Buffer[i][j], field, MAX_LENGTH - 1);
                    Buffer[i][j][MAX_LENGTH - 1] = '\0';
                } else {
                    // Xử lý lỗi
                    ESP_LOGE(TAG, "Handle received frame failed - 1!");
                }
				field = strtok(NULL, ",");
				j++;
                // vTaskDelay(10 / portTICK_PERIOD_MS);
			}
			
			i++;
			
		} else {
			printf("Dont contain ';'.\n");
			break; 
		}
	}


	MaxChamPhay= i;

    for(int index=0;index<MaxChamPhay;index++) {
        CopyBuffToStruct(&wires[index],Buffer[index]);
    }

// printf("Giá trị Buffer[0][1]: %s\n", Buffer[2][0]);
        for (int index = 0; index < MaxChamPhay; index++) {
        printf("Wires Info %d:\n", index + 1);
        printf("  Status Card: %s\n", wires[index].status_card);
        printf("  Serial: %s\n", wires[index].serial);
        printf("Number of color pairs: %d\n", wires[index].colors.color_count);
        for (int i = 0; i < wires[index].colors.color_count; i++) {
            printf("Main color %d: %s, Sub color %d: %s\n", i + 1, wires[index].colors.color[i].main_color, i + 1, wires[index].colors.color[i].sub_color);
        }
        printf("  Group: %s\n", wires[index].group);
        printf("  Sa: %s\n", wires[index].Sa);
        printf("  Comment: %s\n, ", wires[index].comment);

        printf("\n");
	    
    }
       return MaxChamPhay;
}

// Hàm xử lý từng chuỗi con bắt đầu sau "[OK],"
int extract_LA_code_from_segment(char* segment, char* result, uint8_t length) {
    char *token;
    int count = 0;

    token = strtok(segment, ",");   

    while (token != NULL) {
        count++;

        if (count == 11) { // Phần tử thứ 12 tính từ sau [OK]
            if (token[0] == 'L' && (token[1] >= 'A' && token[1] <= 'Z')&& strlen(token) > 2) {
                strncpy(result, token, length);
                result[length] = '\0';
                return 1;
            }
        }

        token = strtok(NULL, ",");
    }

    return 0;
}

// Hàm chính để tìm trong chuỗi có nhiều đoạn
int get_LocationSubstring(char* input, char* result, uint8_t length) {
    char *start = input;

    while ((start = strstr(start, "[OK],")) != NULL) {
        start += 5;  // Bỏ qua "[OK],"
        
        // Tìm đoạn kết thúc bằng dấu chấm phẩy hoặc hết chuỗi
        char *end = strstr(start, "[OK],");
        size_t segment_len = end ? (size_t)(end - start) : strlen(start);

        // Tạo bản sao tạm của đoạn này để xử lý
        char temp[256] = {0};
        strncpy(temp, start, segment_len);
        temp[segment_len] = '\0';

        if (extract_LA_code_from_segment(temp, result, length)) {
            return 1;
        }

        // Nếu còn đoạn [OK], khác, tiếp tục tìm
        start = end;
    }

    return 0;
}

//Struct nhận thông tin từ Center
center_infor_typedef xTCP_Client_CenterInfor;

static void Reconnect_To_Socket(void) {
    //Clear conncet
    // shutdown(sock, 0);
    // close(sock);
    // sock = -1;

    // sock =  socket(addr_family, SOCK_STREAM, ip_protocol);

    while(1) {

        

        //Nhận thông tin từ Center
        if(x_Center_Infor_Queue != NULL) {
            if(xQueuePeek(x_Center_Infor_Queue, &xTCP_Client_CenterInfor, 10 / portTICK_PERIOD_MS) == pdTRUE) { 

            }
        }

        if(xTCP_Client_CenterInfor.operationMode == SLEEP_MODE) {
            vTaskDelay(pdMS_TO_TICKS(5000)); // Delay 5 giây
            continue;
        }

        //Kiểm tra trạng thái socket, gửi lên màn hình
        socketStatus.status = isConnectedToServer;
        if(x_Socket_2_LCD_Task_Queue != NULL) {
            // printf("Socket connect status: %d\n", isConnectedToServer);
            xQueueOverwrite(x_Socket_2_LCD_Task_Queue, &socketStatus);
        }

        //printf("Try to recreate socket............\n");
        if (sock != -1) {
            ESP_LOGI(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
            sock = -1;
        }

        sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE("Reconnect task", "Unable to recreate socket: errno %d", errno);

            //Đợi 1 khoảng thời gian trước khi tạo lại socket
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        // printf("\nTry to reconnect............\n\n");
        // ESP_LOGI("Reconnect task", "Socket created, reconnecting to %s:%d", host_ip, portValue);
        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE("Reconnect task", "Socket unable to reconnect: errno %d", errno);
            isConnectedToServer = 0;
            shutdown(sock, 0);
            close(sock);
            sock = -1;
            // printf("After reconnect sock = %d\n",sock);
        }
        else {
            if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
                ESP_LOGE(TAG, "Failed to set socket send timeout");
            }

            ESP_LOGI("Reconnect task", "Socket: Successfully Reconnected");
            isConnectedToServer = 1;

            return;
        }
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

char old_epc[20] = {0};     //Tránh gửi 2 thẻ giống nhau liên tiếp
char epc_arr[20][20];       //Mảng chống phát tiếng kêu trùng cho thẻ OK
uint8_t socket_cards_arr_count = 0;
char epc_NG_arr[20][20];       //Mảng chống phát tiếng kêu trùng cho thẻ NG
uint8_t socket_NG_cards_arr_count = 0;

static uint32_t checkNGCardTime = 0; //Biến thời gian phục vụ việc reset mảng lọc thẻ NG kêu trùng
uint8_t isGetNGCards = 0;
CARD LocationEpc;  //Biến lưu thẻ dây đăng ký location, phục vụ việc xóa thẻ dây khỏi mảng ALive sau khi đăng ký xong
uint8_t isEnoughSendMsg = 0;           //Biến kiểm soát số lần gửi để server
uint8_t send_count = 0;
//Tính năng: Khởi tạo các biến cần thiết cho việc nhận dữ liệu từ server.
char rx_buffer[SIZE_OF_RX_TCP_BUFFER];
uint8_t rx_check = 0;       // 0 - No response || 1 - Receive OK || 2 - Receive NG || 3- Duplicated card

static void TCP_IP_CLIENT_SEND_task(void *arg)  {
    static const char *TCP_IP_CLIENT_SEND_TAG = "TCP_IP_CLIENT_SEND_TASK";
    esp_log_level_set(TCP_IP_CLIENT_SEND_TAG, ESP_LOG_INFO);

    char epcBatch[MAX_EPC_BATCH][MAX_EPC_LENGTH];  // Bộ đệm chứa các mã EPC
    uint8_t epcCount = 0;  // Đếm số lượng mã thẻ đã lưu

    char strEpc[MAX_EPC_LENGTH] = {0};
    char sendBuffer[256] = {0};
    TickType_t ticks_to_send = xTaskGetTickCount();

    bool is_waiting_for_respone = false;

    //Lưu thời gian hiện tại để theo dõi thời gian đã trôi qua trong vòng lặp sau.
    uint32_t checkDuplicateCardTime = xTaskGetTickCount();

    while (1) {

            //Nhận thông tin từ Center
        if(x_Center_Param_Queue != NULL) {
            //// Peek thành công: xUHF_RFID_CenterInfor chứa thông tin mới từ Center
            if(xQueuePeek(x_Center_Param_Queue, &xTCP_CenterParam, 10 / portTICK_PERIOD_MS) == pdPASS) { 
            }
        } 

        //Nhận thông tin từ Center
        if(PROCESS_CODE && PROCESS_CODE_CL != NULL) {
            if(xQueuePeek(x_Center_Infor_Queue, &xTCP_Client_CenterInfor, 10 / portTICK_PERIOD_MS) == pdTRUE) { 

            }
        }

        static workMode_t oldTCPWorkMode = {
                .operationMode = IN_MODE,
        };
        //Kiểm tra sự thay đổi chế độ làm việc
        if(oldTCPWorkMode.operationMode != xTCP_Client_CenterInfor.operationMode) {
            //Nếu chuyển từ OUT Mode sang IN Mode
            if(oldTCPWorkMode.operationMode == OUT_MODE && xTCP_Client_CenterInfor.operationMode == IN_MODE) {
                
            } 
            //Nếu chuyển từ Work Mode sang Sleep Mode
            if(oldTCPWorkMode.operationMode != SLEEP_MODE && xTCP_Client_CenterInfor.operationMode == SLEEP_MODE) {

            }
            //Nếu chuyển từ Sleep Mode sang Work Mode
            if(oldTCPWorkMode.operationMode == SLEEP_MODE && xTCP_Client_CenterInfor.operationMode != SLEEP_MODE) {
                //Clear buffer lọc
                memset(checkUnDuplicateCardToServer, 0x00, sizeof(checkUnDuplicateCardToServer));
                memset(epcBatch, 0x00, sizeof(epcBatch));

                int ping_err = send(sock, "Ping", strlen("Ping"), 0);
                /* Reconnect to Server if occur error */
                if (ping_err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    isConnectedToServer = 0;
                    Reconnect_To_Socket();
                }
                else {
                    ESP_LOGI(TAG, "Connect to TCP server is OK");
                    isConnectedToServer = 1; 
                }
            }
            oldTCPWorkMode.operationMode = xTCP_Client_CenterInfor.operationMode;
        }


        if(xTCP_Client_CenterInfor.operationMode == SLEEP_MODE) {
            // ESP_LOGW(TAG, "In Sleep Mode!");
            vTaskDelay(pdMS_TO_TICKS(5000)); // Delay 5 giây
            continue;
        }

        //Kiểm tra trạng thái socket, gửi lên màn hình
        socketStatus.status = isConnectedToServer;
        if(x_Socket_2_LCD_Task_Queue != NULL) {
            // printf("Socket connect status: %d\n", isConnectedToServer);
            xQueueOverwrite(x_Socket_2_LCD_Task_Queue, &socketStatus);
        }

        static uint32_t checkUnDuplicateTime = 0;
        //Clear mảng check trùng lặp mỗi 5s
        if(xTaskGetTickCount() - checkUnDuplicateTime >= 5000 / portTICK_PERIOD_MS) {
            for(uint8_t i = 0; i < SOCKET_CHECK_DUPLICATE_SIZE; i++) {
                memset(checkUnDuplicateCardToServer[i].epc, 0, MAX_EPC_LENGTH);     
            }
            checkUnDuplicateTime = xTaskGetTickCount();
        }
        
        //Kiếm tra tình trạng socket có hợp lệ, có kết nối tới server không
        if((sock != -1) && (isConnectedToServer != 0)) 
        {

            fd_set writefds;
            struct timeval timeout;

            // Dùng select để kiểm tra kết nối
            FD_ZERO(&writefds);
            FD_SET(sock, &writefds);

            timeout.tv_sec = 2;
            timeout.tv_usec = 0;
            static uint32_t checkConnectCheckTime = 0; 

            if(xTaskGetTickCount() - checkConnectCheckTime >= 50/ portTICK_PERIOD_MS) {
                int result = select(sock + 1, NULL, &writefds, NULL, &timeout);
                if (result > 0 && FD_ISSET(sock, &writefds)) {
                    int so_error;
                    socklen_t len = sizeof(so_error);
                    getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);

                    if (so_error == 0) {
                        // ESP_LOGI(TAG, "Kết nối thành công tới server.\n");
                    } else {
                        ESP_LOGE(TAG, "Lỗi kết nối: %s\n", strerror(so_error));
                        isConnectedToServer = 0;
                    }
                } else {
                    ESP_LOGW(TAG, "Không thể kết nối tới server hoặc bị timeout.\n");
                    isConnectedToServer = 0;
                }

                char ch;
                int len = recv(sock, &ch, 1,  MSG_PEEK | MSG_DONTWAIT);
                if (len == 0) {
                    // Server đóng kết nối
                    isConnectedToServer = 0;
                }

                checkConnectCheckTime = xTaskGetTickCount();
            }

            // ESP_LOGW(TAG, "Connect OK!");
            int err = 0;
            //Kiểm tra xem hàng đợi x_polling_UHF_RFID_2_Server_Queue có hợp lệ và nhận dữ liệu từ hàng đợi.
            if (x_polling_UHF_RFID_2_Server_Queue != NULL) {

                while ((epcCount < MAX_EPC_BATCH) && (xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &socket_cards, 10/ portTICK_PERIOD_MS) == pdTRUE))
                {
                    if(epcCount==0) {
                        ticks_to_send=xTaskGetTickCount();
                    }
                    /**/    
                    //[Trung]New code to Server      
                    //Kiểm tra xem có thẻ nào đã được quét hay không.
                    if(socket_cards.result_cards <= 0)
                    {
                        continue;
                    }

                    socket_cards.epc[MAX_EPC_LENGTH - 1] = 0x00;
                    if(socket_cards.epc_len > MAX_EPC_LENGTH - 1) {
                        socket_cards.epc_len = MAX_EPC_LENGTH - 1;
                    }

                    uint8_t epc_index = 0;
                    for ( epc_index = 0; epc_index < epcCount; epc_index++)
                    {
                        if (strcmp((char*)socket_cards.epc, epcBatch[epc_index]) == 0)
                        {
                            break;
                        } 
                    }

                    if (epc_index < epcCount)
                    {
                        ESP_LOGE(TAG, "Receive duplicated card, Data=%s\n", socket_cards.epc);
                        continue;
                    }
                    
                    // Xử lý ký tự lạ nếu EPC bắt đầu bằng 'L'
                    // Giới hạn chuỗi còn 8 ký tự

                    // Lọc các thẻ đã gửi trước đó
                    if (TCP_filterCardInfo((char*)socket_cards.epc) == false) {
                        ESP_LOGE(TAG, "Socket: Receive new tag but duplicated, index=%d - Data=%s\n", epcCount, socket_cards.epc);
                        continue;
                    }
                    // In ra thông tin trước khi gửi
                    // uint16_t soLanNhan = 0;  // Số lần nhận
                    // Lưu mã thẻ vào bộ đệm
                    socket_cards.epc[MAX_EPC_LENGTH - 1] = 0x00;
                    strncpy(epcBatch[epcCount++], (char*)socket_cards.epc, socket_cards.epc_len + 1); //Cộng 1 để thêm ký tự kết thúc chuỗi
                    strncpy(strEpc, (char*)socket_cards.epc, socket_cards.epc_len + 1);
                    ESP_LOGI(TAG, "Socket: Receive new tag, index=%d - Data=%s -  String=%s - Buffer=%s\n", epcCount, socket_cards.epc, strEpc, epcBatch[epcCount-1]);
                    // ESP_LOGI(TAG, "Socket: Send Count = %d\n", send_count);
                    // if (epcCount >= MAX_EPC_BATCH) epcCount = MAX_EPC_BATCH;  // Giới hạn không vượt quá bộ đệm
                    // vTaskDelay(10 / portTICK_PERIOD_MS);
                }             
            }   

            //Gửi dữ liệu đến server
            uint8_t isNormalTag = 1;
            for (int i = 0; i < epcCount; i++) {
                if (epcBatch[i][0] == 'L')
                {
                    isNormalTag = 0;
                    break;
                }
            }
            // Send epcBatch every TIME_TO_SEND ms
            if((isNormalTag == 0 && xTaskGetTickCount() - ticks_to_send > (TIME_TO_SEND_LOCATION_TAG/ portTICK_PERIOD_MS)) || (isNormalTag == 1 && (xTaskGetTickCount() - ticks_to_send) > (TIME_TO_SEND/ portTICK_PERIOD_MS)) || (epcCount >= MAX_EPC_BATCH))  
            {
                ticks_to_send= xTaskGetTickCount();

                if (epcCount > 0) {
                    // vTaskDelay(2000 / portTICK_PERIOD_MS);  // Độ trễ 2 giây để gom các mã
                    // Tạo chuỗi chứa nhiều mã thẻ để gửi đi mà không có ký tự xuống dòng

                    memset(sendBuffer, 0x00, sizeof(sendBuffer));

                    // Location Tag
                    int location_index = -1;
                    for (int i = 0; i < epcCount; i++) {
                        if (epcBatch[i][0] == 'L')
                        {
                            // buf_cat(sendBuffer, 256, epcBatch[i]);
                            // buf_cat(sendBuffer, 256, ",");
                            location_index = i;
                            break;
                        }
                    }
                    
                    // Wire Tag, Build Message, Send Request register Location
                    int msg_len = 0;
                    for (int i = 0; i < epcCount; i++) {
                        msg_len = strlen(sendBuffer);

                        // does not contain Location Tag in buffer
                        if (location_index < 0)
                        {
                            if ((msg_len > 0))
                            {
                                buf_cat(sendBuffer, 256, ",");
                            }

                            if (epcBatch[i][0] != 'L')
                            {
                                buf_cat(sendBuffer, 256, epcBatch[i]);
                            }
                        }
                        // contain Location Tag in buffer
                        else
                        {
                            if (epcBatch[i][0] != 'L')
                            {
                                memset(LocationEpc.epc, 0, sizeof(LocationEpc.epc));
                                memset(sendBuffer, '\0', sizeof(sendBuffer));
                                buf_cat(sendBuffer, 256, epcBatch[location_index]);
                                buf_cat(sendBuffer, 256, ",");
                                buf_cat(sendBuffer, 256, epcBatch[i]);
                                strcpy((char*)LocationEpc.epc, epcBatch[location_index]);

                                // Send Location+Tag
                                ESP_LOGI(TAG, "Socket: Send Location + WireTag to Server, nums=%d, data len=%d, data=%s.\n", epcCount, strlen(sendBuffer), sendBuffer);
                                //Nối thêm mã công đoạn
                                char result[280] = {0};
                                if(xTCP_CenterParam.mode == SA_MODE || xTCP_CenterParam.mode == GROUP_MODE || xTCP_CenterParam.mode == SA_GROUP_MODE)
                                {
                                    sprintf(result, "%s:%s", PROCESS_CODE, sendBuffer);
                                }
                                // else if(xTCP_CenterParam.mode == CL03_MODE)
                                // {
                                //     sprintf(result, "%s:%s", PROCESS_CODE_CL, sendBuffer);
                                // }
                                err = send(sock, result, strlen(result), 0);

                                if (err < 0) {
                                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                                    break;
                                } else {
                                    ESP_LOGI(TAG, "Batch of EPCs sent to server");
                                    send_count++;
                                }

                                // epcCount = 0;  // Xóa bộ đệm sau khi gửi
                                checkDuplicateCardTime = xTaskGetTickCount();   // Ticks of the last sent
                            }
                        }
                        
                    }

                    // Only send Wire Tags
                    if (location_index < 0)
                    {
                        //Thêm ký tự ','
                        msg_len = strlen(sendBuffer);
                        if ((msg_len > 0))
                        {
                            buf_cat(sendBuffer, 256, ",");
                        }
                        int len_sendBuffer = strlen(sendBuffer);

                        vTaskDelay(20 / portTICK_PERIOD_MS);
                        
                        if(len_sendBuffer > 0) {
                            // err = send(sock, sendBuffer, len_sendBuffer, 0);
                            //Nối thêm mã công đoạn
                            char result[280] = {0};
                            // sprintf(result, "%s:%s", PROCESS_CODE, sendBuffer);
                            // if(xTCP_CenterParam.mode == CL01_MODE)
                            //     {
                            //         sprintf(result, "%s:%s", PROCESS_CODE, sendBuffer);
                            //     }
                                // else if(xTCP_CenterParam.mode == CL03_MODE)
                                // {
                                //     sprintf(result, "%s:%s", PROCESS_CODE_CL, sendBuffer);
                                // }
                                sprintf(result, "%s:%s", PROCESS_CODE, sendBuffer);
                            ESP_LOGI(TAG, "Socket: Send WireTags to Server, nums=%d, data len=%d, data=%s\n", epcCount, len_sendBuffer, result);
                            err = send(sock, result, strlen(result), 0);
                        }
                        else {
                            err = -1;
                        }
                        

                        if (err < 0) {
                            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                        } else {
                            ESP_LOGI(TAG, "Batch of EPCs sent to server");
                            send_count++;
                        }

                        checkDuplicateCardTime = xTaskGetTickCount();   // Ticks of the last sent
                    }

                    // Clear Buffer
                    epcCount = 0;  // Xóa bộ đệm sau khi gửi 
                    memset(epcBatch, 0x00, sizeof(epcBatch));  
                    
                }

            }


            /* Clear epc_NG_arr buffer */
            if(isGetNGCards == 1 && xTaskGetTickCount() - checkNGCardTime >= 1500 / portTICK_PERIOD_MS) {
                checkNGCardTime = xTaskGetTickCount();
                isGetNGCards = 0;
                
                //Mỗi 1.5s sẽ tiến hành reset buffer lọc thẻ trùng
                for (int j = 0; j < 20; j++) {
                    strcpy(epc_NG_arr[j], "");
                }
            }

            /* Reconnect to Server if occur error */
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                isConnectedToServer = 0;
                Reconnect_To_Socket();
                // CARD temp_socket_cards1;
                // while(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &temp_socket_cards1, 10) == pdTRUE);
            }
        }
        else {
            ESP_LOGE(TAG, "Unable to connect socket...socket = %d, isConnectedToServer = %u !\n", sock, isConnectedToServer);
            isConnectedToServer = 0;
            Reconnect_To_Socket();
            // CARD temp_socket_cards2;
            // while(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &temp_socket_cards2, 10) == pdTRUE);
        }      
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

static void TCP_IP_CLIENT_RECV_task(void *arg)  {
    while(1) {
        //Kiếm tra tình trạng socket có hợp lệ, có kết nối tới server không
        if((sock != -1) && (isConnectedToServer != 0)) 
        {
#if 1   /* Recv non while block */ 
            /* Receive Respone from Server */
            if (1) // send_count > 0
            {
                // Wait for respone
                if(1) 
                {
                    int len = 0;
                    
                    // Đặt lại (reset) nội dung của bộ đệm nhận rx_buffer.
                    for(uint16_t i = 0; i < SIZE_OF_RX_TCP_BUFFER; i++) {
                        rx_buffer[i] = '\0';
                    }

                    len = recv(sock, rx_buffer, SIZE_OF_RX_TCP_BUFFER - 1, MSG_DONTWAIT); 

                    // ESP_LOGI(TAG, "Receiving!: %d", len);

                    // Error occurred during receiving
                    if (len == 0) {
                        ESP_LOGE(TAG, "recv failed: errno %d", errno);
                        isConnectedToServer = 0;
                    }
                    // Data received
                    else if(len >0) {
                        
                        rx_buffer[len] = '\0'; // Null-terminate whatever we received and treat like a string
                        ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                        ESP_LOGI(TAG, "%s", rx_buffer);
                        // ESP_LOGI(TAG, "Thời gian phản hồi từ server: %lu ms", (xTaskGetTickCount() - checkDuplicateCardTime) * portTICK_PERIOD_MS);
                        ESP_LOGI(TAG, "Socket: Send Count 2 = %d\n", send_count);
                        int MaxChamPhay = 0;
                        for(int i = 0; i < len; i ++) 
                        {
                            //Nếu vượt ngoài vị trí có thể chứa đủ frame dữ liệu nhận về
                            if(i >= SIZE_OF_RX_TCP_BUFFER - NORMAL_SIZE_OF_RESPONSE_DATA_FROM_SERVER) {
                                break;
                            }
                            
                            // Duyệt qua từng ký tự trong rx_buffer để kiểm tra xem có chuỗi phản hồi [OK] từ server hay không.
                            if (rx_buffer[i] == '[' && rx_buffer[i + 1] == 'O' && rx_buffer[i + 2] == 'K' && rx_buffer[i + 3] == ']') {

                                #if 0
                                if (strncmp(&rx_buffer[i], "[OK]L", 5) == 0 ) { // && isdigit(rx_buffer[i+6]) && isdigit(rx_buffer[i+7])
                                    ESP_LOGW(TAG, "Nhan phan hoi location");
                                    char LocaIn[15]={0};
                                    strncpy(LocaIn, &rx_buffer[i], 8);
                                    if(xQueueLocation != NULL)
                                        xQueueSend(xQueueLocation, LocaIn, 10);
                                    break;
                                }
                                #endif

                                if (strstr(&rx_buffer[i], "Set new location OK!") != NULL)
                                {
                                    ESP_LOGW(TAG, "Nhan phan hoi dang ki location: THANH CONG");
                                    char LocaIn[10]={0};
                                    // char* location = strchr(&rx_buffer[i], 'L');
                                    // memcpy(LocaIn, location, strchr(location, ',') - location);
                                    uint8_t locationRes = get_LocationSubstring(rx_buffer, LocaIn, 4);

                                    if(locationRes == 1) {
                                        if(xQueueLocation != NULL)
                                            xQueueSend(xQueueLocation, LocaIn, 10);

                                        if(xQueueDeleteWireRegisterLocation != NULL) {
                                            xQueueSend(xQueueDeleteWireRegisterLocation, &LocationEpc, 10);
                                        }
                                    }
                                    break;
                                }

                                ESP_LOGW(TAG, "Nhan phan hoi the day");
                                /* Xu li UHF RFID deo tay */
                                MaxChamPhay= TachData(&rx_buffer[i]);

                                if(MaxChamPhay > 0)
                                {
                                    for(int CountChamPhay = 0; CountChamPhay < MaxChamPhay; CountChamPhay++) {
                                        if(x_Card_2_LCD_Task_Queue != NULL) {
                                            xQueueSend(x_Card_2_LCD_Task_Queue, &wires[CountChamPhay], 10 / portTICK_PERIOD_MS);
                                        }
                                    }
                                    // Receive OK
                                    rx_check = 1;
                                }

                                
                                /*Xu li UHF RFID xe hang*/


                                break; //Thoát ra ngoài, không tìm thêm [OK] nữa           
                            }
                            //Nếu tìm thấy [NG], cập nhật mảng epc_NG_arr để lưu thẻ không hợp lệ và ghi lại thông báo NG CARD!.
                            else if (rx_buffer[i] == '[' && rx_buffer[i + 1] == 'N' && rx_buffer[i + 2] == 'G' && rx_buffer[i + 3] == ']') 
                            {
                                printf("NG CARD!\n");
                                isEnoughSendMsg = 0;    //Cho phép tiếp tục gửi lại tin nhắn

                                // // Receive NG
                                // rx_check = 2;

                                // for(uint8_t check = 0; check < 20; check++) {
                                //     if(strcmp(strEpc, epc_NG_arr[check]) == 0) {
                                //         rx_check = 3;                              
                                //         break;              //Đã gửi thẻ trước đó
                                //     }
                                // }
                                // if(rx_check == 3) {
                                //     break;
                                // }
                                // //Lưu thẻ mới vào mảng 
                                // strcpy(epc_NG_arr[socket_NG_cards_arr_count++], strEpc);    
                                // socket_NG_cards_arr_count = socket_NG_cards_arr_count % 20;

                                ESP_LOGW(TAG, "Nhan phan hoi the day");
                                /* Xu li UHF RFID deo tay */
                                MaxChamPhay= TachData(&rx_buffer[i]);
                                
                                if(MaxChamPhay > 0)
                                {
                                    for(int CountChamPhay = 0; CountChamPhay < MaxChamPhay; CountChamPhay++) {
                                        if(x_Card_2_LCD_Task_Queue != NULL) {
                                            //
                                            // wires[CountChamPhay].location[0] = '?';
                                            // wires[CountChamPhay].location[1] = 0X00;
                                            xQueueSend(x_Card_2_LCD_Task_Queue, &wires[CountChamPhay], 10 / portTICK_PERIOD_MS);
                                            vTaskDelay(10 / portTICK_PERIOD_MS);
                                        }
                                    }
                                    // Receive OK
                                    rx_check = 1;
                                }

                                // Đảm bảo bộ đếm hoạt động khi có thẻ NG
                                if(isGetNGCards == 0) {
                                    checkNGCardTime = xTaskGetTickCount();
                                }
                                isGetNGCards = 1;

                                break; //Break chưa thoát khỏi vòng while wait-time to receive
                            }
                            //Nếu thẻ OK
                            if(rx_check == 1) {                           
                                break;
                            }
                        }
                        if(rx_check == 1 || rx_check == 2) {
                            send_count--;
                        }                 
                    }
                    // Nếu nhận được thông tin thẻ OK hoặc NG

                    // if(rx_check == 1 || rx_check == 2) {
                    //     send_count--;
                    // }
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
                // Timeout waiting respone
                else
                {
                    // checkDuplicateCardTime = xTaskGetTickCount();
                    if(rx_check == 0) {
                        printf("NO RESPONSE [%"PRIu16"]!\n", send_count);
                         if(isEnoughSendMsg < 8){               //Mỗi tin nhắn gửi tối đa 8 lần
                            isEnoughSendMsg++;
                        }                           
                    }
                    send_count = 0;
                }
            }

#endif  /* Recv non while block */ 
        }
        else {
            // ESP_LOGE(TAG, "Unable to connect socket...socket = %d, isConnectedToServer = %u !\n", sock, isConnectedToServer);
            isConnectedToServer = 0;
            // Reconnect_To_Socket();
        }  
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}


uint8_t TCP_filterCardInfo(char* epc) {
    for (int i = 0; i < SOCKET_CHECK_DUPLICATE_SIZE; i++) {
        if (strcmp(epc, (char*)checkUnDuplicateCardToServer[i].epc) == 0) {
            return false;
        }
    }
    return true;
}

void TCP_Init(void) {
    for(uint8_t i = 0; i < SOCKET_CHECK_DUPLICATE_SIZE; i++) {
        memset(checkUnDuplicateCardToServer[i].epc, 0, MAX_EPC_LENGTH);     
    }
    // x_GPIO_UHF_RFID_2_Server_Queue = xQueueCreate( 100, sizeof(CARD));
    //Queue gửi dữ liệu trở lại task UHF RFID để lưu những thẻ đọc đúng xe hàng
   
    x_Server_2_UHF_RFID_Task = xQueueCreate( 100, sizeof(CARD));
    x_Socket_2_LCD_Task_Queue = xQueueCreate( 1, sizeof(sockettcp_info));
    xQueueLocation =xQueueCreate(10, 15 * sizeof(char));
    xQueueDeleteWireRegisterLocation = xQueueCreate(10, sizeof(CARD));
    strcpy(host_ip,HOST_IP_ADDR); 
    portValue = PORT;  

    inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(portValue);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, portValue);
    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        isConnectedToServer = 0;
    }
    else {
        ESP_LOGI(TAG, "Socket: Successfully connected");
        isConnectedToServer = 1;
    }
    ESP_LOGW(TAG,"TCP_Init sock = %d\n",sock);

    // Cấu hình timeout cho send (30ms)
    timeout.tv_sec = 0;            // Số giây
    timeout.tv_usec = 30000;       // Số micro giây (30ms)

    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        ESP_LOGE(TAG, "Failed to set socket send timeout");
        close(sock);
        return;
    }

    xTaskCreate(&TCP_IP_CLIENT_SEND_task, "tcp_ip_client_send_task", 1024*11, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(&TCP_IP_CLIENT_RECV_task, "tcp_ip_client_recv_task", 1024*8, NULL, configMAX_PRIORITIES - 1, NULL);

}
#endif
