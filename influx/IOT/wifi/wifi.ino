#include <WiFi.h>

// ---------- WIFI CREDENTIALS ----------
struct WiFiCredential {
  const char* ssid;
  const char* password;
};

WiFiCredential wifiList[] = {
  { "iot", "password" },
  { "MK 101-103-2.4G", "101102103" },
  { "ACTFIBERNET", "act12345" }
};

const int WIFI_COUNT = sizeof(wifiList) / sizeof(wifiList[0]);

// ---------- CONFIG ----------
#define WIFI_TIMEOUT_MS 15000
#define WIFI_RETRY_DELAY 500

void connectToBestWiFi() {
  Serial.println("Scanning WiFi networks...");
  int n = WiFi.scanNetworks();

  if (n <= 0) {
    Serial.println("No WiFi networks found");
    return;
  }

  int bestIndex = -1;
  int bestRSSI = -999;

  for (int i = 0; i < n; i++) {
    String foundSSID = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);

    for (int j = 0; j < WIFI_COUNT; j++) {
      if (foundSSID == wifiList[j].ssid) {
        if (rssi > bestRSSI) {
          bestRSSI = rssi;
          bestIndex = j;
        }
      }
    }
  }

  if (bestIndex == -1) {
    Serial.println("No known WiFi networks available");
    return;
  }

  Serial.print("Connecting to ");
  Serial.println(wifiList[bestIndex].ssid);

  WiFi.begin(wifiList[bestIndex].ssid,
             wifiList[bestIndex].password);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < WIFI_TIMEOUT_MS) {
    Serial.print(".");
    delay(WIFI_RETRY_DELAY);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());
  } else {
    Serial.println("\nWiFi connection failed");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(500);

  connectToBestWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, retrying...");
    connectToBestWiFi();
  }

  delay(10000);
}
