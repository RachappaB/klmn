#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <TinyGPSPlus.h>
#include <MAX30105.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <time.h>

/* ================= CONFIG ================= */
#define SDA_PIN 21
#define SCL_PIN 22
#define ONE_WIRE_BUS 4
#define TCA_ADDR 0x70
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/* ================= WIFI ================= */
struct WiFiCredential {
  const char* ssid;
  const char* password;
};

WiFiCredential wifiList[] = {
  {"iot", "password"},
  {"MK 101-103-2.4G", "101102103"},
  {"ACTFIBERNET", "act12345"}
};
const int WIFI_COUNT = sizeof(wifiList) / sizeof(wifiList[0]);

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
int yCursor = 0;

/* GPS acceleration */
float gpsSpeedPrev = 0.0;      // m/s
unsigned long gpsTimePrev = 0; // ms
float gpsAcc = 0.0;            // m/s^2

/* ================= TCA ================= */
void tcaSelect(uint8_t ch) {
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << ch);
  Wire.endTransmission();
}

/* ================= MPU ================= */
#define MPU_ADDR 0x68
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H  0x43
#define PWR_MGMT_1   0x6B

int16_t readMPU(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2);
  return (Wire.read() << 8) | Wire.read();
}

/* ================= WIFI ================= */
bool connectToBestWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(300);

  int n = WiFi.scanNetworks();
  int best = -1, bestRSSI = -999;

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < WIFI_COUNT; j++) {
      if (WiFi.SSID(i) == wifiList[j].ssid &&
          WiFi.RSSI(i) > bestRSSI) {
        bestRSSI = WiFi.RSSI(i);
        best = j;
      }
    }
  }

  if (best < 0) return false;

  WiFi.begin(wifiList[best].ssid, wifiList[best].password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
  }

  return WiFi.status() == WL_CONNECTED;
}

/* ================= RTC FROM NTP ================= */
void syncRTCFromNTP() {
  configTime(19800, 0, "pool.ntp.org"); // IST UTC+5:30
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo, 10000)) {
    Serial.println("NTP FAILED");
    return;
  }

  tcaSelect(7);
  rtc.adjust(DateTime(
    timeinfo.tm_year + 1900,
    timeinfo.tm_mon + 1,
    timeinfo.tm_mday,
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec
  ));

  Serial.println("RTC SYNCED FROM NTP");
}

/* ================= OLED ================= */
void oledLine(const String &s) {
  display.setCursor(0, yCursor);
  display.println(s);
  yCursor += 8;
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  /* OLED */
  tcaSelect(0);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();

  /* RTC */
  tcaSelect(7);
  rtc.begin();

  /* MPU */
  tcaSelect(1);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0);
  Wire.endTransmission();

  /* MAX30102 (SparkFun) */
  tcaSelect(2);
  if (max3010.begin(Wire)) {
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

  /* WIFI + TIME */
  if (connectToBestWiFi()) {
    Serial.println("WiFi CONNECTED");
    syncRTCFromNTP();
  } else {
    Serial.println("WiFi OFFLINE, RTC USED");
  }

  Serial.println("SYSTEM READY");
}

/* ================= LOOP ================= */
void loop() {

  while (GPS.available()) gps.encode(GPS.read());

  /* TIME */
  tcaSelect(7);
  DateTime now = rtc.now();

  /* TEMP */
  tempSensor.requestTemperatures();
  float tempC = tempSensor.getTempCByIndex(0);

  /* MPU */
  tcaSelect(1);
  int ax = readMPU(ACCEL_XOUT_H);
  int ay = readMPU(ACCEL_XOUT_H + 2);
  int az = readMPU(ACCEL_XOUT_H + 4);
  int gx = readMPU(GYRO_XOUT_H);
  int gy = readMPU(GYRO_XOUT_H + 2);
  int gz = readMPU(GYRO_XOUT_H + 4);

  /* MAX30102 */
  uint32_t ir = 0, red = 0;
  if (max3010_ok) {
    tcaSelect(2);
    ir  = max3010.getIR();
    red = max3010.getRed();
  }

  /* GPS ACC CALC */
  float gpsSpeedMS = 0.0;
  if (gps.location.isValid() && gps.speed.isValid()) {
    gpsSpeedMS = gps.speed.mps();
    unsigned long nowMs = millis();

    if (gpsTimePrev > 0) {
      float dt = (nowMs - gpsTimePrev) / 1000.0;
      if (dt > 0.2) {
        gpsAcc = (gpsSpeedMS - gpsSpeedPrev) / dt;
      }
    }

    gpsSpeedPrev = gpsSpeedMS;
    gpsTimePrev = nowMs;
  }

  /* ---------- SERIAL ---------- */
  Serial.println("----- DATA -----");
  Serial.printf("TIME %02d:%02d:%02d\n", now.hour(), now.minute(), now.second());
  Serial.printf("TEMP %.2fC\n", tempC);
  Serial.printf("MPU AX:%d AY:%d AZ:%d GX:%d GY:%d GZ:%d\n",
                ax, ay, az, gx, gy, gz);
  Serial.printf("MAX IR:%lu RED:%lu\n", ir, red);

  if (gps.location.isValid()) {
    Serial.printf(
      "GPS LAT:%.6f LON:%.6f SPD:%.2f m/s ACC:%.2f m/s2 ALT:%.1f SAT:%d\n",
      gps.location.lat(),
      gps.location.lng(),
      gpsSpeedMS,
      gpsAcc,
      gps.altitude.meters(),
      gps.satellites.value()
    );
  } else {
    Serial.println("GPS NO FIX");
  }

  /* ---------- OLED ---------- */
  tcaSelect(0);
  display.clearDisplay();
  yCursor = 0;

  oledLine("TIME " + String(now.hour()) + ":" +
           String(now.minute()) + ":" + String(now.second()));
  oledLine("TEMP " + String(tempC, 1) + "C");

  oledLine("AX " + String(ax) + " AY " + String(ay));
  oledLine("AZ " + String(az));
  oledLine("GX " + String(gx) + " GY " + String(gy));
  oledLine("GZ " + String(gz));

  oledLine("IR " + String(ir));
  oledLine("RED " + String(red));

  if (gps.location.isValid()) {
    oledLine("LAT " + String(gps.location.lat(), 4));
    oledLine("LON " + String(gps.location.lng(), 4));
    oledLine("SPD " + String(gpsSpeedMS, 2) + " m/s");
    oledLine("ACC " + String(gpsAcc, 2) + " m/s2");
    oledLine("ALT " + String(gps.altitude.meters(), 0));
    oledLine("SAT " + String(gps.satellites.value()));
  } else {
    oledLine("GPS NO FIX");
  }

  display.display();
  delay(2000);
}
