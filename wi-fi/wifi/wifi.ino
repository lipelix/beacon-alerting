#include <ArduinoWebsockets.h> // https://github.com/gilmaimon/ArduinoWebsockets
#include <ESP8266WiFi.h>
#include <time.h>

using namespace websockets;

#define SERIAL_DEBUG_BAUD 9600
#define WIFI_SSID "<SSID>"
#define WIFI_PASSWORD "<PASSWORD>"
// Optional: Specify BSSID (MAC address) of specific AP to connect to
// Leave empty "" to auto-select strongest signal
// Format: "AA:BB:CC:DD:EE:FF"
#define WIFI_BSSID ""
#define WEBSOCKETS_SERVER_HOST "beacon-alerts.lipelix.workers.dev"
#define WEBSOCKETS_SERVER_PORT 80
#define BEACON_PIN 5 /* GPIO5 (D1) for BEACON_PIN */

// NTP configuration
#define NTP_SERVER "pool.ntp.org"
#define TIMEZONE_OFFSET 3600  // UTC+1 (Central European Time) in seconds. Adjust as needed: UTC+2 = 7200, UTC-5 = -18000, etc.
#define DST_OFFSET 0          // Daylight saving time offset in seconds (3600 for DST, 0 for no DST)

WebsocketsClient client;
bool ntpSynced = false;

String getTimestamp() {
  char timestamp[24];

  if (ntpSynced) {
    // Use real time from NTP
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);

    sprintf(timestamp, "[%04d-%02d-%02d %02d:%02d:%02d]",
            timeinfo->tm_year + 1900,
            timeinfo->tm_mon + 1,
            timeinfo->tm_mday,
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec);
  } else {
    // Fallback to uptime-based timestamp
    unsigned long ms = millis();
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;

    sprintf(timestamp, "[%02lu:%02lu:%02lu.%03lu]",
            hours % 24,
            minutes % 60,
            seconds % 60,
            ms % 1000);
  }

  return String(timestamp);
}

void logPrint(String message) {
  Serial.print(getTimestamp());
  Serial.print(" ");
  Serial.print(message);
}

void logPrintln(String message) {
  Serial.print(getTimestamp());
  Serial.print(" ");
  Serial.println(message);
}

void syncNTP() {
  logPrint("Syncing time with NTP server: ");
  Serial.println(NTP_SERVER);

  configTime(TIMEZONE_OFFSET, DST_OFFSET, NTP_SERVER);

  // Wait for time to be set (max 10 seconds)
  int retries = 0;
  time_t now = time(nullptr);
  while (now < 1000000000 && retries < 20) {
    delay(500);
    now = time(nullptr);
    retries++;
    Serial.print(".");
  }
  Serial.println();

  if (now > 1000000000) {
    ntpSynced = true;
    logPrint("NTP sync successful. Current time: ");
    Serial.println(getTimestamp());
  } else {
    logPrintln("NTP sync failed. Using uptime-based timestamps.");
  }
}

void turnBeaconOn() {
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(BEACON_PIN, HIGH);
  logPrintln("Turn beacon ON");
}

void turnBeaconOff() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(BEACON_PIN, LOW);
  logPrintln("Turn beacon OFF");
}

int findStrongestAP(const char* ssid) {
  int n = WiFi.scanNetworks();
  int bestIndex = -1;
  int bestRSSI = -999;

  for (int i = 0; i < n; i++) {
    if (WiFi.SSID(i) == ssid) {
      if (WiFi.RSSI(i) > bestRSSI) {
        bestRSSI = WiFi.RSSI(i);
        bestIndex = i;
      }
    }
  }
  return bestIndex;
}

void scanAndPrintNetworks() {
  Serial.println();
  logPrintln("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();

  if (n == 0) {
    logPrintln("No networks found");
  } else {
    logPrint("Found ");
    Serial.print(n);
    Serial.println(" networks:");

    int targetCount = 0;
    int strongestIndex = -1;
    int strongestRSSI = -999;

    for (int i = 0; i < n; i++) {
      logPrint("  ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(" dBm) ");

      // Print detailed encryption type
      uint8_t encType = WiFi.encryptionType(i);
      switch(encType) {
        case ENC_TYPE_NONE:
          Serial.print("Open");
          break;
        case ENC_TYPE_WEP:
          Serial.print("WEP");
          break;
        case ENC_TYPE_TKIP:
          Serial.print("WPA-TKIP");
          break;
        case ENC_TYPE_CCMP:
          Serial.print("WPA2-CCMP");
          break;
        case ENC_TYPE_AUTO:
          Serial.print("WPA/WPA2-Auto");
          break;
        default:
          Serial.print("Unknown(");
          Serial.print(encType);
          Serial.print(")");
      }

      Serial.print(" [");
      Serial.print(WiFi.BSSIDstr(i));
      Serial.print("]");

      // Highlight if this is our target network
      if (WiFi.SSID(i) == WIFI_SSID) {
        targetCount++;
        if (WiFi.RSSI(i) > strongestRSSI) {
          strongestRSSI = WiFi.RSSI(i);
          strongestIndex = i;
        }
        Serial.print(" <- TARGET");
      }
      Serial.println();
    }

    if (targetCount > 1) {
      Serial.println();
      logPrint("*** WARNING: Found ");
      Serial.print(targetCount);
      Serial.print(" APs with SSID '");
      Serial.print(WIFI_SSID);
      Serial.println("' ***");
      logPrint("*** Strongest signal: ");
      Serial.print(WiFi.BSSIDstr(strongestIndex));
      Serial.print(" (");
      Serial.print(strongestRSSI);
      Serial.println(" dBm) ***");
      logPrintln("*** Will auto-connect to strongest, or set WIFI_BSSID to target specific AP ***");
    }
  }
  Serial.println();
}

void initWiFi() {
  Serial.println("====================");
  logPrintln("WiFi Diagnostics");
  Serial.println("====================");
  logPrint("Target SSID: ");
  Serial.println(WIFI_SSID);
  logPrint("Password length: ");
  Serial.println(strlen(WIFI_PASSWORD));

  if (strlen(WIFI_BSSID) > 0) {
    logPrint("Target BSSID: ");
    Serial.println(WIFI_BSSID);
  }

  logPrint("WiFi mode: ");
  Serial.println(WiFi.getMode());

  // Scan for networks first
  scanAndPrintNetworks();

  logPrintln("Attempting to connect...");
  WiFi.mode(WIFI_STA);  // Explicitly set to station mode
  WiFi.disconnect();    // Clear any previous connection
  delay(100);

  // If BSSID is specified, use it. Otherwise find strongest AP
  if (strlen(WIFI_BSSID) > 0) {
    // Connect to specific BSSID
    uint8_t bssid[6];
    sscanf(WIFI_BSSID, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]);
    logPrint("Connecting to specific BSSID: ");
    Serial.println(WIFI_BSSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 0, bssid, true);
  } else {
    // Auto-select strongest AP
    int strongestAP = findStrongestAP(WIFI_SSID);
    if (strongestAP >= 0) {
      logPrint("Auto-connecting to strongest AP: ");
      Serial.print(WiFi.BSSIDstr(strongestAP));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(strongestAP));
      Serial.println(" dBm)");

      uint8_t* bssid = WiFi.BSSID(strongestAP);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 0, bssid, true);
    } else {
      logPrintln("Target network not found in scan, trying anyway...");
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
  }

  int attempt = 0;
  int delayMs = 1000;
  const int maxDelay = 8000;
  const int maxAttempts = 20;  // Limit attempts before rescanning

  while (WiFi.status() != WL_CONNECTED){
    attempt++;

    // After 20 attempts, rescan and show diagnostics
    if (attempt == maxAttempts) {
      Serial.println();
      Serial.println("====================");
      logPrintln("Connection failed after multiple attempts. Rescanning...");
      scanAndPrintNetworks();
      logPrintln("Retrying connection with fresh scan...");
      WiFi.disconnect();
      delay(100);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      attempt = 0;  // Reset counter
    }

    // Toggle LED during wait
    for (int i = 0; i < delayMs / 500; i++) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(500);
    }

    // Print detailed status
    Serial.println();
    logPrint("WiFi attempt ");
    Serial.print(attempt);
    Serial.print(" (wait ");
    Serial.print(delayMs / 1000.0, 1);
    Serial.print("s) - Signal: ");
    int rssi = WiFi.RSSI();
    Serial.print(rssi);
    Serial.print(" dBm");

    // Add signal quality indicator
    if (rssi >= -50) {
      Serial.print(" (Excellent)");
    } else if (rssi >= -60) {
      Serial.print(" (Good)");
    } else if (rssi >= -70) {
      Serial.print(" (Fair)");
    } else if (rssi >= -80) {
      Serial.print(" (Weak)");
    } else if (rssi < -80 && rssi != 0) {
      Serial.print(" (Very Weak)");
    }

    Serial.print(" - Status: ");
    Serial.print(WiFi.status());
    Serial.print(" (");
    switch(WiFi.status()) {
      case WL_IDLE_STATUS:
        Serial.print("IDLE");
        break;
      case WL_NO_SSID_AVAIL:
        Serial.print("NO SSID AVAILABLE - Network not found!");
        break;
      case WL_SCAN_COMPLETED:
        Serial.print("SCAN COMPLETED");
        break;
      case WL_CONNECT_FAILED:
        Serial.print("CONNECT FAILED - Check password!");
        break;
      case WL_CONNECTION_LOST:
        Serial.print("CONNECTION LOST");
        break;
      case WL_DISCONNECTED:
        Serial.print("DISCONNECTED - Check SSID and password");
        break;
      default:
        Serial.print("UNKNOWN");
    }
    Serial.print(")");

    // Exponential backoff: double delay each time, up to maxDelay
    delayMs = min(delayMs * 2, maxDelay);
  }
  wifiConnectSuccessMessage();
}

void wifiConnectSuccessMessage() {
  Serial.println("");
  Serial.println("====================");
  logPrintln("Wi-fi connected ");
  logPrint("SSID: ");
  Serial.println(WIFI_SSID);
  logPrint("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("====================");

  // Quick blink 2 times to indicate success
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_BUILTIN, LOW);  // LED on
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);  // LED off
    delay(100);
  }

  // Sync time with NTP server
  syncNTP();
}

void onMessageCallback(WebsocketsMessage message) {
    logPrint("Incoming Message data: ");
    Serial.println(message.data());
    const String msg = message.data();
    if (msg == "OFF") {
      turnBeaconOff();
    }
    if (msg == "ON") {
      turnBeaconOn();
    }
  }

void setup()
{
  Serial.begin(SERIAL_DEBUG_BAUD);
  delay(100);

  logPrintln("========== BOOT ==========");
  logPrintln("Beacon Alert System Starting...");

  // initialize output pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BEACON_PIN, OUTPUT);

  initWiFi();

  // try to connect to Websockets server
  bool connected = client.connect(WEBSOCKETS_SERVER_HOST, WEBSOCKETS_SERVER_PORT, "/");
  if (connected)
  {
    Serial.println("====================");
    logPrint("Connected to: ");
    Serial.println(WEBSOCKETS_SERVER_HOST);
    client.send("Hello Server");
  }
  else
  {
    Serial.println("====================");
    logPrint("Connection to webserver failed: ");
    Serial.println(WEBSOCKETS_SERVER_HOST);
  }

  // run callback when messages are received
  client.onMessage(onMessageCallback);

  logPrintln("Setup complete");
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
    logPrint("WiFi disconnected from: ");
    Serial.println(WIFI_SSID);

    for (int i = 0; i <= 3; i++) {
      logPrint("Retry Wi-fi connection ");
      Serial.print(i);
      Serial.print(" - Wifi connection unavailable for: ");
      Serial.println(WIFI_SSID);
      digitalWrite(LED_BUILTIN, LOW);
      delay(5000);
      digitalWrite(LED_BUILTIN, HIGH);  // turn led off

      if (WiFi.status() == WL_CONNECTED) {
        logPrintln("WiFi reconnected successfully");
        break;
      }

      if (i == 3) {
        logPrintln("Will restart - Wifi connection unavailable");
        ESP.reset();
      }
    }
  }

  // let the websockets client check for incoming messages
  if (client.available()) {
    client.poll();
    digitalWrite(LED_BUILTIN, HIGH); // turn led off
  }
  else {
    for (int i = 0; i <= 3; i++) {
      logPrint("Retry Websocket connection ");
      Serial.print(i);
      Serial.print(" - Websocket connection unavailable for: ");
      Serial.println(WEBSOCKETS_SERVER_HOST);
      digitalWrite(LED_BUILTIN, LOW);
      delay(5000);
      digitalWrite(LED_BUILTIN, HIGH); // turn led off

      client.connect(WEBSOCKETS_SERVER_HOST, WEBSOCKETS_SERVER_PORT, "/");

      if (client.available()) {
        logPrintln("Websocket reconnected successfully");
        break;
      }

      if (i == 3) {
        logPrintln("Will restart - Websocket connection unavailable");
        ESP.reset();
      }
    }
  }

  delay(500);
}