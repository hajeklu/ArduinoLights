/*
   Circuits4you.com
   Get IP Address of ESP8266 in Arduino IDE
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

const char* wifiName = "YZOlogicka";
const char* wifiPass = "hajkovi01";

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  pinMode(D7, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D5, OUTPUT);


  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();

  Serial.print("Connecting to ");
  Serial.println(wifiName);

  WiFi.begin(wifiName, wifiPass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(D7, HIGH);
  }


  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());   //You can get IP address assigned to ESP

}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(D6, HIGH);
    Serial.print("Connected to: ");
    Serial.println(wifiName);


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
          char json[50];
          payload.toCharArray(json, 50);
          DynamicJsonDocument doc(1024);
          DeserializationError error = deserializeJson(doc, json);
          if (error) {
            digitalWrite(D5, LOW);
            blikRedLED();
            Serial.println("JSON parsing error!");
            return;
          }

          String value = payload;
          value.replace("\"","");
          if (value == "TURNON") {
            digitalWrite(D5, HIGH);
          } else if (value == "TURNOFF") {
            digitalWrite(D5, LOW);
          }
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        digitalWrite(D5, LOW);
        blikRedLED();
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
      digitalWrite(D5, LOW);
      blikRedLED();
    }

    delay(1000);
  } else {
    blikRedLED();
  }
}

void blikRedLED() {
  Serial.println("Not connected!");
  digitalWrite(D7, HIGH);
}
