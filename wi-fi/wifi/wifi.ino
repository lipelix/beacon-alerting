#include <ArduinoWebsockets.h> // https://github.com/gilmaimon/ArduinoWebsockets
#include <ESP8266WiFi.h>

using namespace websockets;

#define SERIAL_DEBUG_BAUD 9600
#define WIFI_SSID "<SSID>"
#define WIFI_PASSWORD "<PASSWORD>"
#define WEBSOCKETS_SERVER_HOST "beacon-alerts.lipelix.workers.dev"
#define WEBSOCKETS_SERVER_PORT 80
#define BEACON_PIN 5 /* GPIO5 (D1) for BEACON_PIN */

WebsocketsClient client;

void turnBeaconOn() {
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(BEACON_PIN, HIGH);
  Serial.println("Turn beacon ON");
}

void turnBeaconOff() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(BEACON_PIN, LOW);
  Serial.println("Turn beacon OFF");
}

void initWiFi() {
  Serial.print("Trying to connect to wi-fi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(".");
  }
  wifiConnectSuccessMessage();
}

void wifiConnectSuccessMessage() {
  Serial.println("");
  Serial.println("====================");
  Serial.println("Wi-fi connected ");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("====================");
}

void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Incomming Message data: ");
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
  
  // initialize output pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BEACON_PIN, OUTPUT);

  initWiFi();

  // try to connect to Websockets server
  bool connected = client.connect(WEBSOCKETS_SERVER_HOST, WEBSOCKETS_SERVER_PORT, "/");
  if (connected)
  {
    Serial.println("====================");
    Serial.print("Connected to: ");
    Serial.println(WEBSOCKETS_SERVER_HOST);
    client.send("Hello Server");
  }
  else
  {
    Serial.println("====================");
    Serial.print("Connection to webserver failed: ");
    Serial.println(WEBSOCKETS_SERVER_HOST);
  }

  // run callback when messages are received
  client.onMessage(onMessageCallback);
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(WIFI_SSID);
    
    for (int i = 0; i <= 3; i++) {
      Serial.print("Retry Wi-fi connection ");
      Serial.print(i);
      Serial.print(" - Wifi connection unavailable for: ");
      Serial.println(WIFI_SSID);
      digitalWrite(LED_BUILTIN, LOW);
      delay(5000);
      digitalWrite(LED_BUILTIN, HIGH);  // turn led off

      if (WiFi.status() == WL_CONNECTED) {
        break;
      }

      if (i == 3) {
        Serial.print("Will restart - Wifi connection unavailable");
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
      Serial.print("Retry Websocket connection ");
      Serial.print(i);
      Serial.print(" - Websocket connection unavailable for: ");
      Serial.println(WEBSOCKETS_SERVER_HOST);
      digitalWrite(LED_BUILTIN, LOW);
      delay(5000);
      digitalWrite(LED_BUILTIN, HIGH); // turn led off

      client.connect(WEBSOCKETS_SERVER_HOST, WEBSOCKETS_SERVER_PORT, "/");

      if (client.available()) {
        break;
      }

      if (i == 3) {
        Serial.print("Will restart - Websocket connection unavailable");
        ESP.reset();
      }
    }
  }

  delay(500);
}