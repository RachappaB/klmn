/************************************************************
 * WIFI + INFLUXDB
 ************************************************************/
#if defined(ESP32)
  #include <WiFi.h>
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "ESP32"
#endif

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define INFLUXDB_URL     "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN   "FNF_nx4_x4TbpStzTSRJV3O9SP_dwYqhcifRoLWTVFUd4C-n0NIXBp1IzUmW1SISd9T-x-9QL572WFKB98hGxw=="
#define INFLUXDB_ORG     "861c8c194327e7ec"
#define INFLUXDB_BUCKET  "klmn"

InfluxDBClient client(
  INFLUXDB_URL,
  INFLUXDB_ORG,
  INFLUXDB_BUCKET,
  INFLUXDB_TOKEN,
  InfluxDbCloud2CACert
);

/************************************************************
 * DISPLAY
 ************************************************************/
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/************************************************************
 * SENSOR LIBRARIES
 ************************************************************/
#include <Wire.h>
#include <TinyGPSPlus.h>
#include <MAX30105.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/************************************************************
 * CONFIG
 ************************************************************/
#define SDA_PIN 21
#define SCL_PIN 22
#define ONE_WIRE_BUS 4
#define TCA_ADDR 0x70

#define MPU_ADDR 0x68
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B

#define MPU_SAMPLES 10
#define MPU_DELAY_US 2000

/************************************************************
 * OBJECTS
 ************************************************************/
TinyGPSPlus gps;
HardwareSerial GPS(2);
MAX30105 max3010;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

/************************************************************
 * DATA STRUCT
 ************************************************************/
struct MPUFrame {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
};

MPUFrame mpuData[MPU_SAMPLES];

/************************************************************
 * TCA SELECT
 ************************************************************/
inline void tcaSelect(uint8_t ch) {
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << ch);
  Wire.endTransmission();
}

/************************************************************
 * MPU BURST READ
 ************************************************************/
void readMPUBurst(MPUFrame &f) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (uint8_t)14);

  f.ax = Wire.read() << 8 | Wire.read();
  f.ay = Wire.read() << 8 | Wire.read();
  f.az = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); // temp discard
  f.gx = Wire.read() << 8 | Wire.read();
  f.gy = Wire.read() << 8 | Wire.read();
  f.gz = Wire.read() << 8 | Wire.read();
}

/************************************************************
 * GPS SERVICE
 ************************************************************/
void serviceGPS(uint32_t durationMs) {
  uint32_t start = millis();
  while (millis() - start < durationMs) {
    while (GPS.available()) {
      gps.encode(GPS.read());
    }
    delay(1);
  }
}

/************************************************************
 * HIGH-PRECISION GPS HELPERS (ADDED)
 ************************************************************/
double getLatDouble() {
  if (!gps.location.isValid()) return NAN;
  return gps.location.rawLat().deg +
         gps.location.rawLat().billionths / 1e9;
}

double getLonDouble() {
  if (!gps.location.isValid()) return NAN;
  return gps.location.rawLng().deg +
         gps.location.rawLng().billionths / 1e9;
}

/* 7 decimal places guaranteed (CSV-safe) */
int32_t getLatMicro() {
  if (!gps.location.isValid()) return 0;
  return gps.location.rawLat().deg * 10000000L +
         gps.location.rawLat().billionths / 100;
}

int32_t getLonMicro() {
  if (!gps.location.isValid()) return 0;
  return gps.location.rawLng().deg * 10000000L +
         gps.location.rawLng().billionths / 100;
}

/************************************************************
 * OLED
 ************************************************************/
void showStatusOLED(float temp, uint32_t ir, bool gpsValid) {
  tcaSelect(0);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.printf("Temp: %.1f C", temp);

  display.setCursor(0, 16);
  display.printf("IR: %lu", ir);

  display.setCursor(0, 32);
  display.printf("GPS: %s", gpsValid ? "FIX" : "NO FIX");

  display.display();
}

/************************************************************
 * SETUP
 ************************************************************/
void setup() {

  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("iot", "password");
  wifiMulti.addAP("MK 206-207-2.4G", "206207000");
  wifiMulti.addAP("ACTFIBERNET", "act12345");

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(300);
  }

  client.validateConnection();

  tcaSelect(0);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

  tcaSelect(1);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0x00);
  Wire.endTransmission();

  tcaSelect(2);
  max3010.begin(Wire);
  max3010.setup();
  max3010.setPulseAmplitudeRed(0x3F);
  max3010.setPulseAmplitudeIR(0x3F);

  tempSensor.begin();

  GPS.begin(9600, SERIAL_8N1, 16, 17);
}

/************************************************************
 * LOOP
 ************************************************************/
void loop() {

  /* ---------- GPS FIRST ---------- */
  serviceGPS(1200);
  bool gpsValid = gps.location.isValid();

  /* ---------- MPU ---------- */
  tcaSelect(1);
  for (int i = 0; i < MPU_SAMPLES; i++) {
    readMPUBurst(mpuData[i]);
    delayMicroseconds(MPU_DELAY_US);
  }

  /* ---------- TEMP ---------- */
  tempSensor.requestTemperatures();
  float temp = tempSensor.getTempCByIndex(0);

  /* ---------- MAX30102 ---------- */
  tcaSelect(2);
  uint32_t ir = max3010.getIR();
  uint32_t red = max3010.getRed();

  /* ---------- OLED ---------- */
  showStatusOLED(temp, ir, gpsValid);

  /* ---------- INFLUX ---------- */
  Point p("activity_frame");
  p.addTag("device", DEVICE);

  for (int i = 0; i < MPU_SAMPLES; i++) {
    p.addField("ax" + String(i), mpuData[i].ax);
    p.addField("ay" + String(i), mpuData[i].ay);
    p.addField("az" + String(i), mpuData[i].az);
    p.addField("gx" + String(i), mpuData[i].gx);
    p.addField("gy" + String(i), mpuData[i].gy);
    p.addField("gz" + String(i), mpuData[i].gz);
  }

  if (gpsValid) {
    p.addField("lat_double", getLatDouble());
    p.addField("lon_double", getLonDouble());

    p.addField("lat_micro", getLatMicro());
    p.addField("lon_micro", getLonMicro());

    p.addField("speed_mps", gps.speed.mps());
    p.addField("altitude_m", gps.altitude.meters());
    p.addField("satellites", gps.satellites.value());
    p.addField("hdop", gps.hdop.hdop());
  }

  p.addField("ir", ir);
  p.addField("red", red);
  p.addField("temp", temp);

  client.writePoint(p);

  delay(1000);
}
