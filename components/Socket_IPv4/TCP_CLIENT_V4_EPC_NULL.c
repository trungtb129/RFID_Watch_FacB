/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "TCP_CLIENT_V4.h"


#define TAG  "TCP-IP Client"
#define TIME_TO_SEND 2000
#define TIME_TO_WAIT_RESPONE 3000
// static const char *payload = "Message from ESP32 ";

#ifdef TEST_UHF_RFID_WATCH
extern QueueHandle_t x_polling_UHF_RFID_2_Server_Queue;
extern QueueHandle_t x_Card_2_LCD_Task_Queue;      //Gửi dữ liệu serial lên màn hình LCD

QueueHandle_t x_GPIO_UHF_RFID_2_Server_Queue;                   //Tạm thời không dùng
QueueHandle_t x_Server_2_UHF_RFID_Task;                         ////Queue gửi thông tin thẻ trở lại UHF_RFID_task giúp tránh đọc lặp lại
QueueHandle_t x_Socket_2_LCD_Task_Queue;                        //Queue gửi trạng thái đã/chưa kết nối tới Socket server

// char rx_buffer[128];
char host_ip[18];
uint16_t portValue;
int addr_family = 0;
int ip_protocol = 0;
struct sockaddr_in dest_addr;

int sock;
uint8_t isConnectedToServer = 0;
sockettcp_info socketStatus;

socket_CARD socket_cards;
uint8_t socket_read_buffer[SOCKET_READ_CARD_BUFFER_LENGTH] = {0};
wire_infor wires[MAX_ENTRIES];

socket_EPC_CARD checkUnDuplicateCardToServer[SOCKET_CHECK_DUPLICATE_SIZE];
uint8_t checkUnDuplicateCardToServer_Count = 0;
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
void CopyBuffToStruct (wire_infor *wires, char Buffer[MAX_FIELDS][MAX_LENGTH]) {
        
        memset(wires,'\0',sizeof(wire_infor));
        // strncpy(wires->status_card, Buffer[0], sizeof(wires->status_card) - 1);
        // strncpy(wires->serial, Buffer[1], sizeof(wires->serial) - 1);
        // strncpy(wires->location, Buffer[2], sizeof(wires->location) - 1);
        // strncpy(wires->color, Buffer[3], sizeof(wires->color) - 1);
        // strncpy(wires->length, Buffer[4], sizeof(wires->length) - 1);
        // strncpy(wires->line, Buffer[5], sizeof(wires->line) - 1);
        // strncpy(wires->date, Buffer[6], sizeof(wires->date) - 1);
        // strncpy(wires->lo, Buffer[7], sizeof(wires->lo) - 1);
        // strncpy(wires->group, Buffer[8], sizeof(wires->group) - 1);

        strcpy(wires->status_card, Buffer[0]);
        strcpy(wires->serial, Buffer[1]);
        strcpy(wires->location, Buffer[2]);
        strcpy(wires->color, Buffer[3]);
        strcpy(wires->length, Buffer[4]);
        strcpy(wires->line, Buffer[5]);
        strcpy(wires->date, Buffer[6]);
        strcpy(wires->lo, Buffer[7]);
        strcpy(wires->group, Buffer[8]);
        // printf("\nCHECK LOG1: %s\n", Buffer[8]);
        printf("\nCHECK LOG1: %s\n", wires->group);

}
	char Buffer[MAX_ENTRIES][MAX_FIELDS][MAX_LENGTH] = {}; 

int TachData(char DataFromServer[]) {
	// char DataFromServer[] = "[OK]45E00815,LA99,BR,601,TRFC,15,1,2;[OK]45E03088,LA99,SB,3359,TRFC,15,1,304;[OK]45E00891,LA99,G -1,3969,TRFC,15,1,198;[OK]45E00892,LA99,BR,3969,TRFC,15,1,198;[OK]45E00893,LA99,G,3969,TRFC,15,1,198;[OK]45E03086,LA99,L,2374,TRFC,15,1,269;[OK]45E03087,LA99,GR,3359,TRFC,15,1,304;[OK]45E03373,LA99,LG,2000,TRFC,15,1,17;[OK]45E00894,LA99,L,3969,TRFC,15,1,198;[OK]45I1F292,LA01,,1507,589D,20,1,7;";
int MaxChamPhay=0;
    memset(Buffer, 0, sizeof(Buffer));
	int i = 0;
	
	char str[500]= {0};
	int prev_index = 0;
	while(1) {
		int offset_index =findSemicolonPosition(&DataFromServer[prev_index], ';');
		
		if (offset_index != -1) {
			printf("Offset index: %d\n", offset_index);
			memset(str,0,sizeof(str));
			memcpy(str,&DataFromServer[prev_index], offset_index);
			prev_index += offset_index + 1;

			char *field;
			
			char *statusEnd = strchr(str, ']') + 1;
			
			strncpy(Buffer[i][0], str, 4);
			
			int j = 1;
			field = strtok(statusEnd, ",");
			while (field != NULL && j < MAX_FIELDS) {
				strncpy(Buffer[i][j], field, MAX_LENGTH - 1); // LF0u ta;+ng giC! tra; vC o buffer
				Buffer[i][j][MAX_LENGTH - 1] = '\0'; // Da:#m ba:#o ka:?t thC:c chua;i
				field = strtok(NULL, ",");
				j++;
			}
			
			i++;
			
		} else {
			printf("Dont contain ';'.\n");

            #if 0   /* NoteTestSuMi */
            offset_index = strlen(&DataFromServer[prev_index]);
			printf("End Offset index: %d\n", offset_index);
            if (offset_index == 0)
            {
                break;
            }
            
			memset(str,0,sizeof(str));
			memcpy(str,&DataFromServer[prev_index], offset_index);
			prev_index += offset_index + 1;

			char *field;
			
			char *statusEnd = strchr(str, ']') + 1;
			
			strncpy(Buffer[i][0], str, 4);
			
			int j = 1;
			field = strtok(statusEnd, ",");
			while (field != NULL && j < MAX_FIELDS) {
				strncpy(Buffer[i][j], field, MAX_LENGTH - 1); // LF0u ta;+ng giC! tra; vC o buffer
				Buffer[i][j][MAX_LENGTH - 1] = '\0'; // Da:#m ba:#o ka:?t thC:c chua;i
				field = strtok(NULL, ",");
				j++;
			}
			
			i++;
            #endif

			break; 
		}
	}


	MaxChamPhay= i;
	printf("MaxChamPhay %d\n", MaxChamPhay);

	for(int index=0; index<MaxChamPhay; index++) {
		for(int j=0; j<MAX_FIELDS; j++) {
			if (strlen(Buffer[index][j]) > 0) { //
				printf("  Field %d: %s\n", j, Buffer[index][j]);
			}
		}
		printf("\n");
	}

    for(int index=0;index<MaxChamPhay;index++) {
        CopyBuffToStruct(&wires[index],Buffer[index]);
    }

// printf("Giá trị Buffer[0][1]: %s\n", Buffer[2][0]);
        for (int index = 0; index < MaxChamPhay; index++) {
        printf("Wires Info %d:\n", index + 1);
        printf("  Status Card: %s\n", wires[index].status_card);
        printf("  Serial: %s\n", wires[index].serial);
        printf("  Location: %s\n", wires[index].location);
        printf("  Color: %s\n", wires[index].color);
        printf("  Length: %s\n", wires[index].length);
        printf("  Line: %s\n", wires[index].line);
        printf("  Date: %s\n", wires[index].date);
        printf("  Lo: %s\n", wires[index].lo);
        printf("  Group: %s\n", wires[index].group);
        printf("\n");
	    
    }
       return MaxChamPhay;
}
static void Reconnect_To_Socket(void) {
    //Clear conncet
    // shutdown(sock, 0);
    // close(sock);
    // sock = -1;

    // sock =  socket(addr_family, SOCK_STREAM, ip_protocol);

    while(1) {
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
            shutdown(sock, 0);
            close(sock);
            sock = -1;
            // printf("After reconnect sock = %d\n",sock);
        }
        else {
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

uint8_t isEnoughSendMsg = 0;           //Biến kiểm soát số lần gửi để server

#ifndef TCP_OLD_TU
#define MAX_EPC_BATCH 10  // Số lượng mã EPC tối đa trong một cụm gửi
static void TCP_IP_CLIENT_SEND_task(void *arg)  {
    static const char *TCP_IP_CLIENT_SEND_TAG = "TCP_IP_CLIENT_SEND_TASK";
    esp_log_level_set(TCP_IP_CLIENT_SEND_TAG, ESP_LOG_INFO);

    char epcBatch[MAX_EPC_BATCH][20];  // Bộ đệm chứa các mã EPC
    int epcCount = 0;  // Đếm số lượng mã thẻ đã lưu

    char strEpc[20] = {0};
    char sendBuffer[512] = "";
    TickType_t ticks_to_send = xTaskGetTickCount();
    uint16_t send_count = 0;

    bool is_waiting_for_respone = false;
    //Tính năng: Khởi tạo các biến cần thiết cho việc nhận dữ liệu từ server.
    char rx_buffer[512];
    uint8_t rx_check = 0;       // 0 - No response || 1 - Receive OK || 2 - Receive NG || 3- Duplicated card
    //Lưu thời gian hiện tại để theo dõi thời gian đã trôi qua trong vòng lặp sau.
    uint32_t checkDuplicateCardTime = xTaskGetTickCount();

    while (1) {
        //Kiểm tra trạng thái socket, gửi lên màn hình
        socketStatus.status = isConnectedToServer;
        xQueueOverwrite(x_Socket_2_LCD_Task_Queue, &socketStatus);
        //Kiếm tra tình trạng socket có hợp lệ, có kết nối tới server không
        if((sock != -1) && (isConnectedToServer != 0)) {
            int err = 0;
            uint16_t soLanNhan = 0;  // Số lần nhận

            //Kiểm tra xem hàng đợi x_polling_UHF_RFID_2_Server_Queue có hợp lệ và nhận dữ liệu từ hàng đợi.
            if (x_polling_UHF_RFID_2_Server_Queue != NULL) 
            {
                // while (0)
                // if(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &socket_cards, 10) == pdTRUE) 
                while ((epcCount < MAX_EPC_BATCH) && (xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &socket_cards, 0) == pdTRUE))
                {
                    /**/    
                    //[Trung]New code to Server      
                    //Kiểm tra xem có thẻ nào đã được quét hay không.
                    if(socket_cards.result_cards <= 0)
                    {
                        continue;
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
                        continue;
                    }
                    
                    // Xử lý ký tự lạ nếu EPC bắt đầu bằng 'L'
                    // Giới hạn chuỗi còn 8 ký tự

                    // Lọc các thẻ đã gửi trước đó
                    if (TCP_filterCardInfo((char*)socket_cards.epc) == false) continue;

                    // In ra thông tin trước khi gửi
                    // uint16_t soLanNhan = 0;  // Số lần nhận
                    // Lưu mã thẻ vào bộ đệm
                    strcpy(epcBatch[epcCount++], (char*)socket_cards.epc);
                    ESP_LOGI(TAG, "Socket: Receive new tag, index=%d - Data=%s -  String=%s - Buffer=%s\n", epcCount, socket_cards.epc, strEpc, epcBatch[epcCount-1]);
                    // if (epcCount >= MAX_EPC_BATCH) epcCount = MAX_EPC_BATCH;  // Giới hạn không vượt quá bộ đệm
                }

#if 0            
                if(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &socket_cards, 10) == pdTRUE) 
                {                                           

                    /**/    
                    //[Trung]New code to Server      
                    //Kiểm tra xem có thẻ nào đã được quét hay không.   
                    if(socket_cards.result_cards > 0) 
                    { 
                        sprintf(strEpc, "%s", socket_cards.epc);

                        // Xử lý ký tự lạ nếu EPC bắt đầu bằng 'L'
                        if (strEpc[0] == 'L') {
                            strEpc[5] = '\0';
                        }

                        strEpc[8] = '\0';  // Giới hạn chuỗi còn 8 ký tự

                        // Lọc các thẻ đã gửi trước đó
                        if (TCP_filterCardInfo(strEpc) == false) continue;

                        // In ra thông tin trước khi gửi
                        printf("Socket: Check before Send to Server, index=%d - String: %s\n", epcCount, strEpc);
                        // uint16_t soLanNhan = 0;  // Số lần nhận
                        // Lưu mã thẻ vào bộ đệm
                        strcpy(epcBatch[epcCount++], strEpc);
                        // if (epcCount >= MAX_EPC_BATCH) epcCount = MAX_EPC_BATCH;  // Giới hạn không vượt quá bộ đệm
                    }
                }
#endif               
                        
            }


            //Gửi dữ liệu đến server
            // Send epcBatch every 2s
            if((( xTaskGetTickCount() - ticks_to_send) > (TIME_TO_SEND/ portTICK_PERIOD_MS)) || (epcCount >= MAX_EPC_BATCH)) 
            {
                ticks_to_send= xTaskGetTickCount();

                if (epcCount > 0) {
                    // vTaskDelay(2000 / portTICK_PERIOD_MS);  // Độ trễ 2 giây để gom các mã
                    // Tạo chuỗi chứa nhiều mã thẻ để gửi đi mà không có ký tự xuống dòng

                    memset(sendBuffer, '\0', sizeof(sendBuffer));

                    // Location Tag
                    for (int i = 0; i < epcCount; i++) {
                        if (epcBatch[i][0] == 'L')
                        {
                            strcat(sendBuffer, epcBatch[i]); 
                        }
                        
                        // strcat(sendBuffer, "\n");  // Thêm ký tự xuống dòng
                    }
                    
                    // Wire Tag
                    for (int i = 0; i < epcCount; i++) {
                        if (epcBatch[i][0] != 'L')
                        {
                            strcat(sendBuffer, epcBatch[i]);  // Chỉ nối chuỗi mà không thêm ký tự nào
                        }
                    }

                    ESP_LOGI(TAG, "Socket: Send to Server, nums=%d, data len=%d.\n", epcCount, strlen(sendBuffer));
                    // Gửi chuỗi các mã thẻ qua socket
                    err = send(sock, sendBuffer, strlen(sendBuffer), 0);
                    ESP_LOGI(TAG, "\nThoi gian luc gui: %ld\n", xTaskGetTickCount());  // Thời gian gửi

                    if (err < 0) {
                        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                        is_waiting_for_respone = false;
                    } else {
                        ESP_LOGI(TAG, "Batch of EPCs sent to server");
                        is_waiting_for_respone = true;

                        send_count++;
                    }

                    epcCount = 0;  // Xóa bộ đệm sau khi gửi
                    checkDuplicateCardTime = xTaskGetTickCount();
                }

            }

            /* */
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                isConnectedToServer = 0;
                Reconnect_To_Socket();
                socket_CARD temp_socket_cards1;
                while(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &temp_socket_cards1, 10) == pdTRUE); // clear queue
            }

            /* */    
            //Nhận dữ liệu từ server
            // if (is_waiting_for_respone == true)
            if (send_count > 0)
            {
                //Bắt đầu một vòng lặp kiểm tra trong khoảng thời gian 800ms. 
                //xTaskGetTickCount(): Hàm này trả về số ticks (đơn vị thời gian) từ khi FreeRTOS bắt đầu chạy. 
                //Vòng lặp sẽ tiếp tục cho đến khi thời gian đã trôi qua lớn hơn 800ms.

                // Wait for respone
                if(xTaskGetTickCount() - checkDuplicateCardTime <= TIME_TO_WAIT_RESPONE/ portTICK_PERIOD_MS) 
                {
                    int len = 0;

                    //Đặt lại (reset) nội dung của bộ đệm nhận rx_buffer.
                    for(uint16_t i = 0; i < 512; i++) {
                        rx_buffer[i] = '\0';
                    }

                    len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, MSG_DONTWAIT); 
                    soLanNhan++;

                    // Error occurred during receiving
                    if (len < 0) {
                        // ESP_LOGE(TAG, "recv failed: errno %d", errno);
                        // isConnectedToServer = 0;
                    }
                    // Data received
                    else if(len >0) {

                        send_count--;
                        // checkDuplicateCardTime = xTaskGetTickCount();

                        rx_buffer[len] = '\0'; // Null-terminate whatever we received and treat like a string
                        ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                        ESP_LOGI(TAG, "%s", rx_buffer);
                        
                        /* DinhTU: Check Server's Responde Data */
                        int MaxChamPhay = 0;
                        for(uint16_t i = 0; i < len; i ++) 
                        {
                            //Duyệt qua từng ký tự trong rx_buffer để kiểm tra xem có chuỗi phản hồi [OK] từ server hay không.
                            if (rx_buffer[i] == '[' && rx_buffer[i + 1] == 'O' && rx_buffer[i + 2] == 'K' && rx_buffer[i + 3] == ']') {
                                /*Xu li UHF RFID deo tay*/
                                MaxChamPhay= TachData(&rx_buffer[i]);

                                if(MaxChamPhay > 0)
                                {
                                    for(int CountChamPhay = 0; CountChamPhay < MaxChamPhay; CountChamPhay++) {
                                        if(x_Card_2_LCD_Task_Queue != NULL) {
                                            xQueueSend(x_Card_2_LCD_Task_Queue, &wires[CountChamPhay], 10);
                                            //Lưu lại vào mảng chống trùng lặp
                                            strcpy((char*)checkUnDuplicateCardToServer[checkUnDuplicateCardToServer_Count++].epc, wires[CountChamPhay].serial);
                                            printf("\nTCP: Dem bien count: %d\n", CountChamPhay);
                                        }
                                    }
                                    // Receive OK
                                    rx_check = 1;
                                }

                                
                                /*Xu li UHF RFID xe hang*/


                                break; //Thoát ra ngoài, không tìm thêm [OK] nữa           
                            }
                            /* DinhTU: `[NG]` */
                            //Nếu tìm thấy [NG], cập nhật mảng epc_NG_arr để lưu thẻ không hợp lệ và ghi lại thông báo NG CARD!.
                            else if (rx_buffer[i] == '[' && rx_buffer[i + 1] == 'N' && rx_buffer[i + 2] == 'G' && rx_buffer[i + 3] == ']') 
                            {
                                printf("NG CARD!\n");
                                isEnoughSendMsg = 0;    //Cho phép tiếp tục gửi lại tin nhắn

                                // Receive NG
                                rx_check = 2;

                                for(uint8_t check = 0; check < 20; check++) {
                                    if(strcmp(strEpc, epc_NG_arr[check]) == 0) {
                                        rx_check = 3;                              
                                        break;              //Đã gửi thẻ trước đó
                                    }
                                }
                                if(rx_check == 3) {
                                    break;
                                }
                                //Lưu thẻ mới vào mảng 
                                strcpy(epc_NG_arr[socket_NG_cards_arr_count++], strEpc);    
                                socket_NG_cards_arr_count = socket_NG_cards_arr_count % 20;

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

                        // if (MaxChamPhay > 0)
                        // {
                        //     break; /* NoteTestSumi */
                        // }
                                                
                        ESP_LOGI(TAG, "\nThoi gian luc nhan: %ld va so lan nhan: %d\n",xTaskGetTickCount(), soLanNhan);
                    }
                    //Nếu nhận được thông tin thẻ OK hoặc NG
                    // if(rx_check == 1 || rx_check == 2) {
                    //     break;
                    // }

                }

                // Timeout waiting respone
                else
                {
                    send_count--;
                    checkDuplicateCardTime = xTaskGetTickCount();
                    if(rx_check == 0) {
                        printf("NO RESPONSE!\n");
                        if(isEnoughSendMsg < 8){               //Mỗi tin nhắn gửi tối đa 8 lần
                            isEnoughSendMsg++;
                        }                            
                    }
                }
            }
            
            /*  */
            if(isGetNGCards == 1 && xTaskGetTickCount() - checkNGCardTime >= 1500 / portTICK_PERIOD_MS) {
                checkNGCardTime = xTaskGetTickCount();
                isGetNGCards = 0;
                
                //Mỗi 1.5s sẽ tiến hành reset buffer lọc thẻ trùng
                for (int j = 0; j < 20; j++) {
                    strcpy(epc_NG_arr[j], "");
                }
            } 

        }
        else {
            ESP_LOGE(TAG, "Unable to connect socket...socket = %d, isConnectedToServer = %u !\n", sock, isConnectedToServer);
            isConnectedToServer = 0;
            Reconnect_To_Socket();
            socket_CARD temp_socket_cards2;
            while(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &temp_socket_cards2, 0) == pdTRUE);
        }

        vTaskDelay(80 / portTICK_PERIOD_MS);
    }

}
#endif //TCP_OLD_TU
#ifdef TCP_NEW_TU_CO_RECEIVE
static void TCP_IP_CLIENT_SEND_task(void *arg)  {
    static const char *TCP_IP_CLIENT_SEND_TAG = "TCP_IP_CLIENT_SEND_TASK";
    esp_log_level_set(TCP_IP_CLIENT_SEND_TAG, ESP_LOG_INFO);
    TickType_t tickCount = xTaskGetTickCount();
    #define MAX_EPC_BATCH 10  // Số lượng mã EPC tối đa trong một cụm gửi
            char epcBatch[MAX_EPC_BATCH][20];  // Bộ đệm chứa các mã EPC
            int epcCount = 0;  // Đếm số lượng mã thẻ đã lưu
    while (1) {
        //Kiểm tra trạng thái socket, gửi lên màn hình
        socketStatus.status = isConnectedToServer;
        xQueueOverwrite(x_Socket_2_LCD_Task_Queue, &socketStatus);
        //Kiếm tra tình trạng socket có hợp lệ, có kết nối tới server không
        if((sock != -1) && (isConnectedToServer != 0)) {
            int err = 0;
            //Kiểm tra xem hàng đợi x_polling_UHF_RFID_2_Server_Queue có hợp lệ và nhận dữ liệu từ hàng đợi.
            if (x_polling_UHF_RFID_2_Server_Queue != NULL) {
                if(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &socket_cards, 10) == pdTRUE) 
                {                                           

                    /**/    
                    //[Trung]New code to Server      
                    //Kiểm tra xem có thẻ nào đã được quét hay không.   
                    if(socket_cards.result_cards > 0) 
                    {

                        //Khởi tạo mảng ký tự để lưu trữ kết quả và EPC (Electronic Product Code) của thẻ
                        //Chi tiết: resultPoll có thể được dùng để lưu trữ kết quả từ server, và strEpc để lưu EPC của thẻ quét được.
                        // char resultPoll[512];
                        char strEpc[20];

                        //Sao chép EPC từ cấu trúc socket_cards vào strEpc.
                        sprintf(strEpc,"%s",socket_cards.epc);

                        // printf("Socket: Check before Send to Server 0 - String:    %s\n", strEpc);

                        //Thẻ xe bị ghi ký tự lạ ở đuôi
                        if(strEpc[0] == 'L') {
                            strEpc[5] = '\0';
                        }
                        // //Đảm bảo chuỗi strEpc được kết thúc đúng.
                        // Giới hạn chuỗi chỉ còn 8 ký tự
                        strEpc[8] = '\0';  // Đảm bảo chuỗi kết thúc đúng vị trí
                        
                        vTaskDelay(20 / portTICK_PERIOD_MS);

                        //Lọc các thẻ đã gửi trước đó
                        if(TCP_filterCardInfo(strEpc) == false) continue;

                        // Lưu mã thẻ vào bộ đệm
                        strcpy(epcBatch[epcCount++], strEpc);
                        if (epcCount >= MAX_EPC_BATCH) epcCount = MAX_EPC_BATCH;  // Giới hạn không vượt quá bộ đệm
                                // Đợi 2 giây để gom các mã thẻ
                        if (epcCount > 0) {
                            vTaskDelay(20 / portTICK_PERIOD_MS);  // Độ trễ 2 giây
                        
                     }



                        // // strEpc[socket_cards.epc_len] = '\0';
                        // printf("Socket: Check before Send to Server - String:    %s\n", strEpc);

                        // // Gửi chuỗi 8 ký tự + dấu xuống dòng
                        // err = send(sock, strEpc, 9, 0);  // 8 ký tự + 1 ký tự xuống dòng
                        ESP_LOGI(TAG, "\nThoi gian luc gui: %ld\n",xTaskGetTickCount());
                        uint16_t soLanNhan = 0;
                        // if (err < 0) {
                        //     ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                        // }
                        // else if (err > 0) {
                        //     ESP_LOGI("TAG", "Message sent to server\n");
                        // }

                        /**/    
                        //Nhận dữ liệu từ server    
                        //Tính năng: Khởi tạo các biến cần thiết cho việc nhận dữ liệu từ server.
                        char rx_buffer[512];
                        uint8_t rx_check = 0;       // 0 - No response || 1 - Receive OK || 2 - Receive NG || 3- Duplicated card
                        //Lưu thời gian hiện tại để theo dõi thời gian đã trôi qua trong vòng lặp sau.
                        uint32_t checkDuplicateCardTime = xTaskGetTickCount();
                        //Bắt đầu một vòng lặp kiểm tra trong khoảng thời gian 800ms. 
                        //xTaskGetTickCount(): Hàm này trả về số ticks (đơn vị thời gian) từ khi FreeRTOS bắt đầu chạy. 
                        //Vòng lặp sẽ tiếp tục cho đến khi thời gian đã trôi qua lớn hơn 800ms.
                        while(xTaskGetTickCount() - checkDuplicateCardTime <= 3000/ portTICK_PERIOD_MS) 
                        {
                            int len = 0;

                            //Đặt lại (reset) nội dung của bộ đệm nhận rx_buffer.
                            for(uint16_t i = 0; i < 512; i++) {
                                rx_buffer[i] = '\0';
                            }

                            len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, MSG_DONTWAIT); 
                            soLanNhan++;
                            //
                            // rx_buffer
                            // len
                            //       
                            // Error occurred during receiving
                            if (len < 0) {
                                // ESP_LOGE(TAG, "recv failed: errno %d", errno);
                                // isConnectedToServer = 0;
                            }
                            // Data received
                            else if(len >0) {
                                rx_buffer[len] = '\0'; // Null-terminate whatever we received and treat like a string
                                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                                ESP_LOGI(TAG, "%s", rx_buffer);
                                
                                /* DinhTU: Check Server's Responde Data */
                                int MaxChamPhay = 0;
                                for(uint16_t i = 0; i < len; i ++) 
                                {
                                    //Duyệt qua từng ký tự trong rx_buffer để kiểm tra xem có chuỗi phản hồi [OK] từ server hay không.
                                    if (rx_buffer[i] == '[' && rx_buffer[i + 1] == 'O' && rx_buffer[i + 2] == 'K' && rx_buffer[i + 3] == ']') {
                                        /*Xu li UHF RFID deo tay*/
                                        MaxChamPhay= TachData(&rx_buffer[i]);

                                        if(MaxChamPhay > 0)
                                        {
                                            for(int CountChamPhay = 0; CountChamPhay < MaxChamPhay; CountChamPhay++) {
                                                if(x_Card_2_LCD_Task_Queue != NULL) {
                                                    xQueueSend(x_Card_2_LCD_Task_Queue, &wires[CountChamPhay], 10);
                                                    //Lưu lại vào mảng chống trùng lặp
                                                    strcpy((char*)checkUnDuplicateCardToServer[checkUnDuplicateCardToServer_Count++].epc, wires[CountChamPhay].serial);
                                                    printf("\nTCP: Dem bien count: %d\n", CountChamPhay);
                                                }
                                            }
                                            // Receive OK
                                            rx_check = 1;
                                        }

                                        
                                        /*Xu li UHF RFID xe hang*/


                                        break; //Thoát ra ngoài, không tìm thêm [OK] nữa           
                                    }
                                    /* DinhTU: `[NG]` */
                                    //Nếu tìm thấy [NG], cập nhật mảng epc_NG_arr để lưu thẻ không hợp lệ và ghi lại thông báo NG CARD!.
                                    else if (rx_buffer[i] == '[' && rx_buffer[i + 1] == 'N' && rx_buffer[i + 2] == 'G' && rx_buffer[i + 3] == ']') 
                                    {
                                        printf("NG CARD!\n");
                                        isEnoughSendMsg = 0;    //Cho phép tiếp tục gửi lại tin nhắn

                                        // Receive NG
                                        rx_check = 2;

                                        for(uint8_t check = 0; check < 20; check++) {
                                            if(strcmp(strEpc, epc_NG_arr[check]) == 0) {
                                                rx_check = 3;                              
                                                break;              //Đã gửi thẻ trước đó
                                            }
                                        }
                                        if(rx_check == 3) {
                                            break;
                                        }
                                        //Lưu thẻ mới vào mảng 
                                        strcpy(epc_NG_arr[socket_NG_cards_arr_count++], strEpc);    
                                        socket_NG_cards_arr_count = socket_NG_cards_arr_count % 20;

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

                                if (MaxChamPhay > 0)
                                {
                                    break; /* NoteTestSumi */
                                }
                                                       
                            }
                            //Nếu nhận được thông tin thẻ OK hoặc NG

                            if(rx_check == 1 || rx_check == 2) {
                                break;
                            }
                            vTaskDelay(10 / portTICK_PERIOD_MS);
                        }
                        ESP_LOGI(TAG, "\nThoi gian luc nhan: %ld va so lan nhan: %d\n",xTaskGetTickCount(), soLanNhan);
                        if(rx_check == 0) {
                            printf("NO RESPONSE!\n");
                            if(isEnoughSendMsg < 8){               //Mỗi tin nhắn gửi tối đa 8 lần
                                // xQueueSend(x_polling_UHF_RFID_2_Server_Queue, &socket_cards, 10);
                                isEnoughSendMsg++;
                            }                            
                        
                            //gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 1);
                            //vTaskDelay(1500 / portTICK_PERIOD_MS);
                            //gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 0); 
                        }
                    }
                }
                
                if(isGetNGCards == 1 && xTaskGetTickCount() - checkNGCardTime >= 1500 / portTICK_PERIOD_MS) {
                    checkNGCardTime = xTaskGetTickCount();
                    isGetNGCards = 0;
                    
                    //Mỗi 1.5s sẽ tiến hành reset buffer lọc thẻ trùng
                    for (int j = 0; j < 20; j++) {
                        strcpy(epc_NG_arr[j], "");
                    }
                }                 
            }   
                       if(((xTaskGetTickCount()-tickCount))>(2000/ portTICK_PERIOD_MS))  {           
                            // Tạo chuỗi chứa nhiều mã thẻ để gửi đi mà không có ký tự xuống dòng
                            char sendBuffer[512] = "";  // Bộ đệm gửi
                            printf("Socket: Send to Server, nums=%d\n", epcCount);
                            tickCount= xTaskGetTickCount();
                             if (epcCount > 0) {
                            for (int i = 0; i < epcCount; i++) {
                                strcat(sendBuffer, epcBatch[i]);  // Nối các mã EPC lại với nhau
                            }

                            // Gửi chuỗi các mã thẻ qua socket
                            err = send(sock, sendBuffer, strlen(sendBuffer), 0);
                            ESP_LOGI(TAG, "\nThoi gian luc gui: %ld\n", xTaskGetTickCount());  // Thời gian gửi

                            if (err < 0) {
                                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                            } else {
                                ESP_LOGI(TAG, "Batch of EPCs sent to server: %s", sendBuffer);
                            }

                            epcCount = 0;  // Xóa bộ đệm sau khi gửi
                        }
                    }

            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                isConnectedToServer = 0;
                Reconnect_To_Socket();
                socket_CARD temp_socket_cards1;
                while(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &temp_socket_cards1, 10) == pdTRUE);
            }
        }
           
        
        else {
            ESP_LOGE(TAG, "Unable to connect socket...socket = %d, isConnectedToServer = %u !\n", sock, isConnectedToServer);
            isConnectedToServer = 0;
            Reconnect_To_Socket();
            socket_CARD temp_socket_cards2;
            while(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &temp_socket_cards2, 10) == pdTRUE);
        }      
        vTaskDelay(80 / portTICK_PERIOD_MS);
    }

}

#endif //TCP_NEW_TU_CO_RECEIVE
#ifdef TCP_NEw_TRUNG
static void TCP_IP_CLIENT_SEND_task(void *arg)  {
    static const char *TCP_IP_CLIENT_SEND_TAG = "TCP_IP_CLIENT_SEND_TASK";
    esp_log_level_set(TCP_IP_CLIENT_SEND_TAG, ESP_LOG_INFO);
    #define MAX_EPC_BATCH 10  // Số lượng mã EPC tối đa trong một cụm gửi
    char epcBatch[MAX_EPC_BATCH][20];  // Bộ đệm chứa các mã EPC
    int epcCount = 0;  // Đếm số lượng mã thẻ đã lưu
    while (1) {
        //Kiểm tra trạng thái socket, gửi lên màn hình
        socketStatus.status = isConnectedToServer;
        xQueueOverwrite(x_Socket_2_LCD_Task_Queue, &socketStatus);
        //Kiếm tra tình trạng socket có hợp lệ, có kết nối tới server không
        if((sock != -1) && (isConnectedToServer != 0)) {
            int err = 0;
            //Kiểm tra xem hàng đợi x_polling_UHF_RFID_2_Server_Queue có hợp lệ và nhận dữ liệu từ hàng đợi.
            if (x_polling_UHF_RFID_2_Server_Queue != NULL) {
                if(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &socket_cards, 10) == pdTRUE) 
                {                                           

                    /**/    
                    //[Trung]New code to Server      
                    //Kiểm tra xem có thẻ nào đã được quét hay không.   
                    if(socket_cards.result_cards > 0) 
                    {
                        #ifdef SEND_TO_SERVER_OLD
                        //Khởi tạo mảng ký tự để lưu trữ kết quả và EPC (Electronic Product Code) của thẻ
                        //Chi tiết: resultPoll có thể được dùng để lưu trữ kết quả từ server, và strEpc để lưu EPC của thẻ quét được.

                        char strEpc[20];

                        //Sao chép EPC từ cấu trúc socket_cards vào strEpc.
                        sprintf(strEpc,"%s",socket_cards.epc);

                        // printf("Socket: Check before Send to Server 0 - String:    %s\n", strEpc);

                        //Thẻ xe bị ghi ký tự lạ ở đuôi
                        if(strEpc[0] == 'L') {
                            strEpc[5] = '\0';
                        }
                        // //Đảm bảo chuỗi strEpc được kết thúc đúng.
                        // Giới hạn chuỗi chỉ còn 8 ký tự
                        strEpc[8] = '\0';  // Đảm bảo chuỗi kết thúc đúng vị trí
                        
                        vTaskDelay(20 / portTICK_PERIOD_MS);

                        //Lọc các thẻ đã gửi trước đó
                        if(TCP_filterCardInfo(strEpc) == false) continue;

                        // strEpc[socket_cards.epc_len] = '\0';
                        printf("Socket: Check before Send to Server - String:    %s\n", strEpc);

                        // Gửi chuỗi 8 ký tự + dấu xuống dòng
                        err = send(sock, strEpc, 9, 0);  // 8 ký tự + 1 ký tự xuống dòng
                        ESP_LOGI(TAG, "\nThoi gian luc gui: %ld\n",xTaskGetTickCount());
                        uint16_t soLanNhan = 0;
                        if (err < 0) {
                            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                        }
                        else if (err > 0) {
                            ESP_LOGI("TAG", "Message sent to server\n");
                        }
                        #endif //SEND_TO_SERVER_OLD
                        #ifndef SEND_TO_SERVER_NEW
                        //Khởi tạo mảng ký tự để lưu trữ kết quả và EPC (Electronic Product Code) của thẻ
                        //Chi tiết: resultPoll có thể được dùng để lưu trữ kết quả từ server, và strEpc để lưu EPC của thẻ quét được.

                        char strEpc[20];

                        //Sao chép EPC từ cấu trúc socket_cards vào strEpc.
                        sprintf(strEpc,"%s",socket_cards.epc);

                        // printf("Socket: Check before Send to Server 0 - String:    %s\n", strEpc);

                        //Thẻ xe bị ghi ký tự lạ ở đuôi
                        if(strEpc[0] == 'L') {
                            strEpc[5] = '\0';
                        }
                        // //Đảm bảo chuỗi strEpc được kết thúc đúng.
                        // Giới hạn chuỗi chỉ còn 8 ký tự
                        strEpc[8] = '\0';  // Đảm bảo chuỗi kết thúc đúng vị trí
                        
                        vTaskDelay(20 / portTICK_PERIOD_MS);

                        //Lọc các thẻ đã gửi trước đó
                        if(TCP_filterCardInfo(strEpc) == false) continue;

                        // strEpc[socket_cards.epc_len] = '\0';
                        printf("Socket: Check before Send to Server - String:    %s\n", strEpc);

                        // Gửi chuỗi 8 ký tự + dấu xuống dòng
                        err = send(sock, strEpc, 9, 0);  // 8 ký tự + 1 ký tự xuống dòng
                        ESP_LOGI(TAG, "\nThoi gian luc gui: %ld\n",xTaskGetTickCount());
                        uint16_t soLanNhan = 0;
                        if (err < 0) {
                            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                        }
                        else if (err > 0) {
                            ESP_LOGI("TAG", "Message sent to server\n");
                        }
                        #endif //SEND_TO_SERVER_NEW
                        
                        /**/    
                        //Nhận dữ liệu từ server    
                        //Tính năng: Khởi tạo các biến cần thiết cho việc nhận dữ liệu từ server.
                        char rx_buffer[512];
                        uint8_t rx_check = 0;       // 0 - No response || 1 - Receive OK || 2 - Receive NG || 3- Duplicated card
                        //Lưu thời gian hiện tại để theo dõi thời gian đã trôi qua trong vòng lặp sau.
                        uint32_t checkDuplicateCardTime = xTaskGetTickCount();
                        //Bắt đầu một vòng lặp kiểm tra trong khoảng thời gian 800ms. 
                        //xTaskGetTickCount(): Hàm này trả về số ticks (đơn vị thời gian) từ khi FreeRTOS bắt đầu chạy. 
                        //Vòng lặp sẽ tiếp tục cho đến khi thời gian đã trôi qua lớn hơn 800ms.
                        while(xTaskGetTickCount() - checkDuplicateCardTime <= 3000/ portTICK_PERIOD_MS) 
                        {
                            int len = 0;

                            //Đặt lại (reset) nội dung của bộ đệm nhận rx_buffer.
                            for(uint16_t i = 0; i < 512; i++) {
                                rx_buffer[i] = '\0';
                            }

                            len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, MSG_DONTWAIT); 
                            soLanNhan++;
                            //
                            // rx_buffer
                            // len
                            //       
                            // Error occurred during receiving
                            if (len < 0) {
                                // ESP_LOGE(TAG, "recv failed: errno %d", errno);
                                // isConnectedToServer = 0;
                            }
                            // Data received
                            else if(len >0) {
                                rx_buffer[len] = '\0'; // Null-terminate whatever we received and treat like a string
                                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                                ESP_LOGI(TAG, "%s", rx_buffer);
                                
                                /* DinhTU: Check Server's Responde Data */
                                int MaxChamPhay = 0;
                                for(uint16_t i = 0; i < len; i ++) 
                                {
                                    //Duyệt qua từng ký tự trong rx_buffer để kiểm tra xem có chuỗi phản hồi [OK] từ server hay không.
                                    if (rx_buffer[i] == '[' && rx_buffer[i + 1] == 'O' && rx_buffer[i + 2] == 'K' && rx_buffer[i + 3] == ']') {
                                        /*Xu li UHF RFID deo tay*/
                                        MaxChamPhay= TachData(&rx_buffer[i]);

                                        if(MaxChamPhay > 0)
                                        {
                                            for(int CountChamPhay = 0; CountChamPhay < MaxChamPhay; CountChamPhay++) {
                                                if(x_Card_2_LCD_Task_Queue != NULL) {
                                                    xQueueSend(x_Card_2_LCD_Task_Queue, &wires[CountChamPhay], 10);
                                                    //Lưu lại vào mảng chống trùng lặp
                                                    strcpy((char*)checkUnDuplicateCardToServer[checkUnDuplicateCardToServer_Count++].epc, wires[CountChamPhay].serial);
                                                    printf("\nTCP: Dem bien count: %d\n", CountChamPhay);
                                                }
                                            }
                                            // Receive OK
                                            rx_check = 1;
                                        }

                                        
                                        /*Xu li UHF RFID xe hang*/


                                        break; //Thoát ra ngoài, không tìm thêm [OK] nữa           
                                    }
                                    /* DinhTU: `[NG]` */
                                    //Nếu tìm thấy [NG], cập nhật mảng epc_NG_arr để lưu thẻ không hợp lệ và ghi lại thông báo NG CARD!.
                                    else if (rx_buffer[i] == '[' && rx_buffer[i + 1] == 'N' && rx_buffer[i + 2] == 'G' && rx_buffer[i + 3] == ']') 
                                    {
                                        printf("NG CARD!\n");
                                        isEnoughSendMsg = 0;    //Cho phép tiếp tục gửi lại tin nhắn

                                        // Receive NG
                                        rx_check = 2;

                                        for(uint8_t check = 0; check < 20; check++) {
                                            if(strcmp(strEpc, epc_NG_arr[check]) == 0) {
                                                rx_check = 3;                              
                                                break;              //Đã gửi thẻ trước đó
                                            }
                                        }
                                        if(rx_check == 3) {
                                            break;
                                        }
                                        //Lưu thẻ mới vào mảng 
                                        strcpy(epc_NG_arr[socket_NG_cards_arr_count++], strEpc);    
                                        socket_NG_cards_arr_count = socket_NG_cards_arr_count % 20;

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

                                if (MaxChamPhay > 0)
                                {
                                    break; /* NoteTestSumi */
                                }
                                                       
                            }
                            //Nếu nhận được thông tin thẻ OK hoặc NG

                            if(rx_check == 1 || rx_check == 2) {
                                break;
                            }
                            vTaskDelay(10 / portTICK_PERIOD_MS);
                        }
                        ESP_LOGI(TAG, "\nThoi gian luc nhan: %ld va so lan nhan: %d\n",xTaskGetTickCount(), soLanNhan);
                        if(rx_check == 0) {
                            printf("NO RESPONSE!\n");
                            if(isEnoughSendMsg < 8){               //Mỗi tin nhắn gửi tối đa 8 lần
                                // xQueueSend(x_polling_UHF_RFID_2_Server_Queue, &socket_cards, 10);
                                isEnoughSendMsg++;
                            }                            
                        
                            //gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 1);
                            //vTaskDelay(1500 / portTICK_PERIOD_MS);
                            //gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 0); 
                        }
                    }
                }
                
                if(isGetNGCards == 1 && xTaskGetTickCount() - checkNGCardTime >= 1500 / portTICK_PERIOD_MS) {
                    checkNGCardTime = xTaskGetTickCount();
                    isGetNGCards = 0;
                    
                    //Mỗi 1.5s sẽ tiến hành reset buffer lọc thẻ trùng
                    for (int j = 0; j < 20; j++) {
                        strcpy(epc_NG_arr[j], "");
                    }
                }                 
            }   
          
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                isConnectedToServer = 0;
                Reconnect_To_Socket();
                socket_CARD temp_socket_cards1;
                while(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &temp_socket_cards1, 10) == pdTRUE);
            }
        }
        else {
            ESP_LOGE(TAG, "Unable to connect socket...socket = %d, isConnectedToServer = %u !\n", sock, isConnectedToServer);
            isConnectedToServer = 0;
            Reconnect_To_Socket();
            socket_CARD temp_socket_cards2;
            while(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &temp_socket_cards2, 10) == pdTRUE);
        }      
        vTaskDelay(80 / portTICK_PERIOD_MS);
    }

}

#endif //TCP_NEw_TRUNG
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
        memset(checkUnDuplicateCardToServer[i].epc, 0, 12);     
    }
    x_GPIO_UHF_RFID_2_Server_Queue = xQueueCreate( 100, sizeof(socket_CARD));
    //Queue gửi dữ liệu trở lại task UHF RFID để lưu những thẻ đọc đúng xe hàng
    x_Server_2_UHF_RFID_Task = xQueueCreate( 100, sizeof(socket_CARD));
    x_Socket_2_LCD_Task_Queue = xQueueCreate( 1, sizeof(sockettcp_info));

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

    xTaskCreate(&TCP_IP_CLIENT_SEND_task, "tcp_ip_client_send_task", 1024*10, NULL, configMAX_PRIORITIES - 1, NULL);

}
#endif

#ifndef TEST_UHF_RFID_WATCH
extern QueueHandle_t x_polling_UHF_RFID_2_Server_Queue;
extern QueueHandle_t x_reading_UHF_RFID_2_Server_Queue;
QueueHandle_t x_GPIO_UHF_RFID_2_Server_Queue;                   //Tạm thời không dùng
QueueHandle_t x_Server_2_UHF_RFID_Task;

/* FreeRTOS event group to communicate another task*/
extern EventGroupHandle_t main_eventGroup;
extern QueueHandle_t xParameterQueue;
esp32_parameter_TypeDef TCP_ESP32Parameter;

extern QueueHandle_t xToAudioQueue;

// char rx_buffer[128];
char host_ip[18];
uint16_t portValue;
int addr_family = 0;
int ip_protocol = 0;
struct sockaddr_in dest_addr;

int sock;
uint8_t isConnectedToServer = 0;

socket_CARD socket_cards;
uint8_t socket_read_buffer[SOCKET_READ_CARD_BUFFER_LENGTH] = {0};

static void Reconnect_To_Socket(void) {
    //Clear conncet
    // shutdown(sock, 0);
    // close(sock);
    // sock = -1;

    // sock =  socket(addr_family, SOCK_STREAM, ip_protocol);

    while(1) {
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
            shutdown(sock, 0);
            close(sock);
            sock = -1;
            xEventGroupClearBits(main_eventGroup, EVENT_BIT_SOCKETTASK_CONNECTED);
            xEventGroupSetBits(main_eventGroup, EVENT_BIT_SOCKETTASK_UNCONNECT);
            // printf("After reconnect sock = %d\n",sock);
        }
        else {
            ESP_LOGI("Reconnect task", "Socket: Successfully Reconnected");
            isConnectedToServer = 1;
            xEventGroupClearBits(main_eventGroup, EVENT_BIT_SOCKETTASK_UNCONNECT);
            xEventGroupSetBits(main_eventGroup, EVENT_BIT_SOCKETTASK_CONNECTED);
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

uint8_t isEnoughSendMsg = 0;           //Biến kiểm soát số lần gửi để server

static void TCP_IP_CLIENT_SEND_task(void *arg)  {
    static const char *TCP_IP_CLIENT_SEND_TAG = "TCP_IP_CLIENT_SEND_TASK";
    esp_log_level_set(TCP_IP_CLIENT_SEND_TAG, ESP_LOG_INFO);

    while (1) {
        if((sock != -1) && (isConnectedToServer != 0)) {
            int err = 0;
            // int err = send(sock, payload, strlen(payload), 0);    
            if (x_polling_UHF_RFID_2_Server_Queue != NULL) {
                if(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &socket_cards, 10) == pdTRUE) {                                           
                // printf("pc: %s\n", socket_cards.pc_str);
                // printf("rssi: %s\n", socket_cards.rssi_str);
                // printf("epc: %s\n", socket_cards.epc_str);
                // printf("-----------------\n");
                
                if(socket_cards.result_cards > 0) {
                    // gpio_set_level(CONFIG_GPIO_OUTPUT_GREEN_LED, 0);
                    // // uint8_t tempAudio = 1;
                    // // xQueueSend(xToAudioQueue, &tempAudio, 10);
                    // vTaskDelay(150 / portTICK_PERIOD_MS);
                    // gpio_set_level(CONFIG_GPIO_OUTPUT_GREEN_LED, 1);

                    char resultPoll[512];
                    char strEpc[20];

                    // char temp_rx[100];
                    // // Clear the receive buffer before sending data
                    // recv(sock, temp_rx, sizeof(temp_rx) - 1, MSG_DONTWAIT);
                    
                    // printf("Socket: Check before Send to Server - Hex:");
                    // for(uint8_t i = 0; i < 12; i++) {
                    //     printf("    %X", socket_cards.epc[i]);
                    // }
                    // printf("\n");
                        
                    // for(uint8_t i = 0; i < strlen((char*)socket_cards.epc); i++) {
                    //     strEpc[i] = socket_cards.epc[i];
                    // }
                    // strEpc[12] = '\0';
                    sprintf(strEpc,"%s",socket_cards.epc);

                    //Thẻ xe bị ghi ký tự lạ ở đuôi
                    if(strEpc[0] == 'C') {
                        strEpc[4] = '\0';
                    }

                    // //Lọc thẻ trùng
                    // if(strcmp(strEpc, old_epc) == 0) {
                    //     continue;
                    // }
                    // else {
                    //     strcpy(old_epc, strEpc);
                    // }
                    strEpc[socket_cards.epc_len] = '\0';
                    printf("Socket: Check before Send to Server - String:    %s\n", strEpc);
                        
                    sprintf(resultPoll, "----------------- \nPoll card \npc: %s \nrssi: %s  \nepc: %s\n -----------------\n", socket_cards.pc_str, socket_cards.rssi_str, strEpc);

                    err = send(sock, strEpc, strlen(strEpc), 0);

                    if (err < 0) {
                        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    }
                    else if (err > 0) {
                        ESP_LOGI("TAG", "Message sent to server\n");
                    }

                    char rx_buffer[512];
                    uint8_t rx_check = 0;       // 0 - No response || 1 - Receive OK || 2 - Receive NG || 3- Duplicated card
                    uint32_t checkDuplicateCardTime = xTaskGetTickCount();
                    while(xTaskGetTickCount() - checkDuplicateCardTime <= 800/ portTICK_PERIOD_MS) {
                    int len = 0;

                    //Reset buffer
                    for(uint16_t i = 0; i < 512; i++) {
                        rx_buffer[i] = '\0';
                    }

                    len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, MSG_DONTWAIT);        
                    // Error occurred during receiving
                    if (len < 0) {
                        // ESP_LOGE(TAG, "recv failed: errno %d", errno);
                        // isConnectedToServer = 0;
                    }
                    // Data received
                    else if(len >0) {
                        rx_buffer[len] = '\0'; // Null-terminate whatever we received and treat like a string
                        ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                        ESP_LOGI(TAG, "%s", rx_buffer);
                        
                        /* DinhTU: Check Server's Responde Data */
                        for(uint16_t i = 0; i < len; i ++) {
                            /* DinhTU: `[OK]` */
                            if (rx_buffer[i] == '[' && rx_buffer[i + 1] == 'O' && rx_buffer[i + 2] == 'K' && rx_buffer[i + 3] == ']') {
                                //////////////////////////////////////
                                //Lưu thẻ mới vào mảng 
                                            // strcpy(epc_arr[socket_cards_arr_count++], strEpc);    
                                            // socket_cards_arr_count = socket_cards_arr_count % 20;
                                            // xQueueSend(x_Server_2_UHF_RFID_Task, &socket_cards, 10);
                                ///////////////////////////////
                                for(uint16_t j = i; j < i + 45; j ++) {
                                    /* DinhTU: `Tag:` */
                                    if(rx_buffer[j] == 'T' && rx_buffer[j + 1] == 'a' && rx_buffer[j + 2] == 'g' && rx_buffer[j + 3] == ':') {

                                        // ESP_LOGI(TAG, "Nhan the day hang!");

                                        /* DinhTU: check ECP */
                                        if(strcmp(strEpc, &rx_buffer[j + 5]) == 0) {
                                            //Buffer phản hồi trùng với Epc của thẻ được gửi đi
                                            xEventGroupClearBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_DETECTED_NOCARD);
                                            xEventGroupClearBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_POLLED_A_WRONGCARD);
                                            xEventGroupSetBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_POLLED_A_CARD);

                                            // AUDIO_select_and_play_track(1);
                                            // vTaskDelay(pdMS_TO_TICKS(500)); // Đợi 5 giây để bài hát bắt đầu phát
                                            /* DinhTU: Sound Success */        
                                            uint8_t tempAudio = 1;
                                            xQueueSend(xToAudioQueue, &tempAudio, 10);

                                            isEnoughSendMsg = 0;        //Cho phép tiếp tục gửi lại tin nhắn

                                            // Receive OK
                                            rx_check = 1;
                                            for(uint8_t check = 0; check < 20; check++) {
                                                if(strcmp(strEpc, epc_arr[check]) == 0) {
                                                    rx_check = 3;
                                                    break;              //Đã gửi thẻ trước đó
                                                }
                                            }

                                            if(rx_check == 3) {
                                                break;
                                            }

                                            //Lưu thẻ mới vào mảng 
                                            strcpy(epc_arr[socket_cards_arr_count++], strEpc);    
                                            socket_cards_arr_count = socket_cards_arr_count % 20;
                                            xQueueSend(x_Server_2_UHF_RFID_Task, &socket_cards, 10);

                                            ///////////////////////////////////////////////////////
                                            // printf("\nLOG after send back to RFID Task: \n EPC_Length:%d\n", socket_cards.epc_len);
                                            // for(uint8_t LOG_i = 0; LOG_i <socket_cards.epc_len; LOG_i++) {
                                            //     printf("%X ", socket_cards.epc[LOG_i]);
                                            // }
                                            // printf("\n");
                                            ////////////////////////////////////////////////////////

                                            // gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 1);
                                            // vTaskDelay(200 / portTICK_PERIOD_MS);
                                            // gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 0);
                                            // vTaskDelay(50 / portTICK_PERIOD_MS);


                                            printf("OK CARD!\n");

                                            break; //Break chưa thoát khỏi vòng while wait-time to receive
                                                
                                        }
                                    }
                                    /* DinhTU: `Received CartNo` */
                                    else if(rx_buffer[j] == 'R' && rx_buffer[j + 1] == 'e' && rx_buffer[j + 2] == 'c' && rx_buffer[j + 3] == 'e' && rx_buffer[j + 4] == 'i' && rx_buffer[j + 5] == 'v' && rx_buffer[j + 6] == 'e' && rx_buffer[j + 7] == 'd' && rx_buffer[j + 9] == 'C' && rx_buffer[j + 10] == 'a' && rx_buffer[j + 11] == 'r' && rx_buffer[j + 12] == 't' && rx_buffer[j + 13] == 'N' && rx_buffer[j + 14] == 'o') { 
                                        if(strcmp(strEpc, &rx_buffer[j + 17]) == 0) {
                                            //Buffer phản hồi trùng với Epc của thẻ được gửi đi
                                            xEventGroupClearBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_DETECTED_NOCARD);
                                            xEventGroupClearBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_POLLED_A_WRONGCARD);
                                            xEventGroupSetBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_POLLED_A_CARD);
                                            
                                            //  ESP_LOGI(TAG, "Nhan the xe hang!");
                                            uint8_t tempAudio = 1;
                                            xQueueSend(xToAudioQueue, &tempAudio, 10);

                                            isEnoughSendMsg = 0;        //Cho phép tiếp tục gửi lại tin nhắn
                                            // Receive OK
                                            rx_check = 1;
                                            for(uint8_t check = 0; check < 20; check++) {
                                                if(strcmp(strEpc, epc_arr[check]) == 0) {
                                                    rx_check = 3;
                                                    break;              //Đã gửi thẻ trước đó
                                                    }
                                            }

                                            if(rx_check == 3) {
                                                break;
                                            }
                                            //Lưu thẻ mới vào mảng 
                                            strcpy(epc_arr[socket_cards_arr_count++], strEpc);    
                                            socket_cards_arr_count = socket_cards_arr_count % 20;
                                            xQueueSend(x_Server_2_UHF_RFID_Task, &socket_cards, 10);


                                            // printf("OK CARD!\n");

                                            break; //Break chưa thoát khỏi vòng while wait-time to receive
                                                
                                        }
                                    }                                                  
                                }
                                            
                            }
                            /* DinhTU: `[NG]` */
                            else if (rx_buffer[i] == '[' && rx_buffer[i + 1] == 'N' && rx_buffer[i + 2] == 'G' && rx_buffer[i + 3] == ']') {
                                xEventGroupClearBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_DETECTED_NOCARD);
                                xEventGroupClearBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_POLLED_A_CARD);
                                xEventGroupSetBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_POLLED_A_WRONGCARD);

                                // gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 1);
                                // vTaskDelay(1500 / portTICK_PERIOD_MS);
                                // gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 0);

                                uint8_t tempAudio = 2;
                                xQueueSend(xToAudioQueue, &tempAudio, 10);

                                printf("NG CARD!\n");
                                isEnoughSendMsg = 0;    //Cho phép tiếp tục gửi lại tin nhắn

                                // Receive NG
                                rx_check = 2;

                                for(uint8_t check = 0; check < 20; check++) {
                                    if(strcmp(strEpc, epc_NG_arr[check]) == 0) {
                                        rx_check = 3;                              
                                        break;              //Đã gửi thẻ trước đó
                                    }
                                }
                                if(rx_check == 3) {
                                    break;
                                }
                                //Lưu thẻ mới vào mảng 
                                strcpy(epc_NG_arr[socket_NG_cards_arr_count++], strEpc);    
                                socket_NG_cards_arr_count = socket_NG_cards_arr_count % 20;

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
                    }
                            //Nếu nhận được thông tin thẻ OK hoặc NG
                            if(rx_check == 1 || rx_check == 2) {
                                break;
                            }
                        }
                        if(rx_check == 0) {
                            printf("NO RESPONSE!\n");
                            if(isEnoughSendMsg < 8){               //Mỗi tin nhắn gửi tối đa 8 lần
                                // xQueueSend(x_polling_UHF_RFID_2_Server_Queue, &socket_cards, 10);
                                isEnoughSendMsg++;
                            }                            
                        
                            //gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 1);
                            //vTaskDelay(1500 / portTICK_PERIOD_MS);
                            //gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 0); 
                        }
                    }
                }

                if(isGetNGCards == 1 && xTaskGetTickCount() - checkNGCardTime >= 1500 / portTICK_PERIOD_MS) {
                    checkNGCardTime = xTaskGetTickCount();
                    isGetNGCards = 0;
                    
                    //Mỗi 1.5s sẽ tiến hành reset buffer lọc thẻ trùng
                    for (int j = 0; j < 20; j++) {
                        strcpy(epc_NG_arr[j], "");
                    }
                }                 
            }   
          
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                isConnectedToServer = 0;
                Reconnect_To_Socket();
                socket_CARD temp_socket_cards1;
                while(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &temp_socket_cards1, 10) == pdTRUE);
            }
        }
        else {
            ESP_LOGE(TAG, "Unable to connect socket...socket = %d, isConnectedToServer = %u !\n", sock, isConnectedToServer);
            isConnectedToServer = 0;
            Reconnect_To_Socket();
            socket_CARD temp_socket_cards2;
            while(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &temp_socket_cards2, 10) == pdTRUE);
        }      
        vTaskDelay(80 / portTICK_PERIOD_MS);
    }

}

static void TCP_IP_CLIENT_RECV_task(void *arg) {
    static const char *TCP_IP_CLIENT_RECV_TAG = "TCP_IP_CLIENT_RECV_TASK";
    esp_log_level_set(TCP_IP_CLIENT_RECV_TAG, ESP_LOG_INFO);

    // char rx_buffer[512];
    while(1) {
        // if (sock != -1 && isConnectedToServer == 1) {           
        //     int len = 0;
        //     len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);        
        //     // Error occurred during receiving
        //     if (len < 0) {
        //         ESP_LOGE(TAG, "recv failed: errno %d", errno);
        //         isConnectedToServer = 0;
        //     }
        //     // Data received
        //     else if(len >0) {
        //         rx_buffer[len] = '\0'; // Null-terminate whatever we received and treat like a string
        //         ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
        //         ESP_LOGI(TAG, "%s", rx_buffer);
        //         if(rx_buffer[1] == 'O' && rx_buffer[2]=='K') {
        //             xEventGroupClearBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_DETECTED_NOCARD);
        //             xEventGroupClearBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_POLLED_A_WRONGCARD);
        //             xEventGroupSetBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_DETECTED_NOCARD);

        //             gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 1);
        //             vTaskDelay(200 / portTICK_PERIOD_MS);
        //             gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 0);
        //             vTaskDelay(100 / portTICK_PERIOD_MS);
        //         }
        //         else if(rx_buffer[1] == 'N' && rx_buffer[2]=='G') {
        //             xEventGroupClearBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_DETECTED_NOCARD);
        //             xEventGroupClearBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_POLLED_A_CARD);
        //             xEventGroupSetBits(main_eventGroup, EVENT_BIT_UHFRFIDTASK_POLLED_A_WRONGCARD);

        //             gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 1);
        //             vTaskDelay(1500 / portTICK_PERIOD_MS);
        //             gpio_set_level(CONFIG_GPIO_OUTPUT_HORN, 0);
        //         }
        //     }
        // }
        // ******************************************************************************
        // if(xQueueReceive(x_polling_UHF_RFID_2_Server_Queue, &socket_cards, 10) != NULL) {

        // }


        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


static void Receive_UHF_RFID_Queue_task(void *arg)  {
    static const char *RECEIVE_UHF_RFID_QUEUE_TAG = "RECEIVE_UHF_RFID_QUEUE_TASK";
    esp_log_level_set(RECEIVE_UHF_RFID_QUEUE_TAG, ESP_LOG_INFO);

    while (1) {
        // xQueuePeek(xParameterQueue, &TCP_ESP32Parameter, portMAX_DELAY);
        // // = 1 by menuconfig = 2 by software                                               
        // if(CONFIG_ESP32_UHF_RFID_DEVICE_CONFIG == 1) {
        //     strcpy(host_ip,HOST_IP_ADDR); 
        //     portValue = PORT;
        // }
        // else {
        //     strcpy(host_ip,TCP_ESP32Parameter.server_IP);  
        //     portValue = TCP_ESP32Parameter.server_Port;
        // }
 
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}


void TCP_Init(void) {
    x_GPIO_UHF_RFID_2_Server_Queue = xQueueCreate( 100, sizeof(socket_CARD));
    //Queue gửi dữ liệu trở lại task UHF RFID để lưu những thẻ đọc đúng xe hàng
    x_Server_2_UHF_RFID_Task = xQueueCreate( 100, sizeof(socket_CARD));

    xQueuePeek(xParameterQueue, &TCP_ESP32Parameter, portMAX_DELAY);

    // = 1 by menuconfig = 2 by software                                               
    if(CONFIG_ESP32_UHF_RFID_DEVICE_CONFIG == 1){
        strcpy(host_ip,HOST_IP_ADDR); 
        portValue = PORT;
    }
    else {
        strcpy(host_ip,TCP_ESP32Parameter.server_IP);  
        portValue = TCP_ESP32Parameter.server_Port;
    }
     

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
        xEventGroupClearBits(main_eventGroup, EVENT_BIT_SOCKETTASK_CONNECTED);
        xEventGroupSetBits(main_eventGroup, EVENT_BIT_SOCKETTASK_UNCONNECT);
    }
    else {
        ESP_LOGI(TAG, "Socket: Successfully connected");
        isConnectedToServer = 1;
        xEventGroupClearBits(main_eventGroup, EVENT_BIT_SOCKETTASK_UNCONNECT);
        xEventGroupSetBits(main_eventGroup, EVENT_BIT_SOCKETTASK_CONNECTED);
    }
    ESP_LOGW(TAG,"TCP_Init sock = %d\n",sock);

    xTaskCreate(&TCP_IP_CLIENT_SEND_task, "tcp_ip_client_send_task", 1024*10, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(&TCP_IP_CLIENT_RECV_task, "tcp_ip_client_receive_task", 1024*2, NULL, configMAX_PRIORITIES - 5, NULL);
    xTaskCreate(&Receive_UHF_RFID_Queue_task, "socket_receive_task", 1024*5, NULL, configMAX_PRIORITIES - 5, NULL);

}

#endif

