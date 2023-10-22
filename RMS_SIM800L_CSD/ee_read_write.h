#include <Wire.h>
#include "IDU_EE_data.h"
#include "ODU_EE_data.h"
#define ADDR_Ax 0b000  //A2, A1, A0
#define ADDR_ONE 0x50
#define ADDR_TWO 0x51
#define IDU_ARR_LENGTH 255
#define ODU_ARR_LENGTH 512

short chkSum = 0, chkData = 0, arr_counter = IDU_ARR_LENGTH;


void setup() {
  Serial.begin(9600);
  Wire.begin();               // Master Declearation
  //Wire.begin(0x50);         // Slave Declearation
  write_data(ee_idu_data);
  delay(3000);
  read_data();
  delay(2000);
}

void loop() {

}
void write_data(char chkData[]) {
  Serial.println("IDU Data Writing Start: ");
  for (int j = 0; j < arr_counter; j++) {
    Serial.print("ADD: ");Serial.print(j);
    Serial.print("\t TX: 0x");Serial.println(chkData[j],HEX);
    writeI2CByte(j, chkData[j]);
    delay(10);
  }
}
void writeI2CByte(byte wr_data_addr, byte wrtData) {
  Wire.beginTransmission(ADDR_ONE);
  Wire.write(wr_data_addr);
  Wire.write(wrtData);
  Wire.endTransmission();
  delay(30);
}

void read_data() {
  short tempRdData;
  Serial.println("ODU Data Reading Start: ");
  for (int i = 0; i < arr_counter; i++) {
    Serial.print("ADD: ");Serial.print(i);
    tempRdData = readI2CByte(i);
    chkSum += tempRdData;
    delay(10);
  }
  Serial.println();
  Serial.print("Check Sum: ");
  Serial.println(chkSum, HEX);
  chkSum = 0;
}

byte readI2CByte(byte data_addr) {

  if (data_addr <= 256) {
    byte rdData = NULL;
    Wire.beginTransmission(ADDR_ONE);
    Wire.write(data_addr);
    Wire.endTransmission();
    Wire.requestFrom(ADDR_ONE, 1);  //retrieve 1 returned byte
    delay(1);
    if (Wire.available()) {
      rdData = Wire.read();
    }
    Serial.print("\t RX: 0x");Serial.println(rdData,HEX);
    return rdData;
  }
  if (data_addr > 256) {
    byte rdData = NULL;
    Wire.beginTransmission(ADDR_TWO);
    Wire.write(data_addr);
    Wire.endTransmission();
    Wire.requestFrom(ADDR_TWO, 1);  //retrieve 1 returned byte
    delay(1);
    if (Wire.available()) {
      rdData = Wire.read();
    }
    Serial.print("\t RX: 0x");Serial.println(rdData,HEX);
    return rdData;
  }
}
