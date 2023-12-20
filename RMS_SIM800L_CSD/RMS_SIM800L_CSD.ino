#include "defines.h"
String URI = SRV_IP + PROJECT_PATH;
String URL = URI;
String buf = "";
int EE_Data[] = {};
String MobNo = "";
String MobNoTemp = "";
String IDU_EE_DATA = "";
String ODU_EE_DATA = "";
int memCount = 0;
#define ADDR_Ax 0b000  //A2, A1, A0
#define ADDR (0b1010 << 3) + ADDR_Ax
void setup() {
  DEBUG.begin(DEBUG_BAUD_RATE);
  // UART2 Configuration
  MODEM.begin(MODEM_BAUD_RATE);
  Wire.begin();
  // UART1 Configuration
  uart_config_t Configurazione_UART1 = {
    .baud_rate = UART1_ODU_BAUD_RATE,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_EVEN,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_APB,
  };
  uart_param_config(NUMERO_PORTA_SERIALE, &Configurazione_UART1);
  esp_log_level_set(TAG, ESP_LOG_INFO);
  uart_set_pin(NUMERO_PORTA_SERIALE, U1TXD, U1RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(NUMERO_PORTA_SERIALE, BUF_SIZE, BUF_SIZE, 20, &uart1_queue, 0);
  xTaskCreate(UART_ISR_ROUTINE, "UART_ISR_ROUTINE", 2048, NULL, 12, NULL);
  //vTaskDelete(NULL);
  //Configurations
  test_sim800_module();
  delay(1000);
  gsm_config_gprs();
  delay(1000);
}

void loop() {
  //DEBUG.println(URL);
  //DEBUG.println(URL + DEV_ID + ";" +  MOB_NO + ";"  +  ODU_PAC_ONE + ";" + ODU_PAC_TWO + ";" + IDU_EE_DATA + ";" + ODU_EE_DATA);
  Send_GET_Rqst(URL + DEV_ID + ";" +  MOB_NO + ";"  + ";" + ODU_PAC_ONE + ODU_PAC_TWO + ";" + IDU_EE_DATA + ";" + ODU_EE_DATA, "0");
  DEBUG.println("**********************  START  ********************** ");
  DEBUG.print("[MLOP: ]");
  DEBUG.println(buf);
  explode(",", buf);
  //read_data();
  DEBUG.println("**********************   END   ********************** ");
  delay(1000);
  buf = "";
}
void explode(char delim[], String rcv_Str) {
  //IDU,0,128,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,25555,255,255,255,255,255,255
  // Byte_0               = First segment is Dummy
  // Byte_1               = Start of Address of EEPROM
  // Byte_02              = Total number of Element to be write
  // Byte_03_to_Byte_02+1 = EE Data 
  // Byte_Last            = Summation of Byte_03_to_Byte_02+1

  int str_len = rcv_Str.length() + 1;
  int a = 0, c = 0;
  char char_array[str_len], rslt_Arr[str_len];
  rcv_Str.toCharArray(char_array, str_len);
  char *ptr = strtok(char_array, delim);
  int decimalVal = 0;
  int strt_pos = 0, totl_Elmnt = 0;
  DEBUG.println("-----------> Start Exploding: ");
  while (ptr != NULL) {
    int strLen = strlen(ptr);
    for (int i = 0; i < strLen; i++) {
      decimalVal += (ptr[i] - '0') * pow(10, (strLen - 1 - i));
    }
    if (a > 0) {
      if (a == 1) {
        strt_pos = decimalVal;
        DEBUG.print("\t STRT POS : ");
        DEBUG.print(strt_pos);
      } else if (a == 2) {
        totl_Elmnt = decimalVal + 3;
        DEBUG.print("\t TOTL ELM : ");
        DEBUG.println(totl_Elmnt);
        //a = strt_pos;
      } else if ((a > 2) && (a <= totl_Elmnt)) {
        //totl_Elmnt = decimalVal;
        //DEBUG.print("\t TOTL ELM : "); DEBUG.println(totl_Elmnt);
        //strt_pos;
        //Serial.print("RX DATA : "); Serial.print(ptr);
        //DEBUG.print(" Val of A : "); DEBUG.print(strt_pos);
        DEBUG.print(" Val A : ");
        DEBUG.print(a);
        DEBUG.print("\t ADDR : ");
        DEBUG.print(strt_pos);
        DEBUG.print("\t DECM : ");
        DEBUG.println(decimalVal);
        //writeI2CByte(strt_pos, decimalVal);
        //EE_Data[c] = decimalVal;
        strt_pos++;
      }
    }
    decimalVal = 0;
    ptr = strtok(NULL, delim);
    a++;
    c++;
  }
}
static void UART_ISR_ROUTINE(void *pvParameters)                //uart1
{
  uart_event_t event;
  size_t buffered_size;
  bool exit_condition = false;
  String ODU_PAC_ONE_Temp = "";
  String ODU_PAC_TWO_Temp = "";
  ODU_PAC_ONE = "";
  ODU_PAC_TWO = "";
  while (1) {
    if (xQueueReceive(uart1_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
      if (event.type == UART_DATA) {
        uint8_t UART1_data[128];
        int UART1_data_length = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_1, (size_t*)&UART1_data_length));
        UART1_data_length = uart_read_bytes(UART_NUM_1, UART1_data, UART1_data_length, 100);
        if (UART1_data_length == ODU_DATA_SIZE) {
          if (UART1_data[0] == 170) {

            for (byte i = 0; i < UART1_data_length; i++) {
              ODU_PAC_ONE_Temp += (int) UART1_data[i];
              ODU_PAC_ONE_Temp += ",";
            }
            ODU_PAC_ONE = ODU_PAC_ONE_Temp;
            

          }
          if (UART1_data[0] == 187 && UART1_data[1] == 2) {

            for (byte i = 0; i < UART1_data_length; i++) {
              ODU_PAC_TWO_Temp += (int) UART1_data[i];
              ODU_PAC_TWO_Temp += ",";
            }

            ODU_PAC_TWO = ODU_PAC_TWO_Temp;
            
          }
          //URL = URI; //+ ODU_PAC_ONE + ODU_PAC_TWO;
          ODU_PAC_ONE_Temp = "";
          ODU_PAC_TWO_Temp = "";
          url_pram = "";
        }
      }
      else if (event.type == UART_FRAME_ERR) {

      }
    }
    if (exit_condition) {
      break;
    }
  }
  vTaskDelete(NULL);
}
void updateSerial() {
  delay(200);
  while (DEBUG.available()) {
    MODEM.write(DEBUG.read());  
  }
  //DEBUG.print("[UPDT: ]");
  while (MODEM.available()) {
    DEBUG.write(MODEM.read());  
  }
}

void getData() {

  delay(200);
  while (MODEM.available()>0) {
    //memCount++;
    buf.concat((char)MODEM.read());
    //buf+=(char)MODEM.read();
    //    DEBUG.print("C:");
    //    DEBUG.print(memCount);
    //    DEBUG.print("\tV:");
    // DEBUG.println(MODEM.read());
    //    DEBUG.print(MODEM.read());
  }
  
}

void test_sim800_module() {
  MODEM.println("AT");
  updateSerial();
  MODEM.println("AT+CSQ");
  updateSerial();
  MODEM.println("AT+CCID");
  updateSerial();
  MODEM.println("AT+CREG?");
  updateSerial();
  MODEM.println("ATI");
  updateSerial();
  MODEM.println("AT+CBC");
  updateSerial();
  // MODEM.println("AT+CUSD=1,\"2#\"");
  // updateSerial();
}

void gsm_config_gprs() {
  Serial.println(" --- CONFIG GPRS --- ");

  MODEM.println("AT+SAPBR=3,1,Contype,GPRS");
  updateSerial();
  MODEM.println("AT+SAPBR=3,1,APN," + apn);
  updateSerial();
  if (apn_u != "") {
    MODEM.println("AT+SAPBR=3,1,USER," + apn_u);
    updateSerial();
  }
  if (apn_p != "") {
    MODEM.println("AT+SAPBR=3,1,PWD," + apn_p);
    updateSerial();
  }
}

void Send_GET_Rqst(String Data, String methodPram) {
   DEBUG.println("[URL] : " + Data);
  MODEM.println("AT+HTTPINIT\r\n");
  updateSerial();
  MODEM.println("AT+HTTPPARA=\"CID\",1\r\n");
  updateSerial();
  MODEM.println("AT+CREG?\r\n");
  updateSerial();
  MODEM.println("AT+SAPBR=2,1\r\n");
  updateSerial();
  MODEM.println("AT+SAPBR=1,1\r\n");
  updateSerial();
  MODEM.println("AT+HTTPINIT\r\n");
  updateSerial();
  MODEM.println("AT+HTTPPARA=URL,\"" + Data + "\"\r\n");
  updateSerial();
  MODEM.println("AT+HTTPPARA=\"CONTENT\",\"application / text\"");
  updateSerial();
  String ActionCMD = "AT + HTTPACTION = " + methodPram + "\r\n";
  MODEM.println(ActionCMD);
  delay(500);
  updateSerial();
  delay(2000);
  MODEM.println("AT+HTTPREAD\r\n");
  delay(500);
  getData();
  //MODEM.println("WAIT=6");
  MODEM.println("AT + HTTPTERM\r\n");
  updateSerial();
}

void writeI2CByte(int wr_data_addr, int wrtData) {
  byte slaveAddr = (wr_data_addr < 256) ? ADDR_ONE : ADDR_TWO;
  Wire.beginTransmission(ADDR_ONE);
  Wire.write(wr_data_addr);
  Wire.write(wrtData);
  Wire.endTransmission();
  delay(10);
}

void read_data() {
  short tempRdData;
  Serial.println("IDU Data Reading Start: ");
  for (int i = 0; i < arr_counter; i++) {
    Serial.print("ADD: ");
    Serial.print(i);
    tempRdData = readI2CByte(i);
    Serial.print("EE: ");
    Serial.println(tempRdData, HEX);
    chkSum += tempRdData;
    delay(10);
  }
  Serial.println();
  Serial.print("Check Sum: ");
  Serial.println(chkSum, HEX);
  chkSum = 0;
}

byte readI2CByte(byte data_addr) {
  byte data = NULL;
  Wire.beginTransmission(ADDR);
  Wire.write(data_addr);
  Wire.endTransmission();
  Wire.requestFrom(ADDR, 1);  //retrieve 1 returned byte
  delay(1);
  if (Wire.available()) {
    data = Wire.read();
  }
  return data;
}
