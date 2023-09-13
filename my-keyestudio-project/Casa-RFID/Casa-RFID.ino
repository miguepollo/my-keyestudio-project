//**********************************************************************************
/*  
 * Filename    : RFID
 * Description : RFID 
 * Auther      : miguelpollo
*/
boolean RFIDusadorecientemente = false;
#include <ESP32_Servo.h>
#define led_y 12  //Define the yellow led pin to 12
#define fanPin1 19
#define fanPin2 18
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C mylcd(0x27,16,2);
#include <ESP32_Servo.h>
#define servoPin 5
Servo myservo;
#include <Wire.h>
#include "MFRC522_I2C.h"
MFRC522 mfrc522(0x28);   
#define servoPin  13
#define btnPin 16
boolean btnFlag = 0;

#include "xht11.h"
xht11 xht(17);

unsigned char dht[4] = {0, 0, 0, 0};//Only the first 32 bits of data are received, not the parity bits

String password = "";

void setup() {
  Serial.begin(115200);           // initialize and PC's serial communication
  mylcd.init();
  mylcd.backlight();
  Wire.begin();                   // initialize I2C
  mfrc522.PCD_Init();             // initialize MFRC522 
  pinMode(fanPin1, OUTPUT);
  pinMode(fanPin2, OUTPUT);           // display PCD - MFRC522 read carder
  myservo.attach(servoPin);
  myservo.write(0);
  pinMode(btnPin, INPUT);
  mylcd.setCursor(0, 0);
  mylcd.print("Card please");
}

void loop() {  
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    //Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(mfrc522.uid.uidByte[i]);
    password = password + String(mfrc522.uid.uidByte[i]);
  }

  if (RFIDusadorecientemente == false)
  {
    if (xht.receive(dht)){
      mylcd.clear();
      mylcd.setCursor(0, 0);
      mylcd.print(dht[2]);
      if (dht[2] >=25)
      { 
        analogWrite(fanPin2, 160);
      }
      else if ( dht[2] <25 >23 ) {
        analogWrite(fanPin2, 110);  
      }
      else if (dht[2] <23) 
      {
        analogWrite(fanPin2, 0);
      }
    } 
  }


  if(mfrc522.PICC_IsNewCardPresent() || mfrc522.PICC_ReadCardSerial())  //The card number is correct
  {
    mylcd.setCursor(0, 0);
    mylcd.print("Open");
    myservo.write(180);
    RFIDusadorecientemente = true;
    password = "";
    btnFlag = 1;
    if (xht.receive(dht)){
      mylcd.clear();
      mylcd.setCursor(0, 0);
      mylcd.print(dht[2]);
      if (dht[2] >=25)
      { 
        analogWrite(fanPin2, 200);
      }
      else if ( dht[2] <25 >23 ) {
        analogWrite(fanPin2, 160);  
      }
      else if (dht[2] <23) 
      {
        analogWrite(fanPin2, 0);
      }
    }
    digitalWrite(led_y, HIGH);
  }
  Serial.println(password);
  
}


//**********************************************************************************
