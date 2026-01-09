/************************************************************
 * WIFI + INFLUXDB SECTION
 ************************************************************/

#if defined(ESP32)
  #include <WiFi.h>
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "ESP32"
#endif

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

  #define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
  #define INFLUXDB_TOKEN "JzDKGMssayKklROw-z5jBtw0_T04q-pH5ORXFlCsL4wZWU9b4msMoaGhK5bBb8--NWvJ2Rjeo9Rz0MtLXt7jKg=="
  #define INFLUXDB_ORG "861c8c194327e7ec"
  #define INFLUXDB_BUCKET "iot"
  

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
  Serial.printf("[I2C] Selecting TCA channel %d\n", ch);
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

  Wire.read(); Wire.read(); // skip temp

  f.gx = Wire.read() << 8 | Wire.read();
  f.gy = Wire.read() << 8 | Wire.read();
  f.gz = Wire.read() << 8 | Wire.read();
}

/************************************************************
 * OLED
 ************************************************************/
void showStatusOLED(float temp, uint32_t ir) {

  Serial.println("[OLED] Updating display");

  tcaSelect(0);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.setTextSize(1);
  display.printf("Temp: %.1f C", temp);

  display.setCursor(0, 16);
  display.printf("IR: %lu", ir);

  display.display();
}

/************************************************************
 * SETUP
 ************************************************************/
void setup() {

  Serial.begin(115200);
  Serial.println("\n=== SYSTEM BOOT START ===");

  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("[I2C] Bus initialized");

  WiFi.mode(WIFI_STA);
  Serial.println("[WiFi] Station mode set");

  wifiMulti.addAP("iot", "password");
  wifiMulti.addAP("MK 101-103-2.4G", "101102103");
  wifiMulti.addAP("ACTFIBERNET", "act12345");

  Serial.print("[WiFi] Connecting");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\n[WiFi] Connected");
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());

  tcaSelect(0);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  Serial.println("[OLED] Initialized");

  tcaSelect(1);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0);
  Wire.endTransmission();
  Serial.println("[MPU] MPU6050 awakened");

  tcaSelect(2);
  if (max3010.begin(Wire)) {
    max3010.setup();
    max3010.setPulseAmplitudeRed(0x3F);
    max3010.setPulseAmplitudeIR(0x3F);
    Serial.println("[MAX30102] Initialized");
  } else {
    Serial.println("[MAX30102] NOT FOUND");
  }

  tempSensor.begin();
  Serial.println("[TEMP] DS18B20 initialized");

  GPS.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("[GPS] Serial started");

  Serial.println("=== SYSTEM READY ===");
}

/************************************************************
 * LOOP
 ************************************************************/
void loop() {

  Serial.println("\n--- LOOP START ---");

  while (GPS.available()) {
    gps.encode(GPS.read());
  }

  Serial.println("[MPU] Sampling data");
  tcaSelect(1);
  for (int i = 0; i < MPU_SAMPLES; i++) {
    readMPUBurst(mpuData[i]);
    delayMicroseconds(MPU_DELAY_US);
  }

  tempSensor.requestTemperatures();
  float temp = tempSensor.getTempCByIndex(0);
  Serial.printf("[TEMP] %.2f C\n", temp);

  tcaSelect(2);
  uint32_t ir = max3010.getIR();
  uint32_t red = max3010.getRed();
  Serial.printf("[MAX30102] IR=%lu RED=%lu\n", ir, red);

  bool gpsValid = gps.location.isValid();
  Serial.printf("[GPS] Valid=%d Sats=%d\n", gpsValid, gps.satellites.value());

  showStatusOLED(temp, ir);

  Serial.println("[INFLUX] Preparing data point");
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
    p.addField("lat", gps.location.lat());
    p.addField("lon", gps.location.lng());
    p.addField("speed", gps.speed.kmph());
    p.addField("altitude", gps.altitude.meters());
    p.addField("satellites", gps.satellites.value());
  }

  p.addField("ir", ir);
  p.addField("red", red);
  p.addField("temp", temp);

  Serial.println("[INFLUX] Writing point");
  if (!client.writePoint(p)) {
    Serial.print("[INFLUX] ERROR: ");
    Serial.println(client.getLastErrorMessage());
  } else {
    Serial.println("[INFLUX] Write OK");
  }

  Serial.println("--- LOOP END ---");
  delay(200);
}
