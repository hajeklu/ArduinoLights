/**
   SpringStompDHTSender.ino

   This example listens to two STOMP server topics, "/commands/blink" and "/commands/sample".

   In response it either blinks the on-board LED, or sends temperature and humidity samples from an attached DHT22 device.

   Works best when used with the Spring Websockets+Stomp example code:

   Assuming a NodeMCU device, the DHT22 should be connected:

   NodeMcu   | DHT22
     3V      |   Pin 1
     D3      |   Pin2
     GND     |   Pin4

   A 10K resistor should be connected between the DHT22 pins 1 and 2.
   DHT22 pin 3 is left unconnected.


   Author: Duncan McIntyre <duncan@calligram.co.uk>

*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include "StompClient.h"
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;

/**
  WiFi settings
**/
const char* wlan_ssid             = "AP_Hajkovi";
const char* wlan_password         = "hajkovi01";


int errorLED                      = D6;
int lightLED                      = D5;

/**
  Stomp server settings
**/
bool useWSS                       = false;
const char* ws_host               = "192.168.1.106";
const int ws_port                 = 8080;
const char* ws_baseurl            = "/svetla/websocket/"; // don't forget leading and trailing "/" !!!
bool isConfigured                 = false;
WebSocketsClient webSocket;

Stomp::StompClient stomper(webSocket, ws_host, ws_port, ws_baseurl, true);


void setup() {

  // We'll flash this led whenever a message is received
  pinMode(lightLED, OUTPUT);
  pinMode(errorLED, OUTPUT);


  // setup serial
  Serial.begin(115200);
  Serial.begin(9600);

  // flush it - ESP Serial seems to start with rubbish
  Serial.println();

  // connect to WiFi
  Serial.println("Logging into WLAN: " + String(wlan_ssid));
  Serial.print(" ...");
  WiFi.begin(wlan_ssid, wlan_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" success.");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  // Start the StompClient
  stomper.onConnect(subscribe);
  stomper.onError(error);

  if (useWSS) {
    stomper.beginSSL();
  } else {
    stomper.begin();
  }
}

// Once the Stomp connection has been made, subscribe to a topic
void subscribe(Stomp::StompCommand cmd) {
  Serial.println("Connected to STOMP broker");
  stomper.subscribe("/topic/status", Stomp::CLIENT, handleBlinkMessage);
}

void error(const Stomp::StompCommand cmd) {
  Serial.println("ERROR: " + cmd.body);
  stomper.onConnect(subscribe);
  ESP.restart();
}

Stomp::Stomp_Ack_t handleBlinkMessage(const Stomp::StompCommand cmd) {
  Serial.println("Got a message!");
  Serial.println(cmd.body);
  digitalWrite(errorLED, LOW);
  setLight(cmd.body);

  return Stomp::CONTINUE;
}

void setLight(String status) {
  if (status == "TURNON" || status == "\"TURNON\"") {
    digitalWrite(lightLED, HIGH);
  } else if (status == "TURNOFF" || status == "\"TURNOFF\"") {
    digitalWrite(lightLED , LOW);
  }
}

void initalSetup() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://192.168.1.106:8080/svetla/light")) {  // HTTP


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
          setLight(payload);
          isConfigured = true;
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }
}

void loop() {
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    if (!isConfigured) {
      initalSetup();
    }
    digitalWrite(errorLED, LOW);
    webSocket.loop();
  } else {
    digitalWrite(errorLED, HIGH);
    ESP.restart();
  }
}
