#include "driver/uart.h"
#include "UART.h"
#include "Http.h"

// UART Section
#define DEBUG Serial

#define DEBUG_BAUD_RATE 115200
#define UART1_ODU_BAUD_RATE 2400
#define U1RXD 32
#define U1TXD 33

#define MODEM Serial2
#define MODEM_BAUD_RATE 9600


// GLOBAL VARIABL
#define ODU_DATA_SIZE 38
int oduData[ODU_DATA_SIZE];
String url_pram = "";
bool rd_buff = false;

String apn = "internet";
String apn_u = "";
String apn_p = "";

String SRV_IP = "http://103.243.142.11";
String PROJECT_PATH = "/ac_monitoring/controller/devlogs.php/?data=";
String PRAM = "";
String Data = "";
