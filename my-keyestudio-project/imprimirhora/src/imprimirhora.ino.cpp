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

void setupServer() {
  // Leer API key guardada
  EEPROM.begin(EEPROM_SIZE);
  char tempKey[33];
  EEPROM.get(API_KEY_ADDRESS, tempKey);
  storedApiKey = String(tempKey);
  
  // Configurar rutas del servidor
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial; margin: 20px; }";
    html += "input[type=text] { width: 100%; padding: 12px; margin: 8px 0; }";
    html += "input[type=submit] { background-color: #4CAF50; color: white; padding: 12px 20px; border: none; cursor: pointer; }";
    html += "</style></head><body>";
    html += "<h1>Configuración OpenWeatherMap</h1>";
    html += "<form action='/save' method='post'>";
    html += "API Key: <input type='text' name='apikey' value='" + storedApiKey + "'><br><br>";
    html += "<input type='submit' value='Guardar'>";
    html += "</form>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("apikey", true)) {
      String newApiKey = request->getParam("apikey", true)->value();
      // Guardar en EEPROM
      char tempKey[33];
      newApiKey.toCharArray(tempKey, 33);
      EEPROM.put(API_KEY_ADDRESS, tempKey);
      EEPROM.commit();
      storedApiKey = newApiKey;
      request->send(200, "text/html", "API Key guardada. Reiniciando...");
      delay(1000);
      ESP.restart();
    } else {
      request->send(400, "text/html", "Error: No se proporcionó API Key");
    }
  });

  server.begin();
}

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
  pinMode(pyroelectric, INPUT);
  pinMode(led_y, OUTPUT);
  pinMode(gasPin, INPUT);
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
  myservo.attach(servoPin);
  strip.begin();
  strip.show();
  strip.setBrightness(50);
  pinMode(btn1, INPUT);
  
  // Iniciar servidor web
  setupServer();
}

void StopWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi Off");
}

// Función para desplazar texto en la pantalla LCD
void desplazarTexto(const char* texto, int fila) {
  int longitud = strlen(texto);
  if (longitud <= 16) {
    // Si el texto es corto, simplemente lo mostramos
    mylcd.setCursor(0, fila);
    mylcd.print(texto);
  } else {
    // Si el texto es largo, lo hacemos desplazar
    for (int pos = 0; pos < longitud; pos++) {
      mylcd.setCursor(0, fila);
      for (int i = 0; i < 16; i++) {
        int indice = (pos + i) % longitud;
        mylcd.print(texto[indice]);
      }
      delay(300); // Velocidad del desplazamiento
    }
  }
}

// Función para obtener el clima
bool getWeatherData() {
  if (storedApiKey.length() == 0) {
    Serial.println("API Key no configurada");
    mylcd.setCursor(0,1);
    mylcd.print("Config API Key");
    return false;
  }

  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=Sanchinarro&appid=" + storedApiKey + "&units=metric&lang=es";
  
  Serial.println("Conectando a OpenWeatherMap...");
  Serial.println("URL: " + url);
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Respuesta recibida: " + payload);
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      Serial.print("Error deserializando JSON: ");
      Serial.println(error.c_str());
      mylcd.setCursor(0,1);
      mylcd.print("Error JSON");
      return false;
    }
    
    float temperature = doc["main"]["temp"];
    String weather = doc["weather"][0]["description"];
    float humidity = doc["main"]["humidity"];
    
    // Limpiar la línea antes de escribir
    mylcd.setCursor(0,1);
    mylcd.print("                "); // 16 espacios para limpiar
    
    // Crear mensaje más compacto
    String weatherMessage = String(temperature, 1) + "C " + weather.substring(0, 6);
    if (weatherMessage.length() > 16) {
      weatherMessage = weatherMessage.substring(0, 16);
    }
    
    Serial.println("Mensaje del tiempo: " + weatherMessage);
    
    // Mostrar en la segunda línea
    mylcd.setCursor(0,1);
    mylcd.print(weatherMessage);
    return true;
  } else {
    Serial.println("Error en la conexión HTTP: " + String(httpCode));
    mylcd.setCursor(0,1);
    mylcd.print("Error " + String(httpCode));
  }
  http.end();
  return false;
}

// Función para mostrar mensajes motivacionales
void mostrarMensajeMotivacional() {
  const char* mensajes[] = {
    "¡Buen dia! - ¡Que tengas un excelente dia!",
    "¡Hora de brillar! - ¡Vamos a por ello!",
    "¡Hoy sera genial! - ¡Aprovecha el dia!",
    "¡Buenos dias! - ¡Que tengas un dia maravilloso!",
    "¡Despierta y brilla! - ¡El dia te espera!"
  };
  static int ultimoMensaje = -1;
  int mensajeAleatorio;
  
  do {
    mensajeAleatorio = random(0, 5);
  } while (mensajeAleatorio == ultimoMensaje);
  
  ultimoMensaje = mensajeAleatorio;
  desplazarTexto(mensajes[mensajeAleatorio], 1);
}

// Efecto de LED tipo "alarma"
void efectoAlarma() {
  for(int i = 0; i < 3; i++) {
    strip.fill(strip.Color(255, 0, 0));
    strip.show();
    delay(200);
    strip.clear();
    strip.show();
    delay(200);
  }
}

// Efecto de LED tipo "fiesta"
void efectoFiesta() {
  for(int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(
      random(0, 255),
      random(0, 255),
      random(0, 255)
    ));
  }
  strip.show();
  delay(100);
}

// Función para controlar el servo con un efecto de "bienvenida"
void efectoBienvenida() {
  myservo.write(0);
  delay(500);
  myservo.write(90);
  delay(500);
  myservo.write(180);
  delay(500);
  myservo.write(90);
}

// Efecto de LED tipo "respiración"
void breathingEffect() {
  for(int i = 0; i < 255; i++) {
    strip.setBrightness(50); // Brillo fijo al 20%
    strip.fill(strip.Color(0, 0, 255)); // Color azul
    strip.show();
    delay(5);
  }
  for(int i = 255; i >= 0; i--) {
    strip.setBrightness(50); // Brillo fijo al 20%
    strip.fill(strip.Color(0, 0, 255)); // Color azul
    strip.show();
    delay(5);
  }
}

// Efecto de LED tipo "arcoíris"
void rainbowEffect() {
  for(long firstPixelHue = 0; firstPixelHue < 65536; firstPixelHue += 256) {
    strip.setBrightness(50); // Brillo fijo al 20%
    for(int i = 0; i < strip.numPixels(); i++) {
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    delay(20);
  }
}

void loop() {
  boolean  pyroelectric_val = digitalRead(pyroelectric);
  SetupTime();
  boolean gasVal = digitalRead(gasPin);
  boolean btn1_val = digitalRead(btn1);

  if(CurrentHour >= 7 && CurrentHour <= 23){
    delay(1000);
    mylcd.setCursor(6,0);
    if(CurrentHour < 10) {
      mylcd.print("0");
    }
    mylcd.print(CurrentHour);
    mylcd.print(":");
    if(CurrentMin < 10) {
      mylcd.print("0");
    }
    mylcd.print(CurrentMin);
    mylcd.print(":");
    if(CurrentSec < 10) {
      mylcd.print("0");
    }
    mylcd.print(CurrentSec);
    mylcd.print("   ");

    // Detección de gas mejorada
    if(gasVal == 0) {
      efectoAlarma();
      tone(buzzer_pin,294,250,0);
      delay(500);
      noTone(buzzer_pin,0);
      mylcd.setCursor(0,1);
      mylcd.print("¡ALERTA GAS!");
    } else {
      // Actualizar el clima cada 5 minutos
      static unsigned long lastWeatherUpdate = 0;
      if (millis() - lastWeatherUpdate > 300000) { // 5 minutos
        getWeatherData();
        lastWeatherUpdate = millis();
      }
    }

    // Efectos según la hora del día
    if (pyroelectric_val == 1) {
      if (CurrentHour >= 7 && CurrentHour <= 9) {
        efectoBienvenida();
        rainbowEffect();
      } else if (CurrentHour >= 20 && CurrentHour <= 23) {
        breathingEffect();
      } else {
        strip.fill(strip.Color(0, 255, 0));
        strip.show();
      }
    } else {
      strip.clear();
      strip.show();
    }
  }
  else{
    BeginSleep();
  }
}


