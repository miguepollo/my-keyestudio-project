long SleepDuration   = 10; // Sleep time in minutes, aligned to the nearest minute boundary.
int  WakeupHour      = 7;  // Don't wakeup until after 07:00 to save battery power
int  SleepHour       = 23; // Sleep after 23:00 to save battery power
long StartTime       = 0;
long SleepTimer      = 0;
long Delta           = 30;
int     wifi_signal, CurrentHour = 0;
int     wifi_signal_min, CurrentMin = 0;
int     wifi_signal_sec, CurrentSec = 0;
const char* ssid = "MIWIFI_2G_y36Y";
const char* password = "34sf36crhwnp";

const char* Timezone    = "CET-1CEST,M3.5.0,M10.5.0/3"; // Choose your time zone from: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv 
                                                           // See below for examples
const char* ntpServer   = "0.es.pool.ntp.org"; // Or, choose a time server close to you, but in most cases it's best to use pool.ntp.org or time.google.com to find an NTP server
                                                           // then the NTP system decides e.g. 0.pool.ntp.org, 1.pool.ntp.org as the NTP syem tries to find  the closest available servers
                                                           // EU "0.europe.pool.ntp.org"
                                                           // US "0.north-america.pool.ntp.org"
                                                           // See: https://www.ntppool.org/en/                                                           
int gmtOffset_sec     = +1;    // UK normal time is GMT, so GMT Offset is 0, for US (-5Hrs) is typically -18000, AU is typically (+8hrs) 28800
int daylightOffset_sec = 3600; // In the UK DST is +1hr or 3600-secs, other countries may use 2hrs 7200 or 30-mins 1800 or 5.5hrs 19800 Ahead of GMT use + offset behind - offset
#define buzzer_pin 25
#define pyroelectric 14
int pos = 0;    // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = 13; // GPIO pin for servo signal
int8_t Isused = 0;
int8_t Isused1 = 0;
int8_t Isused2 = 0;
#define led_y 12  //Define the yellow led pin to 12
#include <Adafruit_NeoPixel.h>
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
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "xht11.h"
xht11 xht(17);
unsigned char dht[4] = {0, 0, 0, 0};//Only the first 32 bits of data are received, not the parity bits
boolean i = 1;
boolean j = 1;
#define gasPin 23
#define btn1 16
int btn_count = 0; //Used to count the clicked button times
String PriceBitcoin = "";
String requestType = "GET"; // replace with your actual request type

// OpenWeatherMap API Key
const char* WEATHER_API_KEY = "9ea4c33c5c7679c0d25795d21785a5e8"; // Reemplaza esto con tu API key de OpenWeatherMap
