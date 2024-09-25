#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS 1000  // 1 second

const char* ssid = "IDEA_WIFI_2";  // Replace with your Wi-Fi network name
const char* password = "ideawifi2";  // Replace with your Wi-Fi password

String apiKey = "E0SZO2XMU6OSF6WA";  // Replace with ThingSpeak API key
const char* server = "http://api.thingspeak.com/update";

// Pulse oximeter setup
PulseOximeter pox;
uint32_t tsLastReport = 0;

void onBeatDetected() {
    Serial.println("Beat detected!");
}

void setup() {
    Serial.begin(115200);

    // Initialize the WiFi module
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Initialize the pulse oximeter
    if (!pox.begin()) {
        Serial.println("FAILED to initialize pulse oximeter");
        while (true);
    } else {
        Serial.println("Pulse oximeter initialized");
    }
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
    pox.update();

    // Send data to ThingSpeak every second
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        tsLastReport = millis();

        float heartRate = pox.getHeartRate();
        float oxygenSaturation = pox.getSpO2();

        Serial.print("Raw Heart Rate: ");
        Serial.println(heartRate);
        Serial.print("Raw SpO2: ");
        Serial.println(oxygenSaturation);
        
        // Only send to ThingSpeak if heartRate is valid
        if (heartRate > 0) {
            Serial.print("Heart Rate: ");
            Serial.println(heartRate);
            
            if (WiFi.status() == WL_CONNECTED) {
                HTTPClient http;
                String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(heartRate) + "&field2=" + String(oxygenSaturation);
                http.begin(url);
                int httpCode = http.GET();

                if (httpCode > 0) {
                    Serial.println("Data sent to ThingSpeak successfully");
                } else {
                    Serial.println("Error in sending data to ThingSpeak");
                }
                http.end();
            }
        } else {
            Serial.println("Invalid heart rate or sensor not ready.");
        }
    }
}

