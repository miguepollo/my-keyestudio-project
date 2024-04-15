#include <WiFi.h>
#include <Wire.h>
#include <time.h>
#include <ESP32_Servo.h>
#include "credentials.h"
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>        // https://github.com/bblanchon/ArduinoJson
#include <HTTPClient.h>
#include <ESP32Tone.h>
String  Time_str = "--:--:--";
Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C mylcd(0x27,16,2);




/*
bool getBitcoinData(WiFiClient & client, const String & RequestType) {
  client.stop(); // close connection before sending a new request
  HTTPClient http;
  String uri = "mempool.space/api/v1/prices";
  String url = "mempool.space";

  http.begin(client, uri, 443, url, true); //http.begin(uri,test_root_ca); //HTTPS example connection
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String MempoolRawPrice = http.getString();
    Serial.println(MempoolRawPrice);
    client.stop();
    http.end();
    return true;
  }
  else
  {
    Serial.printf("connection failed, error: %s", http.errorToString(httpCode).c_str());
    client.stop();
    http.end();
    return false;
  }
  http.end();
  return true;
}
*/









uint8_t StartWiFi() {
  Serial.println("\r\nConnecting to: " + String(ssid));
  IPAddress dns(1, 1, 1, 1); // Use Cloudflare DNS
  WiFi.disconnect();
  WiFi.mode(WIFI_STA); // switch off AP
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("STA: Failed!\n");
    WiFi.disconnect(false);
    delay(500);
    WiFi.begin(ssid, password);
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifi_signal = WiFi.RSSI(); // Get Wifi Signal strength now, because the WiFi will be turned off to save power!
    Serial.println("conectado a: " + WiFi.localIP().toString());
  }
  else Serial.println("*** FALLO DE LA WIFI ***");
  return WiFi.status();
}

boolean UpdateLocalTime() {
  struct tm timeinfo;
  char   time_output[30], day_output[30], update_time[30];
  while (!getLocalTime(&timeinfo, 10000)) { // Wait for 5-sec for time to synchronise
    Serial.println("Fallo al obtener tiempo");
    return false;
  }
  CurrentHour = timeinfo.tm_hour;
  CurrentMin  = timeinfo.tm_min;
  CurrentSec  = timeinfo.tm_sec;
  //See http://www.cplusplus.com/reference/ctime/strftime/

  Time_str = time_output;
  return true;
}


void BeginSleep() {
  mylcd.off();
  UpdateLocalTime();
  SleepTimer = (SleepDuration * 60 - ((CurrentMin % SleepDuration) * 60 + CurrentSec)) + Delta; //Some ESP32 have a RTC that is too fast to maintain accurate time, so add an offset
  esp_sleep_enable_timer_wakeup(SleepTimer * 1000000LL); // in Secs, 1000000LL converts to Secs as unit = 1uSec
  Serial.println("Awake for : " + String((millis() - StartTime) / 1000.0, 3) + "-secs");
  Serial.println("Entering " + String(SleepTimer) + " (secs) of sleep time");
  Serial.println("Starting deep-sleep period...");
  esp_deep_sleep_start();  // Sleep for e.g. 30 minutes
}

boolean SetupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "time.nist.gov"); //(gmtOffset_sec, daylightOffset_sec, ntpServer)
  setenv("TZ", Timezone, 1);  //setenv()adds the "TZ" variable to the environment with a value TimeZone, only used if set to 1, 0 means no change
  tzset(); // Set the TZ environment variable
  delay(100);
  return UpdateLocalTime();
}

void setup(){
  Serial.begin(115200);
  pinMode(pyroelectric, INPUT); //Set pin to input mode
  pinMode(led_y, OUTPUT);  //Set pin to output mode
  pinMode(gasPin, INPUT); //Set pin to input mode
  mylcd.init();
  mylcd.backlight();
  mylcd.clear();
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if (StartWiFi() == WL_CONNECTED && SetupTime() == true) {
    bool WakeUp = false;                
    if (WakeupHour > SleepHour)
      WakeUp = (CurrentHour >= WakeupHour || CurrentHour <= SleepHour); 
    else                             
      WakeUp = (CurrentHour >= WakeupHour && CurrentHour <= SleepHour); 
  }
    myservo.attach(servoPin);   // attaches the servo on pin 18 to the servo object
    strip.begin(); // Initialize NeoPixel strip object (REQUIRED) 
    strip.show();  // Initialize all pixels to 'off'
    strip.setBrightness(50); // Set brightness to about 20% (max = 255)
    pinMode(btn1, INPUT); //Set the button pin to input mode

}

void StopWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi Off");
}


void loop() {
  boolean  pyroelectric_val = digitalRead(pyroelectric);
  SetupTime();
  boolean gasVal = digitalRead(gasPin);  //Reads the value detected by the gas sensor
  boolean btn1_val = digitalRead(btn1); //Read the value of the button
  WiFiClient client;
//  getBitcoinData(client, requestType);

//  Serial.println(" Button: " + String(btn1_val));

  if(CurrentHour >= 7 && CurrentHour <= 23){
    delay(500);
    mylcd.setCursor(0,0);
    mylcd.clear(); 
    mylcd.print("Hora: ");
    mylcd.print(CurrentHour);
    mylcd.print(":");
    mylcd.print(CurrentMin);
    mylcd.print(":");
    mylcd.print(CurrentSec);
    mylcd.setCursor(0,1);
    if(gasVal == 0)  //If the hazardous gas is detected，LCD displays dangerous，the buzzer makes an alarm
    {
      tone(buzzer_pin,294,250,0);
      delay(500);
      noTone(buzzer_pin,0);
    }

    if (gasVal == 1 && CurrentHour >= 9 && CurrentHour <= 23 && Isused == 1)
    {
      noTone(buzzer_pin,0);
    }
    {

    }
    if (CurrentHour >= 7 && CurrentHour <= 8)
    {
      if (pyroelectric_val == 1)
      {
        if (Isused == 0)
        {
          Isused = 1;
          mylcd.setCursor(0,1);
          mylcd.print("Buenos dias");
          tone(buzzer_pin,294,250,0);
          delay(500);
          noTone(buzzer_pin,0);
          delay(7000);
          mylcd.clear();
        }
        else
        {
          mylcd.setCursor(0,1);
          mylcd.print("Temperatura = ");
          mylcd.print(dht[2]);
          delay(55);
          if (Isused1 == 0)
          {
            Isused1 = 1;
            strip.setPixelColor(0, strip.Color(255, 0, 0)); // Red
            strip.setPixelColor(1, strip.Color(0, 255, 0)); // Green
            strip.setPixelColor(2, strip.Color(0, 0, 255)); // Blue
            strip.setPixelColor(3, strip.Color(255, 255, 255)); // White
            strip.show(); // This sends the updated pixel color to the hardware.
            delay(5000);
            strip.clear();
          }
          else if (Isused1 == 1)
          {
            /* code */
          }
          
        }
      }
      else
      {
        mylcd.print("No detectado");
      }
      
    }
    else if (CurrentHour >= 9 && CurrentHour <= 20)
    {
      if (pyroelectric_val == 1)
      {
        mylcd.setCursor(0,1); 
        mylcd.print("Buen dia!");

      }
      else if (pyroelectric_val == 0)
      {
        mylcd.print("No detectado");
        if (Isused2 == 0)
        {
          Isused2 = 1;
          digitalWrite(led_y, HIGH);  //Light up the LED
          delay(10000);     //Delay statement, in ms
          digitalWrite(led_y, LOW);   //Close the LED
        }

      }
      
    }
    else if (CurrentHour >= 21 && CurrentHour <= 23)
    {
      Isused = 0;
      Isused1 = 0;
      Isused2 = 0;
      if (pyroelectric_val == 1)
      {
        mylcd.setCursor(0,1);
        mylcd.print("Hora de dormir");
      }
      else if (pyroelectric_val == 0)
      {
        mylcd.setCursor(0,1);
        mylcd.print("No detectado");
      }
      
      
    }
  }
  else{
    BeginSleep();
  }
  
}


