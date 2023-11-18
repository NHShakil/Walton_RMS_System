#include "driver/uart.h"
#include "UART.h"
#include "Http.h"
#include <Wire.h>
#define WIRE Wire
#define DEV_ID 1
#define MOB_NO 00000000000
//#define SERIAL_CMD 0
//#define EE_CMD 0
#define EE_CHECK_SUM 0


#define ODU_DATA_SIZE 38






// UART Section
#define DEBUG Serial
#define DEBUG_BAUD_RATE 9600

#define MODEM Serial2
#define MODEM_BAUD_RATE 9600

#define UART1_ODU_BAUD_RATE 2400
#define U1RXD 32
#define U1TXD 33

// EE Program Uploading Variable Secttion
//#define ADDR_Ax 0b000 //A2, A1, A0
//#define ADDR (0b1010 << 3) + ADDR_Ax
#define ADDR_ONE 0x50
#define ADDR_TWO 0x51
//#define ADDR_ONE 0x52
//#define ADDR_TWO 0x53
//#define IDU_ARR_LENGTH 255
//#define ODU_ARR_LENGTH 512


// GLOBAL VARIABL

int oduData[ODU_DATA_SIZE];
String url_pram = "";
bool rd_buff = false;


// Internet Communication Informations
String apn = "internet";
String apn_u = "";
String apn_p = "";
// Server Infors
String SRV_IP = "http://103.243.142.11";
//String PROJECT_PATH = "/ac_monitoring/controller/devlogs.php/?data="+DEV_ID+","+MOB_NO+","+EE_CHECK_SUM;
String PROJECT_PATH = "/ac_monitoring/controller/devlogs.php/?data={";
String PRAM = "";
String Data = "";
String ODU_PAC_ONE = "";
String ODU_PAC_TWO = "";

const int ledPin = 26;
short chkSum = 0, chkData = 0, arr_counter = 256;
