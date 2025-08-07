#ifndef __TCP_CLIENT_V4_H__
#define __TCP_CLIENT_V4_H__

#include "ESP32_Infor.h"

#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>            // struct addrinfo
#include <arpa/inet.h>
#include "esp_netif.h"

#define TEST_UHF_RFID_WATCH
#ifdef TEST_UHF_RFID_WATCH


#define SOCKET_READ_CARD_BUFFER_LENGTH 4

#define SOCKET_CHECK_DUPLICATE_SIZE 15

#define MAX_ENTRIES 20
#define MAX_FIELDS 20  
#define MAX_LENGTH 35

#define SIZE_OF_RX_TCP_BUFFER 1024

#define PROCESS_CODE "SA01"
#define PROCESS_CODE_CL "CL02"

typedef struct {
    uint8_t epc[12];
} socket_EPC_CARD;



int findSemicolonPosition(const char *str, const char ch);
int TachData(char DataFromServer[]);
void TCP_Init(void);
void CopyBuffToStruct (wire_infor *wires, char Buffer[MAX_FIELDS][MAX_LENGTH]);
void PrintWiresInfo(wire_infor *wires, int numEntries);
uint8_t TCP_filterCardInfo(char* epc);
extern QueueHandle_t xQueueLocation;
#endif

#endif