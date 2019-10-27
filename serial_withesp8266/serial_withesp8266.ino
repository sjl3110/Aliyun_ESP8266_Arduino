/*
    @Author : Jianliang Shen
    @Create time : 2019-10-23 16:15:30
    @Description : ESP8266 + BME280
*/
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C
SoftwareSerial mySerial(13, 12); //D13=RX , D12=tx

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200);
  
  unsigned status;
  status = bme.begin();
  if (!status) {
    mySerial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    mySerial.print("SensorID was: 0x"); Serial.println(bme.sensorID(), 16);
    mySerial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    mySerial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    mySerial.print("        ID of 0x60 represents a BME 280.\n");
    mySerial.print("        ID of 0x61 represents a BME 680.\n");
    while (1);
  }
  
}
void loop()
{
  printValues();
  //printValues2();
  delay(2000);
  //if (mySerial.available()) {
    //Serial.write(mySerial.read());
  //}

}

void printValues() {
  mySerial.print(bme.readTemperature());
  mySerial.print(bme.readPressure() / 100.0F);
  //mySerial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  mySerial.print(bme.readHumidity());
}
void printValues2() {
  Serial.print(bme.readTemperature());
  Serial.print(bme.readPressure() / 100.0F);
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.print(bme.readHumidity());
}
