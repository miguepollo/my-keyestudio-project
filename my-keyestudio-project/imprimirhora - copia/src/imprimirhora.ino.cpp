#include <WiFi.h>
#include <Wire.h>
#include <time.h>
#include <ESP32_Servo.h>
#include "credentials.h"
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>        // https://github.com/bblanchon/ArduinoJson
#include <HTTPClient.h>
#include <ESP32Tone.h>
#include <string.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

String  Time_str = "--:--:--";
Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C mylcd(0x27,16,2);

// Configuración del servidor web asíncrono
AsyncWebServer server(80);
const int EEPROM_SIZE = 512;
const int API_KEY_ADDRESS = 0;
String storedApiKey = "";

// Declaración de funciones
void desplazarTexto(const char* texto, int fila);
void mostrarMensajeMotivacional();
void efectoBienvenida();
void efectoAlarma();
void efectoFiesta();
void breathingEffect();
void rainbowEffect();
bool getWeatherData();



void setup(){
  Serial.begin(115200);
  pinMode(pyroelectric, INPUT);
  pinMode(led_y, OUTPUT);
  pinMode(gasPin, INPUT);
  mylcd.init();
  mylcd.backlight();
  mylcd.clear();
  strip.begin();
  strip.show();
  pinMode(btn1, INPUT);
  mylcd.setCursor(0, 0);
  mylcd.print("HOLA FELIX :)");
  digitalWrite(led_y, HIGH); // Enciende el LED amarillo

}



void rainbowEffect() {
  for(long firstPixelHue = 0; firstPixelHue < 65536; firstPixelHue += 256) {
    strip.setBrightness(128); // Brillo fijo al 50%
    for(int i = 0; i < strip.numPixels(); i++) {
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    delay(20);
  }
}

void loop() {
  rainbowEffect(); // Efecto arcoíris
  delay(1000); // Espera para que el efecto sea visible
}



