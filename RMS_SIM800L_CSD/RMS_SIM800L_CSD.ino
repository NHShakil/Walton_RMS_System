#include "defines.h"

void setup() {
  DEBUG.begin("WALTON_IOT_DEV_46416");
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
  for (int i=0;i<5;i++) {
    gsm_config_gprs();
    delay(1000);
    //ESP.restart();
  }
}

void loop() {
  readCompModel();
  delay(100);
  eePortType = detect_ee();
  delay(100);
  Send_GET_Rqst(URL + DEV_ID + ";" + MOB_NO + ";" + sigLvl + ";" + ODU_PAC_ONE + ";" + ODU_PAC_TWO + ";" + ODU_PAC_THREE + ";" + eePortType + ";" + IDU_EE_DATA + ";" + ODU_EE_DATA, "0");
  explode(",", srvResPns);
  if (IDU_EE_RD_FLG == 1) {
    read_EE_data(255);
  }
  if (ODU_EE_RD_FLG == 1) {
    read_EE_data(1023);
  }

  delay(1000);
  srvResPns = "";
  IDU_EE_RD_FLG = 0;
  IDU_EE_WR_FLG = 0;
  ODU_EE_RD_FLG = 0;
  ODU_EE_WR_FLG = 0;
  eePortType = 0;
}
void explode(char delim[], String rcv_Str) {
  int str_len = rcv_Str.length() + 1;
  int a = 0, c = 0;
  char char_array[str_len], rslt_Arr[str_len];
  rcv_Str.toCharArray(char_array, str_len);
  char *ptr = strtok(char_array, delim);
  int decimalVal = 0;
  int strt_pos = 0, end_pos = 0, totl_Elmnt = 0;
  while (ptr != NULL) {
    int strLen = strlen(ptr);
    for (int i = 0; i < strLen; i++) {
      decimalVal += (ptr[i] - '0') * pow(10, (strLen - 1 - i));
    }
    if (a > 0) {
      if (a == 1) {
        mode = decimalVal;
        DEBUG.print("Mode : ");
        DEBUG.println(mode);
        if (mode == 2) { IDU_EE_WR_FLG = 1; }
        if (mode == 3) { ODU_EE_WR_FLG = 1; }
        if (mode == 4) { IDU_EE_RD_FLG = 1; }
        if (mode == 5) { ODU_EE_RD_FLG = 1; }
      } else if (a == 2) {
        if (decimalVal < 0) { break; }
        strt_pos = decimalVal;
        DEBUG.print("\t Start Pos : ");
        DEBUG.println(strt_pos);
      } else if (a == 3) {
        if (decimalVal < 0) { break; }
        end_pos = decimalVal;
        DEBUG.print("\t END Pos: ");
        DEBUG.println(end_pos);
      } else if (a == 4) {
        totl_Elmnt = decimalVal + 4;
        DEBUG.print("\t TOTL ELM : ");
        DEBUG.println(totl_Elmnt);
      } else if ((a > 4) && (a <= totl_Elmnt)) {
        if (IDU_EE_WR_FLG == 1 || ODU_EE_WR_FLG == 1) {
          writeI2CByte(strt_pos, decimalVal);
          delay(10);
        }
        strt_pos++;
      }
    }
    decimalVal = 0;
    ptr = strtok(NULL, delim);
    a++;
  }
}
static void UART_ISR_ROUTINE(void *pvParameters)  //uart1
{
  uart_event_t event;
  size_t buffered_size;
  bool exit_condition = false;
  String ODU_PAC_ONE_Temp = "";
  String ODU_PAC_TWO_Temp = "";
  String ODU_PAC_Three_Temp = "";
  ODU_PAC_ONE = "";
  ODU_PAC_TWO = "";
  ODU_PAC_THREE = "";
  while (1) {
    if (xQueueReceive(uart1_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
      if (event.type == UART_DATA) {
        uint8_t UART1_data[128];
        int UART1_data_length = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_1, (size_t *)&UART1_data_length));
        UART1_data_length = uart_read_bytes(UART_NUM_1, UART1_data, UART1_data_length, 100);
        if (UART1_data_length == ODU_RSPNS_PAC_SIZE) {
          if (UART1_data[0] == 187 && UART1_data[1] == 2) {
            for (byte i = 0; i < UART1_data_length; i++) {
              ODU_PAC_TWO_Temp += (int)UART1_data[i];
              ODU_PAC_TWO_Temp += ",";
            }
            ODU_PAC_TWO = ODU_PAC_TWO_Temp;
          } else if (UART1_data[0] == 170 && UART1_data[1] == 20) {
            for (byte i = 0; i < UART1_data_length; i++) {
              ODU_PAC_Three_Temp += (int)UART1_data[i];
              ODU_PAC_Three_Temp += ",";
            }
            ODU_PAC_THREE = ODU_PAC_Three_Temp;
          } else {
            if (UART1_data[0] == 170) {
              for (byte i = 0; i < UART1_data_length; i++) {
                ODU_PAC_ONE_Temp += (int)UART1_data[i];
                ODU_PAC_ONE_Temp += ",";
              }
              ODU_PAC_ONE = ODU_PAC_ONE_Temp;
            }
          }
        }
        ODU_PAC_ONE_Temp = "";
        ODU_PAC_TWO_Temp = "";
        ODU_PAC_Three_Temp = "";
        url_pram = "";

      } else if (event.type == UART_FRAME_ERR) {
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
  while (MODEM.available()) {
    DEBUG.write(MODEM.read());
  }
}

void getData() {
  delay(200);
  while (MODEM.available() > 0) {
    srvResPns.concat((char)MODEM.read());
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
}

void gsm_config_gprs() {
  DEBUG.println(" --- CONFIG GPRS --- ");
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
  //DEBUG.println("Sending Data To Server: ");
  //DEBUG.println("[URL] : " + Data);
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
  MODEM.println("AT + HTTPACTION = 0\r\n");
  delay(2500);
  updateSerial();
  MODEM.println("AT+HTTPREAD");
  delay(3000);
  getData();
  MODEM.println("AT + HTTPTERM\r\n");
  updateSerial();
  IDU_EE_DATA = "";
  ODU_EE_DATA = "";
}

void writeI2CByte(int wr_data_addr, int wrtData) {
  byte slaveAddr = (wr_data_addr < 256) ? ADDR_ONE : ADDR_TWO;
  Wire.beginTransmission(slaveAddr);
  Wire.write(wr_data_addr);
  Wire.write(wrtData);
  Wire.endTransmission();
  delay(10);
}

void read_EE_data(int buff_size) {
  short tempRdData;
  IDU_EE_DATA = "";
  for (int i = 0; i < buff_size; i++) {
    tempRdData = readI2CByte(i);
    delay(10);
    chkSum += tempRdData;
    IDU_EE_DATA += (tempRdData);
    IDU_EE_DATA += (",");
  }
  IDU_EE_DATA += (chkSum);
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
int detect_ee() {
  byte error, address;
  int nDevices;
  nDevices = 0;
  for (address = 80; address < 82; address++) {
    WIRE.beginTransmission(address);
    error = WIRE.endTransmission();
    if (error == 0) {
      nDevices++;
    }
  }
  return nDevices;
}
void readCompModel() {
  for (unsigned int on_B = 0; on_B <= 4; on_B++) {
    const char x = (int16_t)compModel[on_B];
    uart_write_bytes(UART_NUM_1, &x, 1);
  }
  uart_wait_tx_done(UART_NUM_1, 300);
}