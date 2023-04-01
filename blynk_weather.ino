#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "bsec.h"

#include "secrets.h"

// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);

char ssid[] = SSID;
char pass[] = PASS;

// Create an object of the class Bsec
Bsec iaqSensor;

String output;

BlynkTimer timer;
// Entry point for the example
void setup(void) {
  /* Initializes the Serial communication */
  Serial.begin(115200);
  Serial.println("\nConnecting to Blynk");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  pinMode(LED_BUILTIN, OUTPUT);
  iaqSensor.begin(BME68X_I2C_ADDR_LOW, Wire);
  output = "BSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();

  bsec_virtual_sensor_t sensorList[13] = {
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_GAS_PERCENTAGE
  };

  iaqSensor.updateSubscription(sensorList, 13, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();

  // Print the header
  output = "Timestamp [ms], IAQ, IAQ accuracy, Static IAQ, CO2 equivalent, breath VOC equivalent, raw temp[°C], pressure [hPa], raw relative humidity [%], gas [Ohm], Stab Status, run in status, comp temp[°C], comp humidity [%], gas percentage";
  Serial.println(output);

  getSensorValues();

  // Setup a function to be called every 60 seconds
  timer.setInterval(60000L, getSensorValues);
}

// Function that is looped forever
void loop(void) {
  Blynk.run();
  timer.run();
}

void getSensorValues() {
  unsigned long time_trigger = millis();
  if (iaqSensor.run()) {  // If new data is available
    digitalWrite(LED_BUILTIN, LOW);

    Blynk.virtualWrite(V1, iaqSensor.iaq);
    Blynk.virtualWrite(V2, iaqSensor.temperature);
    Blynk.virtualWrite(V3, iaqSensor.humidity);
    Blynk.virtualWrite(V4, iaqSensor.pressure / 100);  //Dividing by 100 to convert Pa to hPa

    if (iaqSensor.stabStatus == 1.00) {
      Blynk.virtualWrite(V5, "Finished");
    } else {
      Blynk.virtualWrite(V5, "Ongoing");
    }

    if (iaqSensor.runInStatus == 1.00) {
      Blynk.virtualWrite(V6, "Finished");
    } else {
      Blynk.virtualWrite(V6, "Ongoing");
    }

    output = String(time_trigger);
    output += ", " + String(iaqSensor.iaq);
    output += ", " + String(iaqSensor.iaqAccuracy);
    output += ", " + String(iaqSensor.staticIaq);
    output += ", " + String(iaqSensor.co2Equivalent);
    output += ", " + String(iaqSensor.breathVocEquivalent);
    output += ", " + String(iaqSensor.rawTemperature);
    output += ", " + String(iaqSensor.pressure / 100);
    output += ", " + String(iaqSensor.rawHumidity);
    output += ", " + String(iaqSensor.gasResistance);
    output += ", " + String(iaqSensor.stabStatus);
    output += ", " + String(iaqSensor.runInStatus);
    output += ", " + String(iaqSensor.temperature);
    output += ", " + String(iaqSensor.humidity);
    output += ", " + String(iaqSensor.gasPercentage);
    Serial.println(output);

    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    checkIaqSensorStatus();
  }
}

// Helper function definitions
void checkIaqSensorStatus(void) {
  if (iaqSensor.bsecStatus != BSEC_OK) {
    if (iaqSensor.bsecStatus < BSEC_OK) {
      output = "BSEC error code : " + String(iaqSensor.bsecStatus);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BSEC warning code : " + String(iaqSensor.bsecStatus);
      Serial.println(output);
    }
  }

  if (iaqSensor.bme68xStatus != BME68X_OK) {
    if (iaqSensor.bme68xStatus < BME68X_OK) {
      output = "BME68X error code : " + String(iaqSensor.bme68xStatus);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BME68X warning code : " + String(iaqSensor.bme68xStatus);
      Serial.println(output);
    }
  }
}

void errLeds(void) {
  Serial.println("Encountered an error");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED() {
  Serial.println("Connected to Blynk");
}