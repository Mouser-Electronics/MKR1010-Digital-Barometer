/***************************************************************************
 MKR1010 Digital Galileo Thermometer and Glass Globe Barometer

  
  The program includes a library and portions of sample code from Adafruit
  with their description below:
  This is a library for the BMP280 humidity, temperature & pressure sensor
  Designed specifically to work with the Adafruit BMEP280 Breakout
  ----> http://www.adafruit.com/products/2651
  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!
  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP085.h>
#include "secretstuff.h"

const int LEDPIN = 6;
const int FANPIN = 7;
const int NUMPIXELS = 64;
const int DELAYAMOUNT = 500;
const int REDCOLOR = 25;
const int BLUECOLOR = 25;
const int LOWPRESSURE = 85;
const int HIGHPRESSURE = 110;
const int FANLOWSPEED = 193;
const int FANHIGHSPEED = 255;

Adafruit_BMP085 bmp;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

static int heartbeat_timer = 0;
static int sensor_timer = 0;
static int heartbeat_period = 60000;
static int sensor_period = 5000;
long lastReconnectAttempt = 0;

WiFiClient wifiClient;

static float prevTempC = 0.0;

void setup() {

  Serial.begin(9600);
  while (!Serial) {}

  WiFi.begin(myssid, myssidpw);
  delay(5000);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Failed to connect, resetting"));
    WiFi.begin(myssid, myssidpw);
    delay(1000);
  }

  Serial.println(F("Connected to Wifi!"));

  Serial.println(F("Init hardware settings..."));
  pinMode(LEDPIN, OUTPUT);
  pinMode(FANPIN, OUTPUT);

  connectMQTT();

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  pixels.begin();

  Serial.println("Setup is complete!");
}

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  int i = 0;
  char message_buff[length + 1];
  for (i = 0; i < length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';

  Serial.print(F("Received some data: "));
  Serial.println(String(message_buff));
}

PubSubClient client(server, port, callback, wifiClient);

boolean connectMQTT()
{
  if (client.connect((char*) mqtt_username, (char*) mqtt_username, (char*) mqtt_password)) {
    Serial.println(F("Connected to MQTT broker"));

    if (client.publish((char*) pub_topic, "{\"event_data\":{\"mqtt_connected\":true}}")) {
      Serial.println("Publish connected message ok");
    } else {
      Serial.print(F("Publish connected message failed: "));
      Serial.println(String(client.state()));
    }

    if (client.subscribe((char *)sub_topic, 1)) {
      Serial.println(F("Successfully subscribed"));
    } else {
      Serial.print(F("Subscribed failed: "));
      Serial.println(String(client.state()));
    }
  } else {
    Serial.println(F("MQTT connect failed"));
    Serial.println(F("Will reset and try again..."));
    abort();
  }
  return client.connected();
}

void loop() {
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 1000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (connectMQTT()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
  heartbeat_loop();
  bmp_loop();
}

void bmp_loop() {
  float pressureKPA = 0.0;
  float tempC = 0.0;
  if ((millis() - sensor_timer) > sensor_period) {
    sensor_timer = millis();
    tempC = bmp.readTemperature();
    pressureKPA = bmp.readPressure();

    String payload = "{\"event_data\":{\"temperature\":";
    payload += tempC;
    payload += ",\"pressure\":";
    payload += pressureKPA;
    payload += "}}";

    if (client.loop()) {
      Serial.print(F("Sending sensor: "));
      Serial.println(payload);

      if (client.publish((char *) pub_topic, (char*) payload.c_str()) ) {
        Serial.println("Publish ok");
      } else {
        Serial.print(F("Failed to publish sensor data: "));
        Serial.println(String(client.state()));
      }
    }

    Serial.println();
    delay(2000);
    displayResults(tempC, pressureKPA);
  }
}

void heartbeat_loop() {
  if ((millis() - heartbeat_timer) > heartbeat_period) {
    heartbeat_timer = millis();
    String payload = "{\"event_data\":{\"millis\":";
    payload += millis();
    payload += ",\"heartbeat\":true}}";

    if (client.loop()) {
      Serial.print(F("Sending heartbeat: "));
      Serial.println(payload);

      if (client.publish((char *) pub_topic, (char*) payload.c_str()) ) {
        Serial.println(F("Publish ok"));
      } else {
        Serial.print(F("Failed to publish heartbeat: "));
        Serial.println(String(client.state()));
      }
    }
  }
}




void displayResults(float tempC, float pressureKPA) {
  if (tempC > 64.0) {
    tempC = 64.0;
  }

  for (int i = 0; i < (int)tempC; i++) {
    if (prevTempC < tempC) {
      pixels.setPixelColor(i, pixels.Color(REDCOLOR, 0, 0));
    }
    else {
      pixels.setPixelColor(i, pixels.Color(0, 0, BLUECOLOR));
    }
  }

  pixels.show();
  prevTempC = tempC;

  int fanSpeed = map((int)pressureKPA, LOWPRESSURE, HIGHPRESSURE, FANLOWSPEED, FANHIGHSPEED);
  analogWrite(FANPIN, fanSpeed);
}
