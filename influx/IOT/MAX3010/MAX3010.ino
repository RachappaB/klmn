#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MAX30105.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define TCA_ADDR 0x70

// OLED
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// MAX30102
MAX30105 particleSensor;

void tcaSelect(uint8_t channel) {
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(1000);

  // ---------- OLED INIT ----------
  tcaSelect(0);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED NOT detected");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // ---------- MAX30102 INIT ----------
  tcaSelect(2);
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 NOT detected");
    while (1);
  }

  particleSensor.setup();               // default configuration
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);
  particleSensor.setPulseAmplitudeGreen(0); // not used

  Serial.println("MAX30102 + OLED READY");
}

void loop() {
  // Read MAX30102
  tcaSelect(2);
  uint32_t irValue  = particleSensor.getIR();
  uint32_t redValue = particleSensor.getRed();

  // Update OLED
  tcaSelect(0);
  display.clearDisplay();
  display.setCursor(0, 0);

  display.println("MAX30102 DATA");
  display.println("----------------");
  display.print("IR  : ");
  display.println(irValue);
  display.print("RED : ");
  display.println(redValue);
  display.println();
  display.println("Place finger");

  display.display();
  delay(500);
}
