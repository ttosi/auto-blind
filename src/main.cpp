#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "BasicStepperDriver.h"

void setupOta();

#define MOTOR_STEPS 200
#define MICROSTEPS 1
#define RPM 100
#define DIR 7
#define STEP 9

#define POSITION_INPUT_PIN 11
#define LDR_INPUT_PIN 12

const char *ssid = "raven";
const char *password = "makenna7122";

const uint16_t WHEN_TO_OPEN_BRIGHTNESS = 75;
const uint16_t WHEN_TO_CLOSE_BRIGHTNESS = 15;
const uint16_t WHEN_TO_STOP_OPENING = 10; // lower number open further
const uint16_t WHEN_TO_STOP_CLOSING = 340; // larger number close further

int16_t currentBrightness = 0;
boolean isDay;
boolean isNight;
boolean hasRunDay = false;
boolean hasRunNight = false;

BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);

boolean runStepper = true;

void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  analogReadResolution(9);

  stepper.begin(RPM, MICROSTEPS);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println(WiFi.localIP());

  setupOta();

  Serial.println("initialized...");
}

void loop()
{
  ArduinoOTA.handle();

  currentBrightness = analogRead(LDR_INPUT_PIN);

  if (!runStepper)
  {
    Serial.print("brightness: ");
    Serial.println(currentBrightness);
    Serial.print("position:   ");
    Serial.println(analogRead(POSITION_INPUT_PIN));
  }

  isDay = currentBrightness > WHEN_TO_OPEN_BRIGHTNESS;
  isNight = currentBrightness < WHEN_TO_CLOSE_BRIGHTNESS;

  if (runStepper)
  {

    // open the blinds
    if (isDay && !hasRunDay)
    {
      digitalWrite(LED_BUILTIN, HIGH);

      stepper.enable();
      while (analogRead(POSITION_INPUT_PIN) > WHEN_TO_STOP_OPENING)
      {
        stepper.rotate(-100);
      }
      stepper.disable();

      hasRunDay = true;
      hasRunNight = false;
    }

    // close the blinds
    if (isNight && !hasRunNight)
    {
      digitalWrite(LED_BUILTIN, HIGH);

      stepper.enable();
      while (analogRead(POSITION_INPUT_PIN) < WHEN_TO_STOP_CLOSING)
      {
        stepper.rotate(100);
      }
      stepper.disable();

      hasRunDay = false;
      hasRunNight = true;
    }
  }
  
  delay(100);
}

void setupOta()
{
  ArduinoOTA.onStart([]() {})
      .onEnd([]() {})
      .onProgress([](unsigned int progress, unsigned int total) {})
      .onError([](ota_error_t error)
               { Serial.printf("Error[%u]: ", error); });

  ArduinoOTA.begin();
}