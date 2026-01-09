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
  #define INFLUXDB_TOKEN "6XnZ0lw-UoVmBVruio2IAcd8rqDcNk55k5SwIKN3XJEpGEVLrY2ZRz3WkSAsynkwYcfyamN-NzbuzCobTAkD7w=="
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

  Wire.read(); Wire.read(); // skip temp

  f.gx = Wire.read() << 8 | Wire.read();
  f.gy = Wire.read() << 8 | Wire.read();
  f.gz = Wire.read() << 8 | Wire.read();
}

/************************************************************
 * SETUP
 ************************************************************/
void setup() {

  Serial.begin(115200);
  Serial.println("\n=== SYSTEM BOOT START ===");

  Wire.begin(SDA_PIN, SCL_PIN);

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("iot", "password");
  wifiMulti.addAP("MK 206-207-2.4G", "206207000");
  wifiMulti.addAP("ACTFIBERNET", "act12345");

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(300);
  }

  tcaSelect(1);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0);
  Wire.endTransmission();

  tcaSelect(2);
  max3010.begin(Wire);
  max3010.setup();
  max3010.setPulseAmplitudeRed(0x3F);
  max3010.setPulseAmplitudeIR(0x3F);

  tempSensor.begin();

  GPS.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("=== SYSTEM READY ===");
}

/************************************************************
 * LOOP
 ************************************************************/
void loop() {

  while (GPS.available()) {
    gps.encode(GPS.read());
  }

  tcaSelect(1);
  for (int i = 0; i < MPU_SAMPLES; i++) {
    readMPUBurst(mpuData[i]);
    delayMicroseconds(MPU_DELAY_US);
  }

  tempSensor.requestTemperatures();
  float temp = tempSensor.getTempCByIndex(0);

  tcaSelect(2);
  uint32_t ir = max3010.getIR();
  uint32_t red = max3010.getRed();

  bool gpsValid = gps.location.isValid();

  Point p("activity_frame");

  for (int i = 0; i < MPU_SAMPLES; i++) {
    p.addField("ax" + String(i), mpuData[i].ax);
    p.addField("ay" + String(i), mpuData[i].ay);
    p.addField("az" + String(i), mpuData[i].az);
    p.addField("gx" + String(i), mpuData[i].gx);
    p.addField("gy" + String(i), mpuData[i].gy);
    p.addField("gz" + String(i), mpuData[i].gz);
  }

  if (gpsValid) {
    double lat = gps.location.lat();
    double lon = gps.location.lng();

    // force 6-digit precision
    lat = round(lat * 1000000.0) / 1000000.0;
    lon = round(lon * 1000000.0) / 1000000.0;

    p.addField("lat", lat);
    p.addField("lon", lon);
    p.addField("speed", gps.speed.kmph());
    p.addField("altitude", gps.altitude.meters());
    p.addField("satellites", gps.satellites.value());
  }

  p.addField("ir", ir);
  p.addField("red", red);
  p.addField("temp", temp);

  client.writePoint(p);

  delay(2000);
}
