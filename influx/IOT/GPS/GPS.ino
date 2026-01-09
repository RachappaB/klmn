#include <TinyGPSPlus.h>

TinyGPSPlus gps;
HardwareSerial GPS(2);

void setup() {
  Serial.begin(115200);
  GPS.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("GPS RAW TEST");
}

void loop() {
  while (GPS.available()) {
    char c = GPS.read();
    Serial.write(c);
  }
}
