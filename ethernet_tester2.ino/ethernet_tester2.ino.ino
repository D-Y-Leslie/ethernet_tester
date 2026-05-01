#include <Arduino.h>

#if __has_include(<esp_arduino_version.h>)
#include <esp_arduino_version.h>
#endif

/*
  AD8310 TPin diagnostic test

  Wiring for this test:
    ESP32 GPIO40 -> 10k resistor -> AD8310 TPin
    ESP32 GND    -> AD8310 TPcom / GND

  Optional serial voltage monitor:
    AD8310 TPVout -> 10k -> ESP32 GPIO2
                         |
                        20k
                         |
                        GND

  Do not connect PlutoSDR, AD9959, SMA RF input, or RJ45 cable during this test.
*/

static const int PIN_TPIN_TEST = 40;
static const int PIN_AD8310_ADC = 2;

static const uint32_t TEST_FREQ_HZ = 1000000UL;
static const uint8_t TEST_RESOLUTION_BITS = 1;
static const uint32_t PHASE_DURATION_MS = 60000;
static const uint32_t SAMPLE_INTERVAL_MS = 1000;

#ifndef ESP_ARDUINO_VERSION_MAJOR
#define ESP_ARDUINO_VERSION_MAJOR 2
#endif

#if ESP_ARDUINO_VERSION_MAJOR >= 3
static bool squareWaveAttached = false;

void startSquareWave() {
  if (!squareWaveAttached) {
    squareWaveAttached = ledcAttach(PIN_TPIN_TEST, TEST_FREQ_HZ, TEST_RESOLUTION_BITS);
  }
  ledcWrite(PIN_TPIN_TEST, 1);
}

void stopSquareWave() {
  if (squareWaveAttached) {
    ledcWrite(PIN_TPIN_TEST, 0);
    ledcDetach(PIN_TPIN_TEST);
    squareWaveAttached = false;
  }
  pinMode(PIN_TPIN_TEST, OUTPUT);
  digitalWrite(PIN_TPIN_TEST, LOW);
}
#else
static const int LEDC_CHANNEL = 0;
static bool squareWaveAttached = false;

void startSquareWave() {
  if (!squareWaveAttached) {
    ledcSetup(LEDC_CHANNEL, TEST_FREQ_HZ, TEST_RESOLUTION_BITS);
    ledcAttachPin(PIN_TPIN_TEST, LEDC_CHANNEL);
    squareWaveAttached = true;
  }
  ledcWrite(LEDC_CHANNEL, 1);
}

void stopSquareWave() {
  if (squareWaveAttached) {
    ledcWrite(LEDC_CHANNEL, 0);
    ledcDetachPin(PIN_TPIN_TEST);
    squareWaveAttached = false;
  }
  pinMode(PIN_TPIN_TEST, OUTPUT);
  digitalWrite(PIN_TPIN_TEST, LOW);
}
#endif

float readGpio2Voltage() {
  const int samples = 100;
  uint32_t sum = 0;

  for (int i = 0; i < samples; i++) {
    sum += analogRead(PIN_AD8310_ADC);
    delay(2);
  }

  float raw = (float)sum / samples;
  return raw * 3.3f / 4095.0f;
}

void printSample(bool signalOn) {
  uint32_t now = millis();
  float gpio2_v = readGpio2Voltage();
  float ad8310_out_est = gpio2_v * 1.5f;

  Serial.print("AD8310_TPIN_TEST,ms=");
  Serial.print(now);
  Serial.print(",state=");
  Serial.print(signalOn ? "ON" : "OFF");
  Serial.print(",gpio40=");
  Serial.print(signalOn ? "1MHz" : "LOW");
  Serial.print(",gpio2_v=");
  Serial.print(gpio2_v, 3);
  Serial.print(",AD8310_OUT_est=");
  Serial.print(ad8310_out_est, 3);
  Serial.println(",unit=V");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_AD8310_ADC, INPUT);
  pinMode(PIN_TPIN_TEST, OUTPUT);
  digitalWrite(PIN_TPIN_TEST, LOW);

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_AD8310_ADC, ADC_11db);

  stopSquareWave();

  Serial.println();
  Serial.println("====================================");
  Serial.println("AD8310 TPin diagnostic test");
  Serial.println("GPIO40 OFF/ON square-wave step test");
  Serial.println("Wire: GPIO40 -> 10k resistor -> AD8310 TPin");
  Serial.println("Wire: ESP32 GND -> AD8310 TPcom");
  Serial.println("Do not connect PlutoSDR, AD9959, SMA RF input, or RJ45 during this test.");
  Serial.println("Each phase lasts 60 seconds: OFF -> ON -> OFF -> ON ...");
  Serial.println("Output:");
  Serial.println("AD8310_TPIN_TEST,ms=<time>,state=<OFF|ON>,gpio40=<LOW|1MHz>,gpio2_v=<voltage>,AD8310_OUT_est=<voltage>,unit=V");
  Serial.println("====================================");
}

void loop() {
  static bool signalOn = false;
  static uint32_t phaseStart = millis();
  static uint32_t lastSample = 0;

  uint32_t now = millis();

  if (now - phaseStart >= PHASE_DURATION_MS) {
    signalOn = !signalOn;
    phaseStart = now;

    if (signalOn) {
      startSquareWave();
    } else {
      stopSquareWave();
    }

    Serial.print("PHASE_CHANGE,ms=");
    Serial.print(now);
    Serial.print(",state=");
    Serial.println(signalOn ? "ON" : "OFF");
  }

  if (lastSample == 0 || now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = now;
    printSample(signalOn);
  }
}
