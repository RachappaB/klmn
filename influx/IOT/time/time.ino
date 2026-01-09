#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define TCA_ADDR  0x70
#define OLED_ADDR 0x3C

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RTC_DS3231 rtc;

void tcaSelect(uint8_t channel) {
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(1000);

  // ---------- OLED ----------
  tcaSelect(0);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED NOT detected");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ---------- RTC ----------
  tcaSelect(7);  // ✅ CORRECT CHANNEL
  if (!rtc.begin()) {
    Serial.println("RTC NOT detected");
    while (1);
  }

  // Uncomment ONCE to set RTC
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.println("RTC + OLED READY");
}

void loop() {
  // Read RTC
  tcaSelect(7);
  DateTime now = rtc.now();

  // Display time
  tcaSelect(0);
  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 0);

  if (now.hour() < 10) display.print("0");
  display.print(now.hour());
  display.print(":");

  if (now.minute() < 10) display.print("0");
  display.print(now.minute());
  display.print(":");

  if (now.second() < 10) display.print("0");
  display.print(now.second());

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print(now.day());
  display.print("/");
  display.print(now.month());
  display.print("/");
  display.print(now.year());

  display.display();
  delay(1000);
}
