#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_MPL115A2.h>
#include <SPI.h>
#include <WiFi101.h>
#include "secretstuff.h"


const int LEDPIN = 6;
const int FANPIN = 7;
const int NUMPIXELS = 64;
const int DELAYAMOUNT = 500;
const int REDCOLOR = 150;
const int BLUECOLOR = 150;


static int heartbeat_timer = 0;

// ongoing timer counter for sensor
static int sensor_timer = 0;

// set heartbeat period in milliseconds
static int heartbeat_period = 60000;

// set sensor transmit period in milliseconds
static int sensor_period = 5000;

// track time when last connection error occurs
long lastReconnectAttempt = 0;


static float prevTemperatureC = 0.0;
int fanSpeed = 0;



float pressureKPA = 0.0;
float temperatureC = 0.0;
int pressureKPA_INTEGER = 0;
int temperatureC_INTEGER = 0;

unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 20L * 1000L;

int status = WL_IDLE_STATUS;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
Adafruit_MPL115A2 mpl115a2;
WiFiClient wifiClient;


void setup() {
  pinMode(LEDPIN, OUTPUT);
  pinMode(FANPIN, OUTPUT);

  Serial.begin(9600);
  pixels.begin();
  mpl115a2.begin();

  while ( status != WL_CONNECTED) {
    status = WiFi.begin(MYSSID, MYSSIDPASSWORD);
    delay(10000);
  }

  connectMQTT();
}


void callback(char* topic, byte * payload, unsigned int length) {
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


    if ((millis() - sensor_timer) > sensor_period) {
      sensor_timer = millis();

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
  handleSensor();
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
