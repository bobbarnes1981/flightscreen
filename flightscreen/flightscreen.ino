#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "<ssid>";
const char* password = "<password>";

const char* api = "http://<ipaddress>/tar1090/data/aircraft.json";

#define LCD_ADDR 0x27
#define NUM_COLS 16
#define NUM_ROWS 2

#define PIXEL_ADDR 13
#define NUM_PIXELS 67

#define DELAY 5000

Adafruit_NeoPixel pixels (NUM_PIXELS, PIXEL_ADDR, NEO_RGB);
LiquidCrystal_I2C lcd(LCD_ADDR, NUM_COLS, NUM_ROWS);

unsigned long elapsed = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);

  pixels.begin();
  pixels.setBrightness(20);

  lcd.init();
  lcd.backlight();

  WiFi.begin(ssid, password);
  lcd.setCursor(0, 0);
  lcd.print(ssid);
  lcd.setCursor(0, 1);
  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    lcd.print(".");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(ssid);
  lcd.setCursor(0, 1);
  lcd.print("Connected");
}

void loop() {
  unsigned long m = millis();

  if (m-elapsed > DELAY) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(".");

      for (int i = 0; i < NUM_PIXELS; i++) {
        pixels.setPixelColor(i, 0xff, 0x00, 0x00);
      }
      pixels.show();

      HTTPClient http;
      http.begin(api);
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
          JsonArray aircraft = doc["aircraft"];
          if (aircraft.size() > 0) {
            const char* hex = aircraft[0]["hex"];
            const char* flight = aircraft[0]["flight"];

            float lat = aircraft[0]["lat"] | 0.0;
            float lon = aircraft[0]["lon"] | 0.0;
            int altitude = aircraft[0]["alt_baro"] | 0;

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.printf("%s %dft", flight ? flight : "", altitude);
            lcd.setCursor(0, 1);
            lcd.printf("%.4f,%.4f", lat, lon);
          }
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.printf("JSON error");
        }
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.printf("HTTP error: %d", httpCode);
      }

      http.end();

    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("wifi disconnected!");
    }

    elapsed = m;
  }
}
