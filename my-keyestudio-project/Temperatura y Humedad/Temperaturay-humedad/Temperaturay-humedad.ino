//**********************************************************************************
/*  
 * Filename    : xht11
 * Description : Read the temperature and humidity values of XHT11.
 * Auther      : http//www.keyestudio.coml
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C mylcd(0x27,16,2);
#include "xht11.h"
xht11 xht(17);

unsigned char dht[4] = {0, 0, 0, 0};//Only the first 32 bits of data are received, not the parity bits
void setup() {
  Serial.begin(115200);//Start the serial port monitor and set baud rate to 115200
  mylcd.init();
  mylcd.backlight();
}

void loop() {
  if (xht.receive(dht)) { //Returns true when checked correctly
    Serial.print("RH:");
    Serial.print(dht[0]); //The integral part of humidity, DHT [1] is the fractional part
    Serial.print("%  ");
    Serial.print("Temp:");
    Serial.print(dht[2]); //The integral part of temperature, DHT [3] is the fractional part
    Serial.println("C");
    
    mylcd.setCursor(0, 0);
    mylcd.print("Temperatura = ");
    mylcd.print(dht[2]);
    delay(1700);
    mylcd.setCursor(0, 1);
    mylcd.print("Humedad = ");
    mylcd.print(dht[0]);
    delay(4000);
    mylcd.clear();
    delay(100);
  } else {    //Read error
    Serial.println("sensor error");
  }
  delay(90);  //It takes 90ms to wait for the device to read
}
//**********************************************************************************
