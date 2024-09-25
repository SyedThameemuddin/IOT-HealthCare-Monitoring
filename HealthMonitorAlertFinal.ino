#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>
#include <Wire.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Pulse Sensor setup
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = 34;  // GPIO34 for ESP32
const int PULSE_BLINK = 2;   // On-board LED pin or another GPIO pin for blink
const int THRESHOLD = 550;   // Adjust this threshold for noise filtering
PulseSensorPlayground pulseSensor;

// DHT setup
#define DHTPIN 23                 // Pin where the DHT sensor is connected
#define DHTTYPE DHT11             // DHT 11 or DHT 22
DHT dht(DHTPIN, DHTTYPE);

// Wi-Fi and ThingSpeak setup
const char* ssid = "IDEA_WIFI_2";   // Your Wi-Fi network name
const char* password = "ideawifi2"; // Your Wi-Fi password
String apiKey = "E0SZO2XMU6OSF6WA"; // ThingSpeak API key
const char* server = "http://api.thingspeak.com/update";

// IFTTT setup (replace with your IFTTT event name and key)
const char* ifttt_event_name = "health_alert";  // Replace with your IFTTT event name
const char* ifttt_key = "caT3j1GCDuE7i6NMXRFw5c";  // Replace with your IFTTT Webhook key
String ifttt_url = "http://maker.ifttt.com/trigger/" + String(ifttt_event_name) + "/with/key/" + String(ifttt_key);

// Variables for timing ThingSpeak updates
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 15000;  // 15 seconds (ThingSpeak limit)

// Store the last valid heart rate
int lastValidHeartRate = 0;

void setup() {
    Serial.begin(115200);  // Start serial communication at 115200 baud

    // Configure the PulseSensor manager
    pulseSensor.analogInput(PULSE_INPUT);
    pulseSensor.blinkOnPulse(PULSE_BLINK);
    pulseSensor.setOutputType(OUTPUT_TYPE);
    pulseSensor.setThreshold(THRESHOLD);

    // Start reading the PulseSensor signal
    if (!pulseSensor.begin()) {
        for(;;) {
            digitalWrite(PULSE_BLINK, LOW);
            delay(50);
            digitalWrite(PULSE_BLINK, HIGH);
            delay(50);
        }
    }

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
}

void loop() {
    // Update PulseSensor readings continuously
    pulseSensor.outputSample();

    // Check if a beat has been detected
    if (pulseSensor.sawStartOfBeat()) {
        lastValidHeartRate = pulseSensor.getBeatsPerMinute(); // Store latest heart rate
        Serial.print("Heart Rate: ");
        Serial.println(lastValidHeartRate); // Print heart rate to Serial
    }

    // Get temperature reading from DHT sensor
    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor!");
    } else {
        Serial.print("Temperature: ");
        Serial.println(temperature);
    }

    // Send data to ThingSpeak every 15 seconds
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime > updateInterval) {
        lastUpdateTime = currentTime;

        // Send temperature and the most recent heart rate to ThingSpeak
        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(temperature) + "&field2=" + String(lastValidHeartRate); // Send last valid heart rate
            http.begin(url);
            int httpCode = http.GET();

            if (httpCode > 0) {
                Serial.println("Data sent to ThingSpeak successfully");
            } else {
                Serial.println("Error in sending data to ThingSpeak");
            }
            http.end();
        }
    }

    // Check if heart rate exceeds 120 BPM or temperature exceeds 35°C and trigger IFTTT
    if (lastValidHeartRate > 120 || temperature > 35) {
        sendAlert(lastValidHeartRate, temperature); // Call function to trigger alert
    }
}

void sendAlert(int heartRate, float temperature) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String ifttt_alert_url = ifttt_url + "?value1=" + String(heartRate) + "&value2=" + String(temperature);
        http.begin(ifttt_alert_url);
        int httpCode = http.POST("");  // Send a POST request to trigger the alert

        if (httpCode > 0) {
            Serial.println("IFTTT alert triggered successfully");
        } else {
            Serial.println("Error in triggering IFTTT alert");
        }
        http.end();
    }
}
