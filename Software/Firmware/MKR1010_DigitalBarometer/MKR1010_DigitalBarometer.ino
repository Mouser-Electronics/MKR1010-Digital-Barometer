#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_MPL115A2.h>


const int LEDPIN = 6;
const int FANPIN = 7;
const int NUMPIXELS = 64;
const int DELAYAMOUNT = 500;
const int REDCOLOR = 150;
const int BLUECOLOR = 150;

float prevTemperatureC = 0.0;
int fanSpeed = 0;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
Adafruit_MPL115A2 mpl115a2;


void setup() {
  pinMode(LEDPIN, OUTPUT);
  pinMode(FANPIN, OUTPUT);
  
  Serial.begin(9600);
  pixels.begin();
  mpl115a2.begin();
}

void loop() {
  float pressureKPA = mpl115a2.getPressure();
  float temperatureC = mpl115a2.getTemperature();
  int pressureKPA_INTEGER = (int)pressureKPA;
  int temperatureC_INTEGER = (int)temperatureC;
  
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
    delay(DELAYAMOUNT);
  }


  fanSpeed = map(pressureKPA_INTEGER, 85, 110, 0, 255);
  analogWrite(FANPIN, fanSpeed);

  
}
