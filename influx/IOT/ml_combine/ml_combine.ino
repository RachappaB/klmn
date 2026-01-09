#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <TinyGPSPlus.h>
#include <MAX30105.h>              // SparkFun library
#include <OneWire.h>
#include <DallasTemperature.h>

/* ================= CONFIG ================= */
#define SDA_PIN 21
#define SCL_PIN 22
#define ONE_WIRE_BUS 4
#define TCA_ADDR 0x70
#define OLED_ADDR 0x3C

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define MPU_ADDR 0x68
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H  0x43
#define PWR_MGMT_1   0x6B

#define MPU_SAMPLES 10
#define MPU_SAMPLE_INTERVAL_MS 300   // 10 samples ≈ 3 sec

/* ================= OBJECTS ================= */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RTC_DS3231 rtc;
TinyGPSPlus gps;
HardwareSerial GPS(2);
MAX30105 max3010;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

/* ================= STATE ================= */
bool max3010_ok = false;

/* ================= DATA STRUCT ================= */
struct MPUFrame {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
};

MPUFrame mpuData[MPU_SAMPLES];

/* ================= TCA ================= */
void tcaSelect(uint8_t ch) {
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << ch);
  Wire.endTransmission();
}

/* ================= MPU ================= */
int16_t readMPU(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2);
  return (Wire.read() << 8) | Wire.read();
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  /* OLED */
  tcaSelect(0);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  /* RTC */
  tcaSelect(7);
  rtc.begin();

  /* MPU */
  tcaSelect(1);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0);
  Wire.endTransmission();

  /* MAX30102 */
  tcaSelect(2);
  if (max3010.begin(Wire, I2C_SPEED_FAST)) {
    max3010.setup();
    max3010_ok = true;
    Serial.println("MAX30102 READY");
  } else {
    Serial.println("MAX30102 NOT FOUND");
  }

  /* TEMP */
  tempSensor.begin();

  /* GPS */
  GPS.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("SYSTEM READY");
}

/* ================= LOOP ================= */
void loop() {

  /* ========= MPU CONTINUOUS WINDOW ========= */
  for (int i = 0; i < MPU_SAMPLES; i++) {
    tcaSelect(1);
    mpuData[i].ax = readMPU(ACCEL_XOUT_H);
    mpuData[i].ay = readMPU(ACCEL_XOUT_H + 2);
    mpuData[i].az = readMPU(ACCEL_XOUT_H + 4);
    mpuData[i].gx = readMPU(GYRO_XOUT_H);
    mpuData[i].gy = readMPU(GYRO_XOUT_H + 2);
    mpuData[i].gz = readMPU(GYRO_XOUT_H + 4);
    delay(MPU_SAMPLE_INTERVAL_MS);
  }

  /* ========= GPS (ONCE) ========= */
  while (GPS.available()) gps.encode(GPS.read());

  float lat = 0, lon = 0, spd = 0, alt = 0;
  int sats = 0;
  if (gps.location.isValid()) {
    lat = gps.location.lat();
    lon = gps.location.lng();
    spd = gps.speed.mps();
    alt = gps.altitude.meters();
    sats = gps.satellites.value();
  }

  /* ========= HEART ========= */
  uint32_t ir = 0, red = 0;
  if (max3010_ok) {
    tcaSelect(2);
    ir  = max3010.getIR();
    red = max3010.getRed();
  }

  /* ========= TEMP ========= */
  tempSensor.requestTemperatures();
  float tempC = tempSensor.getTempCByIndex(0);

  /* ========= TIME ========= */
  tcaSelect(7);
  DateTime now = rtc.now();

  /* ========= ONE ACTIVITY FRAME ========= */
  Serial.println("===== ACTIVITY_FRAME_START =====");

  Serial.printf("TIME,%04d-%02d-%02d %02d:%02d:%02d\n",
                now.year(), now.month(), now.day(),
                now.hour(), now.minute(), now.second());

  Serial.printf("TEMP,%.2f\n", tempC);
  Serial.printf("HEART,IR:%lu,RED:%lu\n", ir, red);
  Serial.printf("GPS,%.6f,%.6f,%.2f,%.1f,%d\n",
                lat, lon, spd, alt, sats);

  for (int i = 0; i < MPU_SAMPLES; i++) {
    Serial.printf("MPU,%d,%d,%d,%d,%d,%d\n",
                  mpuData[i].ax,
                  mpuData[i].ay,
                  mpuData[i].az,
                  mpuData[i].gx,
                  mpuData[i].gy,
                  mpuData[i].gz);
  }

  Serial.println("===== ACTIVITY_FRAME_END =====\n");
}
