#include "defines.h"
String URI =  SRV_IP + PROJECT_PATH;
String URL = URI;
String val = "";
int EE_Data[] = {};
int memCount = 0;
#define ADDR_Ax 0b000 //A2, A1, A0
#define ADDR (0b1010 << 3) + ADDR_Ax
void setup()
{
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
  test_sim800_module(); delay(1000);
  gsm_config_gprs(); delay(1000);
}

void loop() {
  DEBUG.println(URL);
  Send_GET_Rqst(URL);
  //sendGetRequest(URL);
  PRAM = "";
  delay(3000);
  DEBUG.println("**********************  START  ********************** ");
  DEBUG.println(val);
  explode(",", val);
  read_data();
//    for (int b = 0; b < 110; b++) {
//      readI2CByte(b);
//      //DEBUG.println(EE_Data[b]);
//    }
  DEBUG.println("**********************   END   ********************** ");
  val = "";
}
void explode (char delim[], String rcv_Str) {
  int str_len = rcv_Str.length() + 1;
  int a = 0, c = 0;
  char char_array[str_len], rslt_Arr[str_len];
  rcv_Str.toCharArray(char_array, str_len);
  char *ptr = strtok(char_array, delim);
  int decimalVal = 0;
  int strt_pos = 0, totl_Elmnt = 0;
  DEBUG.println("-----------> Start Exploding: ");
  while (ptr != NULL)
  {
    int  strLen = strlen(ptr);
    for (int i = 0; i < strLen; i++) {
      decimalVal +=  (ptr[i] - '0') * pow(10, (strLen - 1 - i));
    }
    if (a > 0 ) {
      if (a == 1) {
        strt_pos = decimalVal;
        DEBUG.print("\t STRT POS : "); DEBUG.print(strt_pos);
      }
      else if (a == 2) {
        totl_Elmnt = decimalVal+3;
        DEBUG.print("\t TOTL ELM : "); DEBUG.println(totl_Elmnt);
        //a = strt_pos;
      }
      else if ((a > 2) && (a <=totl_Elmnt)) {
        //totl_Elmnt = decimalVal;
        //DEBUG.print("\t TOTL ELM : "); DEBUG.println(totl_Elmnt);
        //strt_pos;
        //Serial.print("RX DATA : "); Serial.print(ptr);
        //DEBUG.print(" Val of A : "); DEBUG.print(strt_pos);
        DEBUG.print(" Val A : "); DEBUG.print(a);
        DEBUG.print("\t ADDR : "); DEBUG.print(strt_pos);
        DEBUG.print("\t DECM : "); DEBUG.println(decimalVal);
        writeI2CByte(strt_pos, decimalVal);
        //EE_Data[c] = decimalVal;
        strt_pos++;
      }

    }
    decimalVal = 0;
    ptr = strtok(NULL, delim);
    a++; c++;
  }
}
//void explode (char delim[], String rcv_Str) {
//  int str_len = rcv_Str.length() + 1;
//  int a = 0;
//  char char_array[str_len], rslt_Arr[str_len];
//  rcv_Str.toCharArray(char_array, str_len);
//  char *ptr = strtok(char_array, delim);
//  int decimalVal = 0;
//  int strt_pos = 0, totl_Elmnt = 1024;
//  DEBUG.println("-----------> Start Exploding: ");
//  while (ptr != NULL)
//  {
//    int  strLen = strlen(ptr);
//    for (int i = 0; i < strLen; i++) {
//      decimalVal +=  (ptr[i] - '0') * pow(10, (strLen - 1 - i));
//    }
//    DEBUG.print("\t FNL : "); DEBUG.println(decimalVal);
//    if (a > 0 ) {
//      if (a == 1) {
//        strt_pos = decimalVal;
//        DEBUG.print("\t STRT POS : "); DEBUG.print(strt_pos);
//      }
//      else if (a == 2) {
//        totl_Elmnt = decimalVal;
//        DEBUG.print("\t TOTL ELM : "); DEBUG.println(totl_Elmnt);
//        //a = strt_pos;
//      }
//      else if (a > 2) {
//        //if ((a + 3) < totl_Elmnt - 1) {
//        DEBUG.print(" Add : "); DEBUG.print(strt_pos);
//        DEBUG.print("\t FNL : "); DEBUG.println(decimalVal);
//        //    Wire.beginTransmission(ADDR_ONE);
//        //    Wire.write(a);
//        //    Wire.write(decimalVal);
//        //    Wire.endTransmission();
//        //    delay(10);
//        strt_pos++;
//        //}
//    }
//    decimalVal = 0;
//    ptr = strtok(NULL, delim);
//    a++;
//  }
//  DEBUG.println("-----------> END Exploding: ");
//}

static void UART_ISR_ROUTINE(void *pvParameters)                //uart1
{
  uart_event_t event;
  size_t buffered_size;
  bool exit_condition = false;
  while (1) {
    //Loop will continually block (i.e. wait) on event messages from the event queue
    if (xQueueReceive(uart1_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
      if (event.type == UART_DATA) {
        uint8_t UART1_data[128];
        int UART1_data_length = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_1, (size_t*)&UART1_data_length));
        UART1_data_length = uart_read_bytes(UART_NUM_1, UART1_data, UART1_data_length, 100);
        //Serial.print("LEN= "); Serial.println(UART1_data_length);
        if (UART1_data_length == ODU_DATA_SIZE) {
          //          DEBUG.print("DTA LEN : ");
          //          DEBUG.println(UART1_data_length);
          //          DEBUG.print(" ");
          //          DEBUG.print(" DATA : ");
          if (UART1_data[0] == 170) {
            for (byte i = 0; i < UART1_data_length; i++) {
              url_pram += (int) UART1_data[i];
              url_pram += ",";
            }
          }
          URL = URI + url_pram;
          //rd_buff = true;
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
  delay(100);
  while (DEBUG.available())
  {
    MODEM.write(DEBUG.read());//Forward what Serial received to Software Serial Port
  }
  while (MODEM.available())
  {
    DEBUG.write(MODEM.read());//Forward what Software Serial received to Serial Port
  }
}

void getData() {

  delay(200);
  while (DEBUG.available())
  {
    MODEM.write(DEBUG.read());//Forward what Serial received to Software Serial Port
  }
  while (MODEM.available())
  {
    //memCount++;
    val.concat((char) MODEM.read());
    //    DEBUG.print("C:");
    //    DEBUG.print(memCount);
    //    DEBUG.print("\tV:");
    //    DEBUG.println((char) MODEM.read());
    //    DEBUG.print(MODEM.read());

  }
}

void test_sim800_module()
{
  MODEM.println("AT");  updateSerial();
  MODEM.println("AT+CSQ");  updateSerial();
  MODEM.println("AT+CCID");  updateSerial();
  MODEM.println("AT+CREG?");  updateSerial();
  MODEM.println("ATI");  updateSerial();
  MODEM.println("AT+CBC"); updateSerial();
  MODEM.println("AT+CUSD=1,\"2#\""); updateSerial();

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

void Send_GET_Rqst(String Data) {
  //"AT+CIPSHUT"
  DEBUG.println("Sending Data To Server: ");
  DEBUG.println("[URL] : " + Data );
  MODEM.println("AT+HTTPINIT\r\n"); updateSerial();
  MODEM.println("AT+HTTPPARA=\"CID\",1\r\n"); updateSerial();
  MODEM.println("AT+CREG?\r\n"); updateSerial();
  MODEM.println("AT+SAPBR=2,1\r\n"); updateSerial();
  MODEM.println("AT+SAPBR=1,1\r\n"); updateSerial();
  MODEM.println("AT+HTTPINIT\r\n"); updateSerial();
  MODEM.println("AT+HTTPPARA=URL,\"" + Data +  "\"\r\n"); updateSerial();
  MODEM.println("AT+HTTPPARA=\"CONTENT\",\"application / text\""); updateSerial();
  MODEM.println("AT + HTTPACTION = 0\r\n"); delay(2000); updateSerial();
  MODEM.println("AT+HTTPREAD"); delay(2000); getData();
  MODEM.println("AT + HTTPTERM\r\n"); updateSerial();
}

//void write_data(char chkData[]) {
//  Serial.println("IDU Data Writing Start: ");
//  for (int j = 0; j < arr_counter; j++) {
//    Serial.print("ADD: "); Serial.print(j);
//    Serial.print("\t TX: 0x"); Serial.println(chkData[j], HEX);
//    writeI2CByte(j, chkData[j]);
//    delay(10);
//  }
//}
void writeI2CByte(int wr_data_addr, int wrtData) {
  byte slaveAddr = ( wr_data_addr < 256 ) ? ADDR_ONE : ADDR_TWO;
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
    Serial.print("ADD: "); Serial.print(i);
    tempRdData = readI2CByte(i);
    Serial.print("EE: "); Serial.println(tempRdData,HEX);
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
  Wire.requestFrom(ADDR, 1); //retrieve 1 returned byte
  delay(1);
  if (Wire.available()) {
    data = Wire.read();
  }
  return data;
}
