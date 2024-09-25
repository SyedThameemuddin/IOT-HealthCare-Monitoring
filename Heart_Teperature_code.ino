#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>
#include <Wire.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Pulse Sensor setup
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = 34; // GPIO34 for ESP32
const int PULSE_BLINK = 2;  // On-board LED pin or another GPIO pin for blink
const int THRESHOLD = 550;   // Adjust this threshold for noise filtering
PulseSensorPlayground pulseSensor;

// DHT setup
#define DHTPIN 23                 // Pin where the DHT sensor is connected
#define DHTTYPE DHT11             // DHT 11 or DHT 22
DHT dht(DHTPIN, DHTTYPE);

// Wi-Fi and ThingSpeak setup
const char* ssid = "IDEA_WIFI_2";  // Your Wi-Fi network name
const char* password = "ideawifi2"; // Your Wi-Fi password
String apiKey = "E0SZO2XMU6OSF6WA";  // ThingSpeak API key
const char* server = "http://api.thingspeak.com/update";

void setup() {
    Serial.begin(115200);  // Start serial communication at 115200 baud

    // Configure the PulseSensor manager
    pulseSensor.analogInput(PULSE_INPUT);
    pulseSensor.blinkOnPulse(PULSE_BLINK);
    pulseSensor.setOutputType(OUTPUT_TYPE);
    pulseSensor.setThreshold(THRESHOLD);

    // Start reading the PulseSensor signal
    if (!pulseSensor.begin()) {
        // Initialization failed, flash the LED to indicate an error
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
    delay(20);  // Wait a bit before processing the next sample

    // Update PulseSensor readings
    pulseSensor.outputSample();

    // Check if a beat has happened
    if (pulseSensor.sawStartOfBeat()) {
        int heartRate = pulseSensor.getBeatsPerMinute(); // Get heart rate
        Serial.print("Heart Rate: ");
        Serial.println(heartRate); // Print heart rate to Serial

        // Send heart rate to ThingSpeak
        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            String url = String(server) + "?api_key=" + apiKey + "&field2=" + String(heartRate);
            http.begin(url);
            int httpCode = http.GET();

            if (httpCode > 0) {
                Serial.println("Heart rate sent to ThingSpeak successfully");
            } else {
                Serial.println("Error in sending heart rate to ThingSpeak");
            }
            http.end();
        }
    }

    // Get temperature reading
    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor!");
    } else {
        Serial.print("Temperature: ");
        Serial.println(temperature);
    }

    // Send temperature to ThingSpeak
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(temperature);
        http.begin(url);
        int httpCode = http.GET();

        if (httpCode > 0) {
            Serial.println("Temperature sent to ThingSpeak successfully");
        } else {
            Serial.println("Error in sending temperature to ThingSpeak");
        }
        http.end();
    }

    delay(2000);  // Delay between measurements
}
