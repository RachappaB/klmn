#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ---------- PINS ----------
#define SDA_PIN 21
#define SCL_PIN 22
#define ONE_WIRE_BUS 4   // DS18B20 DATA (D4)

// ---------- TCA ----------
#define TCA_ADDR 0x70

// ---------- OLED ----------
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- DS18B20 ----------
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ---------- TCA SELECT ----------
void tcaSelect(uint8_t channel) {
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(1000);

  // ----- OLED INIT -----
  tcaSelect(0);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED NOT detected");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ----- DS18B20 INIT -----
  sensors.begin();
  Serial.println("DS18B20 + OLED READY");
}

void loop() {
  // Read temperature
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  // Update OLED
  tcaSelect(0);
  display.clearDisplay();
  display.setCursor(0, 0);

  display.setTextSize(1);
  display.println("DS18B20 TEMP");
  display.println("----------------");

  display.setTextSize(2);
  if (tempC == DEVICE_DISCONNECTED_C) {
    display.println("ERROR");
  } else {
    display.print(tempC, 1);
    display.print(" C");
  }

  display.display();
  delay(1000);
}
