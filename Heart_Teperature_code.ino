#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// MAX30100 setup
#define REPORTING_PERIOD_MS 1000  // Reporting interval for heart rate and SpO2
PulseOximeter pox;                // Create a PulseOximeter object
uint32_t tsLastReport = 0;        // Time for the last heart rate report

// DHT setup
#define DHTPIN 23                 // Pin where the DHT sensor is connected
#define DHTTYPE DHT11             // DHT 11 or DHT 22
DHT dht(DHTPIN, DHTTYPE);

// Wi-Fi and ThingSpeak setup
const char* ssid = "IDEA_WIFI_2";  // Your Wi-Fi network name
const char* password = "ideawifi2";  // Your Wi-Fi password
String apiKey = "E0SZO2XMU6OSF6WA";  // ThingSpeak API key
const char* server = "http://api.thingspeak.com/update";

// IFTTT Webhook setup
const char* iftttWebhookURL = "https://maker.ifttt.com/trigger/tempreature_alert/with/key/caT3j1GCDuE7i6NMXRFw5c";  // Replace with your IFTTT Webhook key
const char* iftttHeartRateAlertURL = "https://maker.ifttt.com/trigger/heart_rate_alert/with/key/caT3j1GCDuE7i6NMXRFw5c";  // Replace with your IFTTT Webhook key for heart rate alert

void onBeatDetected() {
    Serial.println("Beat detected!");
}

void setup() {
    Serial.begin(115200);
    
    // Wi-Fi setup
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("...");
    }
    Serial.println("Connected to WiFi");
    
    // Initialize DHT sensor
    dht.begin();

    // Initialize MAX30100
    Serial.print("Initializing MAX30100 Pulse Oximeter...");
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;); // Halt on failure
    } else {
        Serial.println("SUCCESS");
    }
    
    pox.setIRLedCurrent(MAX30100_LED_CURR_46_8MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
    // Update PulseOximeter readings
    pox.update();
    
    // Get temperature reading
    float temperature = dht.readTemperature();
    
    if (isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor!");
    } else {
        Serial.print("Temperature: ");
        Serial.println(temperature);
    }

    // Report heart rate and SpO2
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart Rate: ");
        int heartRate = pox.getHeartRate();
        Serial.print(heartRate);
        Serial.print(" bpm / SpO2: ");
        Serial.print(pox.getSpO2());
        Serial.println(" %");

        // Send data to ThingSpeak
        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(temperature) + "&field2=" + String(heartRate) + "&field3=" + String(pox.getSpO2());
            http.begin(url);
            int httpCode = http.GET();

            if (httpCode > 0) {
                Serial.println("Data sent to ThingSpeak successfully");
            } else {
                Serial.println("Error in sending data to ThingSpeak");
            }
            http.end();
        }

        // Trigger IFTTT alert if temperature exceeds 30°C
        if (temperature > 30) {
            HTTPClient http;
            http.begin(iftttWebhookURL);
            int httpCode = http.GET();

            if (httpCode > 0) {
                Serial.println("Temperature alert sent to IFTTT successfully");
            } else {
                Serial.println("Error in sending temperature alert to IFTTT");
            }
            http.end();
        }

        // Trigger IFTTT alert if heart rate exceeds 200 bpm
        if (heartRate > 200) {
            HTTPClient http;
            http.begin(iftttHeartRateAlertURL);
            int httpCode = http.GET();

            if (httpCode > 0) {
                Serial.println("Heart rate alert sent to IFTTT successfully");
            } else {
                Serial.println("Error in sending heart rate alert to IFTTT");
            }
            http.end();
        }

        tsLastReport = millis();  // Update timestamp
    }

    delay(2000);  // Delay between measurements
}

