/***************************************************************************
  This is an example of program for connected the Adafruit Huzzah and BMP280
  to the Medium One Prototyping Sandbox.  Visit www.medium.one for more information.
  Author: Medium One
  Last Revision Date: May 1, 2018
  The program includes a library and portions of sample code from Adafruit
  with their description below:

  This is a library for the BMP280 humidity, temperature & pressure sensor
  Designed specifically to work with the Adafruit BMEP280 Breakout
  ----> http://www.adafruit.com/products/2651
  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.
  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!
  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi101.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP085.h>
#include "secretstuff.h"

const int LEDPIN = 6;
const int FANPIN = 7;
const int NUMPIXELS = 64;
const int DELAYAMOUNT = 500;
const int REDCOLOR = 150;
const int BLUECOLOR = 150;

<<<<<<< HEAD
Adafruit_BMP085 bmp; // I2C

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
=======
static int heartbeatMs = 0;
static int sensorTimer = 0;
static int heartbeatPeriod = 60000;
static int sensorPeriod = 5000;
long lastReconnectAttempt = 0;
unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 20L * 1000L;

float pressureKPA = 0.0;
float temperatureC = 0.0;
int pressureKPA_INTEGER = 0;
int temperatureC_INTEGER = 0;
static float prevTemperatureC = 0.0;

static int fanSpeed = 0;
>>>>>>> 6f24e0fe787ebfb6a4038a2ae012c76861d788b9

  WiFi.begin(myssid, myssidpw);
  delay(5000);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Failed to connect, resetting"));
  }

  Serial.println(F("Connected to Wifi!"));

<<<<<<< HEAD
  Serial.println(F("Init hardware settings..."));
=======


void setup() {
>>>>>>> 6f24e0fe787ebfb6a4038a2ae012c76861d788b9
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
  // Important Note: MQTT requires a unique id (UUID), we are using the mqtt_username as the unique ID
  // Besure to create a new device ID if you are deploying multiple devices.
  // Learn more about Medium One's self regisration option on docs.mediumone.com
  if (client.connect((char*) mqtt_username, (char*) mqtt_username, (char*) mqtt_password)) {
    Serial.println(F("Connected to MQTT broker"));

    // send a connect message
    if (client.publish((char*) pub_topic, "{\"event_data\":{\"mqtt_connected\":true}}")) {
      Serial.println("Publish connected message ok");
    } else {
      Serial.print(F("Publish connected message failed: "));
      Serial.println(String(client.state()));
    }

    // subscrive to MQTT topic
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

<<<<<<< HEAD
=======



void handleSensor() {
  pressureKPA = mpl115a2.getPressure();
  temperatureC = mpl115a2.getTemperature();
  pressureKPA_INTEGER = (int)pressureKPA;
  temperatureC_INTEGER = (int)temperatureC;

  Serial.print(F("P: "));
  Serial.print(pressureKPA);
  Serial.print(F("KPA  T: "));
  Serial.print(temperatureC);
  Serial.println(F("C"));

  if (temperatureC_INTEGER > 64) {
    temperatureC_INTEGER = 64;
  }

  for (int i = 0; i < temperatureC_INTEGER; i++) {
    if (prevTemperatureC < temperatureC) {
      pixels.setPixelColor(i, pixels.Color(REDCOLOR, 0, 0));
    }
    else {
      pixels.setPixelColor(i, pixels.Color(0, 0, BLUECOLOR));
    }

    pixels.show();
    prevTemperatureC = temperatureC;


    if ((millis() - sensorTimer) > sensorPeriod) {
      sensorTimer = millis();

      String payload = "{\"event_data\":{\"temperature\":";
      payload += temperatureC;
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
      delay(DELAYAMOUNT);
    }


    fanSpeed = map(pressureKPA_INTEGER, 85, 110, 0, 255);
    analogWrite(FANPIN, fanSpeed);
  }
}






>>>>>>> 6f24e0fe787ebfb6a4038a2ae012c76861d788b9
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
    // Client connected
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
    //payload += ",\"altitude\":";
    //payload += bmp.readAltitude(1013.25);
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
  if ((millis() - heartbeatMs) > heartbeatPeriod) {
    heartbeatMs = millis();
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

  int fanSpeed = map((int)pressureKPA, 85, 110, 0, 255);
  analogWrite(FANPIN, fanSpeed);
}
