#include "defines.h"
String URI =  SRV_IP + PROJECT_PATH;
String URL = "";
void setup()
{
  DEBUG.begin(DEBUG_BAUD_RATE);
  // UART2 Configuration
  MODEM.begin(MODEM_BAUD_RATE);
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

  //Configurations
  test_sim800_module(); delay(1000);
  gsm_config_gprs(); delay(1000);
}

void loop() {
  Send_GET_Rqst(URL);
  PRAM = "";
  delay(3000);
}


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
          
          for (byte i = 0; i < UART1_data_length; i++) {
            url_pram += (int) UART1_data[i];
            url_pram += ",";
          }
          Serial.println(url_pram);
          URL = URI + url_pram;
          //rd_buff = true;
          url_pram = "";
        }
        //DEBUG.println("");
      }
      else if (event.type == UART_FRAME_ERR) {
        //TODO...
      } else {
        //TODO...
      }
    }
    if (exit_condition) {
      break;
    }
  }
  vTaskDelete(NULL);
}
void updateSerial() {
  delay(500);
  while (Serial.available())
  {
    MODEM.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (MODEM.available())
  {
    Serial.write(MODEM.read());//Forward what Software Serial received to Serial Port
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
  DEBUG.println("[URL] : " + Data );
  MODEM.println("AT+HTTPINIT"); updateSerial();
  MODEM.println("AT+HTTPPARA=\"CID\",1"); updateSerial();
  MODEM.println("AT+CREG?"); updateSerial();
  MODEM.println("AT+SAPBR=2,1"); updateSerial();
  MODEM.println("AT+SAPBR=1,1"); updateSerial();
  MODEM.println("AT+HTTPINIT"); updateSerial();
  MODEM.println("AT+HTTPPARA=URL,\"" + Data +  "\""); updateSerial();
  MODEM.println("AT+HTTPPARA=\"CONTENT\",\"application / text\""); updateSerial();
  MODEM.println("AT + HTTPACTION = 0"); updateSerial();
  MODEM.println("AT + HTTPREAD=0,300"); updateSerial();
  MODEM.println("AT + HTTPTERM"); updateSerial();
}
