#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS 1000  // Reporting interval for heart rate and SpO2

PulseOximeter pox;                // Create a PulseOximeter object
uint32_t tsLastReport = 0;        // Time at which the last report occurred

// This function is called when a beat is detected
void onBeatDetected() {
    Serial.println("Beat detected!");
}

void setup() {
    Serial.begin(115200);         // Start serial communication
    Serial.print("Initializing MAX30100 Pulse Oximeter...");

    // Initialize the sensor
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);  // Halt the program if initialization fails
    } else {
        Serial.println("SUCCESS");
    }

    // Set the current of the IR LED (you can adjust this if needed)
    pox.setIRLedCurrent(MAX30100_LED_CURR_46_8MA);

    // Register a callback function to be called when a pulse is detected
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
    // Update the sensor readings
    pox.update();

    // Report heart rate and SpO2 once every REPORTING_PERIOD_MS milliseconds
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart Rate: ");
        Serial.print(pox.getHeartRate());
        Serial.print(" bpm / SpO2: ");
        Serial.print(pox.getSpO2());
        Serial.println(" %");

        tsLastReport = millis();  // Update the timestamp for the next report
    }
}
