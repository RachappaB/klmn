#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define TCA_ADDR  0x70
#define OLED_ADDR 0x3C

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void tcaSelect(uint8_t channel) {
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(1000);

  Serial.println("Selecting TCA channel 0 (OLED)");
  tcaSelect(0);
  delay(10);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED NOT detected");
    while (1);
  }

  Serial.println("OLED detected successfully");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  static int counter = 0;

  tcaSelect(0);   // IMPORTANT: always reselect channel

  display.clearDisplay();
  display.setCursor(0, 0);

  display.println("OLED CHECK OK");
  display.println("----------------");
  display.print("Counter: ");
  display.println(counter++);
  display.println();
  display.println("ESP32 + TCA9548A");
  display.println("SSD1306 128x64");

  display.display();
  delay(1000);
}
