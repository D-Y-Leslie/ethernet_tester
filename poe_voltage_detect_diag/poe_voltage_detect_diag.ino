#include <Arduino.h>

/*
  PoE voltage detect diagnostic sketch

  Hardware under test:
    Mode A bridge output midpoint -> 47k -> ESP32 GPIO2
    Mode B bridge output midpoint -> 47k -> ESP32 GPIO13
    Each bridge negative output -> ESP32 GND

  Keep this sketch isolated from the main tester firmware. It only reads
  protected ADC nodes and prints a CSV-style diagnostic line.
*/

static const int PIN_POE_ADC_A = 2;
static const int PIN_POE_ADC_B = 13;

static const float POE_DIVIDER_RATIO = 20.6078f;
static const float POE_BRIDGE_DROP_COMP_V = 1.2f;
static const float ADC_NOISE_FLOOR_V = 0.05f;

static const float NO_POE_MAX_V = 5.0f;
static const float POE_PRESENT_MIN_V = 30.0f;
static const float POE_OVER_RANGE_V = 60.0f;

static const uint16_t POE_ADC_SAMPLES = 120;
static const uint16_t SAMPLE_DELAY_US = 1000;
static const uint32_t PRINT_INTERVAL_MS = 1000;

float readPoeAdcVoltageV(int pin) {
  uint32_t sumMv = 0;

  for (uint16_t i = 0; i < POE_ADC_SAMPLES; i++) {
    sumMv += analogReadMilliVolts(pin);
    delayMicroseconds(SAMPLE_DELAY_US);
  }

  return (float)sumMv / (float)POE_ADC_SAMPLES / 1000.0f;
}

float estimatePoeInputVoltageV(float adcV) {
  if (adcV < ADC_NOISE_FLOOR_V) {
    return 0.0f;
  }

  return adcV * POE_DIVIDER_RATIO + POE_BRIDGE_DROP_COMP_V;
}

const char* classifyPoeStatus(float va, float vb) {
  if (va > POE_OVER_RANGE_V || vb > POE_OVER_RANGE_V) {
    return "OVER_RANGE";
  }

  if (va >= POE_PRESENT_MIN_V || vb >= POE_PRESENT_MIN_V) {
    return "OK";
  }

  if (va <= NO_POE_MAX_V && vb <= NO_POE_MAX_V) {
    return "NO_POE";
  }

  return "UNSAFE";
}

const char* classifyPoeMode(float va, float vb) {
  bool modeA = (va >= POE_PRESENT_MIN_V && va <= POE_OVER_RANGE_V);
  bool modeB = (vb >= POE_PRESENT_MIN_V && vb <= POE_OVER_RANGE_V);

  if (modeA && modeB) {
    return "A+B";
  }

  if (modeA) {
    return "A";
  }

  if (modeB) {
    return "B";
  }

  return "UNKNOWN";
}

void printPoeDiagLine(float adcA, float adcB, float va, float vb, const char* mode, const char* status) {
  Serial.print("POE_DIAG,ms=");
  Serial.print(millis());
  Serial.print(",adcA=");
  Serial.print(adcA, 3);
  Serial.print(",adcB=");
  Serial.print(adcB, 3);
  Serial.print(",VA=");
  Serial.print(va, 2);
  Serial.print(",VB=");
  Serial.print(vb, 2);
  Serial.print(",mode=");
  Serial.print(mode);
  Serial.print(",status=");
  Serial.print(status);
  Serial.println(",unit=V");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_POE_ADC_A, INPUT);
  pinMode(PIN_POE_ADC_B, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_POE_ADC_A, ADC_11db);
  analogSetPinAttenuation(PIN_POE_ADC_B, ADC_11db);

  Serial.println();
  Serial.println("====================================");
  Serial.println("PoE voltage detect diagnostic");
  Serial.println("Input A: protected Mode A ADC node on pin 2");
  Serial.println("Input B: protected Mode B ADC node on pin 13");
  Serial.println("Output: POE_DIAG,ms=<time>,adcA=<v>,adcB=<v>,VA=<v>,VB=<v>,mode=<mode>,status=<status>,unit=V");
  Serial.println("Stop immediately if the board heats, resets, or any ADC input approaches 3.0 V.");
  Serial.println("====================================");
}

void loop() {
  static uint32_t lastPrintMs = 0;
  uint32_t now = millis();

  if (lastPrintMs == 0 || now - lastPrintMs >= PRINT_INTERVAL_MS) {
    lastPrintMs = now;

    float adcA = readPoeAdcVoltageV(PIN_POE_ADC_A);
    float adcB = readPoeAdcVoltageV(PIN_POE_ADC_B);
    float va = estimatePoeInputVoltageV(adcA);
    float vb = estimatePoeInputVoltageV(adcB);
    const char* status = classifyPoeStatus(va, vb);
    const char* mode = classifyPoeMode(va, vb);

    printPoeDiagLine(adcA, adcB, va, vb, mode, status);
  }
}
