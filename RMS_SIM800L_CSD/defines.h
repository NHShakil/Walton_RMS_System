
//// BLE Test Code
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;


#include "driver/uart.h"
#include "UART.h"
#include "Http.h"
#include <Wire.h>
#define WIRE Wire
#define DEV_ID 1
#define EE_CHECK_SUM 0
#define ODU_RSPNS_PAC_SIZE 38
#define MODEM Serial2
#define MODEM_BAUD_RATE 9600
#define UART1_ODU_BAUD_RATE 2400
#define DEBUG SerialBT
//#define DEBUG Serial
#define DEBUG_BAUD_RATE 9600
#define U1RXD 33
#define U1TXD 32
#define ADDR_Ax 0b000  //A2, A1, A0
#define ADDR (0b1010 << 3) + ADDR_Ax
#define ADDR_ONE 0x50
#define ADDR_TWO 0x51


// GLOBAL VARIABL
int oduData[ODU_RSPNS_PAC_SIZE];
int mode = 0;
int eePortType = 0;
int IDU_EE_RD_FLG = 0;
int IDU_EE_WR_FLG = 0;
int ODU_EE_RD_FLG = 0;
int ODU_EE_WR_FLG = 0;
// APN information for GPRS  Communication
String apn = "internet";
String apn_u = "";
String apn_p = "";

// Server Infors
//String PROJECT_PATH = "/ac_monitoring/controller/devlogs.php/?data="+DEV_ID+","+MOB_NO+","+EE_CHECK_SUM;
String SRV_IP = "http://103.243.142.11";
String PROJECT_NAME = "/walton_rcrms/";
String PROJECT_PATH = "/walton_rcrms/controller/devlogs.php/?data=";
String PRAM = "";
String URI = SRV_IP + PROJECT_PATH;
String URL = URI;
String srvResPns = "";
String sigLvl = "0";
String url_pram = "";
String ODU_PAC_ONE = "";
String ODU_PAC_TWO = "";
String ODU_PAC_THREE = "";
String IDU_EE_DATA = "";
String ODU_EE_DATA = "";
String MOB_NO = "01608984560";

short chkData = 0, arr_counter = 256;
int chkSum = 0;

byte compModel[5] = { 0xAA, 0x05, 0x00, 0xFB, 0x55 };