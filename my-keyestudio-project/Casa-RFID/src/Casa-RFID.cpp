//**********************************************************************************
/*  
 * Filename    : RFID
 * Description : RFID 
 * Auther      : miguelpollo
*/
bool RFIDusadorecientemente = false;
#include "xht11.h"
#include <ESP32_Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
#include "MFRC522_I2C.h"
MFRC522 Rfid(0x28);
#define led_y 12  //Define the yellow led pin to 12
#define fanPin1 19
#define fanPin2 18
LiquidCrystal_I2C mylcd(0x27,16,2);
Servo servoDoor;
Servo servoWindow;
#define servoDoorPin 13
#define servoWindowPin 5
#define btnPin 16
boolean btnFlag = 0;
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    26
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 4
// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


xht11 xht(17);

unsigned char dht[4] = {0, 0, 0, 0};//Only the first 32 bits of data are received, not the parity bits

String password = "";

void setup() {
  Serial.begin(115200);           // initialize and PC's serial communication
  mylcd.init();
  mylcd.backlight();
  Wire.begin();                   // initialize I2C
  Rfid.PCD_Init();             // initialize MFRC522 
  pinMode(fanPin1, OUTPUT);
  pinMode(fanPin2, OUTPUT);           // display PCD - MFRC522 read carder
  servoDoor.attach(servoDoorPin);
  servoWindow.attach(servoWindowPin); 
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(255);
  servoDoor.write(0);
  servoWindow.write(0);
  pinMode(btnPin, INPUT);
  mylcd.setCursor(0, 0);
  mylcd.print("Card please");
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(1000);                           //  Pause for a moment
  }
}

void loop() {  
  for (byte i = 0; i < Rfid.uid.size; i++) {
    Serial.print(Rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    //Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(Rfid.uid.uidByte[i]);
    password = password + String(Rfid.uid.uidByte[i]);
  }

  if (RFIDusadorecientemente == false)
  {
    if (xht.receive(dht)){
      mylcd.setCursor(0, 1);
      mylcd.print(dht[2]);
      if (dht[2] >=28)
      { 
        analogWrite(fanPin2, 150);
      }
      else if ( dht[2] <28 >25 ) {
        analogWrite(fanPin2, 110);  
      }
      else if (dht[2] <23) 
      {
        analogWrite(fanPin2, 0);
      }
    } 
  }


  if(Rfid.PICC_IsNewCardPresent() || Rfid.PICC_ReadCardSerial())  //The card number is correct
  {
    mylcd.setCursor(0, 0);
    mylcd.print("Open");
    servoDoor.write(180);
    servoWindow.write(180);
    RFIDusadorecientemente = true;
    password = "";
    btnFlag = 1;
    colorWipe(strip.Color(  220,   0, 255), 50); 
    if (xht.receive(dht)){
      mylcd.clear();
      mylcd.setCursor(0, 0);
      mylcd.print(dht[2]);
      if (dht[2] >=28)
      { 
        analogWrite(fanPin2, 190);
      }
      else if ( dht[2] <28 >25 ) {
        analogWrite(fanPin2, 150);  
      }
      else if (dht[2] <25) 
      {
        analogWrite(fanPin2, 0);
      }
    }
    digitalWrite(led_y, HIGH);
  }
  Serial.println(password);
  
}


//**********************************************************************************